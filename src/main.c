/**
 * @file main.c
 * @brief Main Entrypoint & Multi-line Dialogue REPL for RimLang.
 */

#include "rim/common.h"
#include "rim/ast.h"
#include "rim/parser.h"
#include "rim/runtime.h"
#include "rim/engine.h"
#include "rim/joke_table.h"
#include "rim/korean_nlp.h"

static void run_repl(int workers) {
    RimVM vm;
    vm_init(&vm);

    RimEngine engine;
    if (workers > 1) {
        engine_init(&engine, workers);
    }

    char multiline_buf[4096] = {0};
    bool in_block = false;
    char pending_question[512] = {0};

    while (1) {
        if (in_block) {
            printf("..> ");
        } else if (pending_question[0] != '\0') {
            printf("..> ");
        } else {
            printf("림> ");
        }
        fflush(stdout);

        char line[1024];
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 && !in_block && pending_question[0] == '\0') continue;
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0 || strcmp(line, "끝.") == 0) {
            printf("후후후...\n");
            fflush(stdout);
            break;
        }

        // Check for block start (림하하..., 크흡..., 교주님...)
        if (strncmp(line, "림하하...", 9) == 0 || strncmp(line, "크흡...", 6) == 0 ||
            (strncmp(line, "교주님...", 9) == 0 && !strstr(line, "?"))) {
            in_block = true;
            snprintf(multiline_buf, sizeof(multiline_buf), "%s\n", line);
            continue;
        }

        // Inside block accumulation
        if (in_block) {
            strncat(multiline_buf, line, sizeof(multiline_buf) - strlen(multiline_buf) - 2);
            strcat(multiline_buf, "\n");
            if (strstr(line, "하하...림") || strstr(line, "하하..림") || strstr(line, "흡...흐흡")) {
                in_block = false;
                Parser parser;
                parser_init(&parser, multiline_buf);
                AstNode *node = parser_parse_program(&parser);
                if (node) {
                    if (workers > 1) {
                        engine_eval_program(&engine, &vm, node);
                    } else {
                        vm_eval_node(&vm, node);
                        if (vm.last_output[0]) {
                            printf("%s\n", vm.last_output);
                            fflush(stdout);
                        }
                    }
                    ast_free(node);
                }
                multiline_buf[0] = '\0';
            }
            continue;
        }

        // 2-line QA Pair Accumulation in REPL
        if (pending_question[0] == '\0' && (strstr(line, "?") || strstr(line, "...?"))) {
            // First line of QA pair: hold pending and prompt for second line
            strncpy(pending_question, line, sizeof(pending_question) - 1);
            continue;
        }

        if (pending_question[0] != '\0') {
            // Second line arrived: evaluate pair
            snprintf(multiline_buf, sizeof(multiline_buf), "%s\n%s\n", pending_question, line);
            pending_question[0] = '\0';

            Parser parser;
            parser_init(&parser, multiline_buf);
            AstNode *node = parser_parse_qa_pair(&parser);
            if (node) {
                if (workers > 1) {
                    engine_eval_program(&engine, &vm, node);
                } else {
                    vm_eval_node(&vm, node);
                    if (vm.last_output[0]) {
                        printf("%s\n", vm.last_output);
                        fflush(stdout);
                    }
                }
                ast_free(node);
            }
            multiline_buf[0] = '\0';
            continue;
        }

        // Single line statement (e.g. function call, prefix delimiter, etc.)
        Parser parser;
        parser_init(&parser, line);
        AstNode *node = parser_parse_qa_pair(&parser);
        if (node) {
            if (workers > 1) {
                engine_eval_program(&engine, &vm, node);
            } else {
                vm_eval_node(&vm, node);
                if (vm.last_output[0]) {
                    printf("%s\n", vm.last_output);
                    fflush(stdout);
                }
            }
            ast_free(node);
        }
    }

    if (workers > 1) {
        engine_shutdown(&engine);
    }
}

static void eval_and_print_ast(RimVM *vm, AstNode *node) {
    if (!node) return;
    if (node->type == NODE_BLOCK || node->type == NODE_GUARD_BLOCK || node->type == NODE_PROG_BLOCK) {
        for (size_t i = 0; i < node->block.count; ++i) {
            eval_and_print_ast(vm, node->block.statements[i]);
        }
    } else {
        vm_eval_node(vm, node);
        if (vm->last_output[0]) {
            printf("%s\n", vm->last_output);
            fflush(stdout);
        }
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
            eval_and_print_ast(&vm, program);
        }

        ast_free(program);
        free(buf);
    } else {
        run_repl(workers);
    }

    kor_nlp_shutdown();
    return 0;
}
