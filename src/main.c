/**
 * @file main.c
 * @brief RimLang Entry Point & Interactive REPL.
 */

#include "rim/common.h"
#include "rim/parser.h"
#include "rim/runtime.h"
#include "rim/engine.h"
#include "rim/joke_table.h"
#include "rim/korean_synonym.h"
#include "rim/korean_nlp.h"
#include <unistd.h>

#define MAX_DEFERRED_STMT 64

typedef struct {
    char raw_text[512];
    bool is_resolved;
} DeferredStatement;

static DeferredStatement s_deferred_stack[MAX_DEFERRED_STMT];
static size_t s_deferred_count = 0;

static void parse_single_table_line(const char *line) {
    if (!line) return;
    char key[128] = {0};
    char val[128] = {0};

    const char *p_is = strstr(line, "은");
    if (!p_is) p_is = strstr(line, "는");
    if (!p_is) p_is = strstr(line, "->");
    if (!p_is) p_is = strstr(line, ":");

    if (p_is) {
        size_t klen = p_is - line;
        strncpy(key, line, klen < sizeof(key) - 1 ? klen : sizeof(key) - 1);
        key[klen < sizeof(key) - 1 ? klen : sizeof(key) - 1] = '\0';

        if (strncmp(p_is, "은", 3) == 0 || strncmp(p_is, "는", 3) == 0) {
            p_is += 3;
        } else if (strncmp(p_is, "->", 2) == 0) {
            p_is += 2;
        } else {
            p_is += 1;
        }

        while (*p_is == ' ') p_is++;
        strncpy(val, p_is, sizeof(val) - 1);

        size_t vlen = strlen(val);
        while (vlen > 0 && (val[vlen-1] == '.' || val[vlen-1] == ' ' || val[vlen-1] == '\r' || val[vlen-1] == '\n')) {
            val[--vlen] = '\0';
        }

        size_t key_len = strlen(key);
        while (key_len > 0 && (key[key_len-1] == ' ')) key[--key_len] = '\0';

        joke_add_dynamic_map(key, val);
    }
}

static void eval_deferred_stack(RimVM *vm, RimEngine *engine, int workers) {
    for (size_t i = 0; i < s_deferred_count; ++i) {
        if (!s_deferred_stack[i].is_resolved) {
            Parser p;
            parser_init(&p, s_deferred_stack[i].raw_text);
            AstNode *node = parser_parse_qa_pair(&p);
            if (node) {
                if (workers > 1 && engine) {
                    engine_eval_program(engine, vm, node);
                } else {
                    RimValue res = vm_eval_node(vm, node);
                    if (vm->last_output[0]) {
                        printf("%s\n", vm->last_output);
                    } else if (res.type == VAL_INT) {
                        printf("%ld\n", res.i);
                    } else if (res.type == VAL_STR) {
                        printf("%s\n", res.s);
                    }
                    fflush(stdout);
                }
                s_deferred_stack[i].is_resolved = true;
                ast_free(node);
            }
        }
    }
}

static bool is_table_definition_line(const char *line) {
    if (strncmp(line, "~~~", 3) == 0) return true;
    // A table row is like "케이크는 1..." or "서울은 3." (ends with ... or . and is not a question)
    if (strstr(line, "?") || strstr(line, "...?")) return false;
    if ((strstr(line, "는") || strstr(line, "은")) && (strstr(line, "...") || strstr(line, "."))) {
        return true;
    }
    return false;
}

static void run_repl(int workers) {
    RimVM vm;
    vm_init(&vm);

    RimEngine engine;
    if (workers > 1) {
        engine_init(&engine, workers);
    }

    char line[1024];
    while (1) {
        printf("림> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0) continue;
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0 || strcmp(line, "끝.") == 0) {
            printf("후후후...\n");
            fflush(stdout);
            break;
        }

        // 1. Dynamic table definition line (only if not a question!)
        if (is_table_definition_line(line)) {
            parse_single_table_line(line);
            eval_deferred_stack(&vm, workers > 1 ? &engine : NULL, workers);
            continue;
        }

        // 2. Statement parse & execute
        Parser parser;
        parser_init(&parser, line);
        AstNode *node = parser_parse_qa_pair(&parser);

        if (node) {
            if (workers > 1) {
                engine_eval_program(&engine, &vm, node);
            } else {
                RimValue res = vm_eval_node(&vm, node);
                if (vm.last_output[0]) {
                    printf("%s\n", vm.last_output);
                } else if (res.type == VAL_INT) {
                    printf("%ld\n", res.i);
                } else if (res.type == VAL_STR) {
                    printf("%s\n", res.s);
                } else if (res.type == VAL_BOOL) {
                    printf("%s\n", res.b ? "참" : "거짓");
                }
                fflush(stdout);
            }
            ast_free(node);
        }
    }

    if (workers > 1) {
        engine_shutdown(&engine);
    }
}

int main(int argc, char **argv) {
    int workers = 1;
    bool worker_flag_set = false;
    const char *file_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-j") == 0 && i + 1 < argc) {
            workers = atoi(argv[++i]);
            worker_flag_set = true;
            if (workers < 1) workers = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("사용법: ./rimlang [옵션] [파일.rim]\n");
            printf("옵션:\n");
            printf("  -j <코어수>   멀티프로세스 비동기 워커 풀 실행\n");
            printf("  -h, --help    도움말 출력\n");
            return 0;
        } else if (argv[i][0] != '-') {
            file_path = argv[i];
        }
    }

    kor_nlp_init();

    if (file_path != NULL) {
        FILE *f = fopen(file_path, "rb");
        if (!f) {
            fprintf(stderr, "오류: 파일을 열 수 없습니다: %s\n", file_path);
            kor_nlp_shutdown();
            return 1;
        }

        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *buf = malloc(sz + 1);
        if (!buf) {
            fclose(f);
            kor_nlp_shutdown();
            return 1;
        }

        size_t read_bytes = fread(buf, 1, sz, f);
        buf[read_bytes] = '\0';
        fclose(f);

        Parser parser;
        parser_init(&parser, buf);
        AstNode *program = parser_parse_program(&parser);

        RimVM vm;
        vm_init(&vm);

        if (worker_flag_set && workers > 1) {
            RimEngine engine;
            engine_init(&engine, workers);
            engine_eval_program(&engine, &vm, program);
            engine_shutdown(&engine);
        } else {
            if (program && program->type == NODE_BLOCK) {
                for (size_t i = 0; i < program->block.count; ++i) {
                    AstNode *stmt = program->block.statements[i];
                    RimValue res = vm_eval_node(&vm, stmt);
                    if (vm.last_output[0]) {
                        printf("%s\n", vm.last_output);
                    } else if (res.type == VAL_INT) {
                        printf("%ld\n", res.i);
                    } else if (res.type == VAL_STR) {
                        printf("%s\n", res.s);
                    } else if (res.type == VAL_BOOL) {
                        printf("%s\n", res.b ? "참" : "거짓");
                    }
                    fflush(stdout);
                }
            } else {
                vm_run(&vm, program);
                if (vm.last_output[0]) {
                    printf("%s\n", vm.last_output);
                    fflush(stdout);
                }
            }
        }

        ast_free(program);
        free(buf);
    } else {
        run_repl(workers);
    }

    kor_nlp_shutdown();
    return 0;
}
