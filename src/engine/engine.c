#include "rim/engine.h"
#include "rim/parser.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

typedef struct {
    int stmt_index;
    char text[512];
    bool is_realtime;
} WorkerTask;

typedef struct {
    int stmt_index;
    RimValue result;
    char output[RIM_MAX_STR_LEN];
} WorkerResult;

static void worker_loop(int read_fd, int write_fd) {
    RimVM vm;
    vm_init(&vm);

    WorkerTask task;
    while (read(read_fd, &task, sizeof(WorkerTask)) > 0) {
        WorkerResult res;
        memset(&res, 0, sizeof(res));
        res.stmt_index = task.stmt_index;

        Parser p;
        parser_init(&p, task.text);
        AstNode *node = parser_parse_qa_pair(&p);
        if (node) {
            res.result = vm_eval_node(&vm, node);
            strncpy(res.output, vm.last_output, sizeof(res.output) - 1);
            ast_free(node);
        } else {
            res.result = rim_nil();
        }

        if (write(write_fd, &res, sizeof(WorkerResult)) <= 0) {
            break;
        }
    }
    close(read_fd);
    close(write_fd);
    exit(0);
}

void engine_init(RimEngine *engine, int num_workers) {
    if (num_workers <= 0) num_workers = 1;
    if (num_workers > RIM_MAX_WORKERS) num_workers = RIM_MAX_WORKERS;

    engine->num_workers = num_workers;

    for (int i = 0; i < num_workers; ++i) {
        if (pipe(engine->task_pipes[i]) < 0 || pipe(engine->res_pipes[i]) < 0) {
            perror("pipe failed");
            exit(1);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            close(engine->task_pipes[i][1]); // Close parent write
            close(engine->res_pipes[i][0]);  // Close parent read
            worker_loop(engine->task_pipes[i][0], engine->res_pipes[i][1]);
        } else {
            engine->worker_pids[i] = pid;
            close(engine->task_pipes[i][0]); // Close worker read
            close(engine->res_pipes[i][1]);  // Close worker write
        }
    }
}

void engine_eval_program(RimEngine *engine, RimVM *vm, AstNode *program) {
    if (!program) return;
    if (program->type != NODE_BLOCK) {
        vm_eval_node(vm, program);
        if (vm->last_output[0]) {
            printf("%s\n", vm->last_output);
        }
        return;
    }

    size_t count = program->block.count;
    if (count == 0) return;

    for (size_t i = 0; i < count; ++i) {
        AstNode *stmt = program->block.statements[i];
        (void)engine;
        vm_eval_node(vm, stmt);
        if (vm->last_output[0]) {
            printf("%s\n", vm->last_output);
        }
    }
}

void engine_shutdown(RimEngine *engine) {
    for (int i = 0; i < engine->num_workers; ++i) {
        close(engine->task_pipes[i][1]);
        close(engine->res_pipes[i][0]);
        kill(engine->worker_pids[i], SIGTERM);
        waitpid(engine->worker_pids[i], NULL, 0);
    }
}
