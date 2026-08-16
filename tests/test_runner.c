#include "rim/common.h"
#include "rim/ast.h"
#include "rim/parser.h"
#include "rim/runtime.h"
#include "rim/engine.h"
#include "rim/korean_nlp.h"

static void test_joke_eval(void) {
    const char *src = "케이크가 지르는 비명은...?\n";
    Parser p;
    parser_init(&p, src);
    AstNode *prog = parser_parse_program(&p);
    assert(prog != NULL);

    RimVM vm;
    vm_init(&vm);
    RimValue val = vm_eval_node(&vm, prog);
    assert(val.type == VAL_STR);
    assert(strstr(val.s, "이크") != NULL);
    ast_free(prog);
    printf("[PASS] test_joke_eval: %s\n", val.s);
}

static void test_sbn_oisc(void) {
    const char *src = "10에서 4를 흘려서 0이 되면...\n";
    Parser p;
    parser_init(&p, src);
    AstNode *prog = parser_parse_program(&p);
    assert(prog != NULL);

    RimVM vm;
    vm_init(&vm);
    vm.memory[10] = 10;
    vm.memory[4] = 4;
    RimValue val = vm_eval_node(&vm, prog);
    assert(val.type == VAL_INT);
    assert(val.i == 6);
    assert(vm.memory[10] == 6);
    ast_free(prog);
    printf("[PASS] test_sbn_oisc: memory[10] = %ld\n", vm.memory[10]);
}

static void test_ternary(void) {
    const char *src = "해골 가면을 쓴 림과, 가면을 안 쓴 림으로 나누어 게임을 했지요...?\n";
    Parser p;
    parser_init(&p, src);
    AstNode *prog = parser_parse_program(&p);
    assert(prog != NULL);

    RimVM vm;
    vm_init(&vm);
    RimValue val = vm_eval_node(&vm, prog);
    assert(val.type == VAL_STR);
    assert(strstr(val.s, "낯.가.림") != NULL);
    ast_free(prog);
    printf("[PASS] test_ternary: %s\n", val.s);
}

static void test_engine_multiprocess(void) {
    printf("--- Multi-process Engine Run ---\n");
    RimEngine engine;
    engine_init(&engine, 2);

    const char *src = "케이크가 지르는 비명은...?\n";
    Parser p;
    parser_init(&p, src);
    AstNode *prog = parser_parse_program(&p);
    assert(prog != NULL);

    RimVM vm;
    vm_init(&vm);
    engine_eval_program(&engine, &vm, prog);
    ast_free(prog);
    engine_shutdown(&engine);
    printf("[PASS] test_engine_multiprocess completed.\n");
}

int main(void) {
    printf("=== RimLang Test Runner ===\n");
    kor_nlp_init();
    test_joke_eval();
    test_sbn_oisc();
    test_ternary();
    test_engine_multiprocess();
    kor_nlp_shutdown();
    printf("All RimLang tests passed successfully!\n");
    return 0;
}
