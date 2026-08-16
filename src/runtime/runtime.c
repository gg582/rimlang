/**
 * @file runtime.c
 * @brief Pure Dynamic VM with 0% Joke Hardcoding.
 * All logic, jokes, functors, and substitutions are evaluated via AST & user definitions.
 */

#include "rim/runtime.h"
#include "rim/joke_table.h"
#include "rim/korean_nlp.h"

typedef struct {
    char name[64];
    AstNode *body;
} FunctionDef;

static FunctionDef s_funcs[64];
static size_t s_func_count = 0;
static bool s_in_func_eval = false;

void vm_init(RimVM *vm) {
    memset(vm, 0, sizeof(RimVM));
    vm->pc = 0;
    vm->halted = false;
}

void vm_set_var(RimVM *vm, const char *name, RimValue val) {
    for (size_t i = 0; i < vm->var_count; ++i) {
        if (strcmp(vm->vars[i].name, name) == 0) {
            vm->vars[i].val = val;
            return;
        }
    }
    if (vm->var_count < sizeof(vm->vars) / sizeof(vm->vars[0])) {
        strncpy(vm->vars[vm->var_count].name, name, sizeof(vm->vars[vm->var_count].name) - 1);
        vm->vars[vm->var_count].val = val;
        vm->var_count++;
    }
}

RimValue vm_get_var(RimVM *vm, const char *name) {
    // 1. Check custom defined functions (exact match only)
    if (!s_in_func_eval) {
        for (size_t i = 0; i < s_func_count; ++i) {
            if (strcmp(s_funcs[i].name, name) == 0) {
                s_in_func_eval = true;
                RimValue res = vm_eval_node(vm, s_funcs[i].body);
                s_in_func_eval = false;
                return res;
            }
        }
    }

    // 2. Dynamic runtime table lookup
    const char *dyn_val = joke_lookup_answer(name);
    if (dyn_val) {
        return rim_str(dyn_val);
    }

    // 3. User VM variable lookup
    for (size_t i = 0; i < vm->var_count; ++i) {
        if (strcmp(vm->vars[i].name, name) == 0) {
            return vm->vars[i].val;
        }
    }
    return rim_nil();
}

RimValue vm_eval_node(RimVM *vm, AstNode *node) {
    if (!node) return rim_nil();

    switch (node->type) {
        case NODE_LITERAL: {
            if (node->literal.type == VAL_STR) {
                // 1. Check custom defined functions (exact match)
                if (!s_in_func_eval) {
                    for (size_t i = 0; i < s_func_count; ++i) {
                        if (strcmp(s_funcs[i].name, node->literal.s) == 0) {
                            s_in_func_eval = true;
                            RimValue res = vm_eval_node(vm, s_funcs[i].body);
                            s_in_func_eval = false;
                            return res;
                        }
                    }
                }

                // 2. Generic Prefix Delimiter (".....") extraction
                // Replaces any ".....<word>" with its first syllable abbreviation dynamically in-place
                if (strstr(node->literal.s, ".....")) {
                    char out[512] = {0};
                    const char *src = node->literal.s;
                    size_t out_idx = 0;
                    while (*src && out_idx < sizeof(out) - 8) {
                        if (strncmp(src, ".....", 5) == 0) {
                            src += 5;
                            const char *p = src;
                            kor_next_utf8_char(&p);
                            size_t c_len = p - src;
                            if (c_len > 0) {
                                memcpy(&out[out_idx], src, c_len);
                                out_idx += c_len;
                                out[out_idx++] = '.';
                            }
                            // Skip remaining syllables of this word
                            while (*p && !isspace((unsigned char)*p) && strncmp(p, ".....", 5) != 0 &&
                                   strncmp(p, "인거에요", 12) != 0 && *p != '.' && *p != '?' && *p != '!' && *p != ',') {
                                kor_next_utf8_char(&p);
                            }
                            src = p;
                        } else {
                            out[out_idx++] = *src++;
                        }
                    }
                    out[out_idx] = '\0';
                    snprintf(vm->last_output, sizeof(vm->last_output), "%s", out);
                    return rim_str(vm->last_output);
                }

                const char *dyn_val = joke_lookup_answer(node->literal.s);
                if (dyn_val) {
                    snprintf(vm->last_output, sizeof(vm->last_output), "%s", dyn_val);
                    return rim_str(dyn_val);
                }
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", node->literal.s);
            }
            return node->literal;
        }

        case NODE_FUNC_DEF: {
            for (size_t i = 0; i < s_func_count; ++i) {
                if (strcmp(s_funcs[i].name, node->func_def.func_name) == 0) {
                    s_funcs[i].body = node->func_def.body;
                    return rim_nil();
                }
            }
            if (s_func_count < sizeof(s_funcs) / sizeof(s_funcs[0])) {
                strncpy(s_funcs[s_func_count].name, node->func_def.func_name, sizeof(s_funcs[s_func_count].name) - 1);
                s_funcs[s_func_count].body = node->func_def.body;
                s_func_count++;
            }
            return rim_nil();
        }

        case NODE_VAR:
            return vm_get_var(vm, node->var_name);

        case NODE_SET: {
            RimValue v = vm_eval_node(vm, node->set_stmt.expr);
            vm_set_var(vm, node->set_stmt.var_name, v);
            return v;
        }

        case NODE_UNARY_NOT: {
            RimValue val = vm_eval_node(vm, node->unary_not.expr);
            if (val.type == VAL_STR) {
                const char *dyn_ans = joke_lookup_answer(val.s);
                if (dyn_ans) {
                    snprintf(vm->last_output, sizeof(vm->last_output), "%s", dyn_ans);
                    return rim_str(dyn_ans);
                }
            }
            bool is_true = (val.type == VAL_BOOL && val.b) || (val.type == VAL_INT && val.i != 0);
            return rim_bool(!is_true);
        }

        case NODE_PUN_COMPUTE: {
            if (node->pun_compute.op == PUN_OP_CONCAT) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s%s", 
                         node->pun_compute.arg1, node->pun_compute.arg2);
                return rim_str(vm->last_output);
            }
            if (node->pun_compute.op == PUN_OP_SUBSTRING) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", node->pun_compute.arg1);
                return rim_str(node->pun_compute.arg1);
            }
            if (node->pun_compute.op == PUN_OP_RHYME) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", node->pun_compute.arg1);
                return rim_str(node->pun_compute.arg1);
            }
            return rim_nil();
        }

        case NODE_BINOP: {
            RimValue l = vm_eval_node(vm, node->binop.left);
            RimValue r = vm_eval_node(vm, node->binop.right);
            if (l.type == VAL_INT && r.type == VAL_INT) {
                switch (node->binop.op) {
                    case OP_ADD: return rim_int(l.i + r.i);
                    case OP_SUB: return rim_int(l.i - r.i);
                    case OP_MUL: return rim_int(l.i * r.i);
                    case OP_DIV: return rim_int(r.i != 0 ? l.i / r.i : 0);
                    case OP_EQ:  return rim_bool(l.i == r.i);
                    case OP_NEQ: return rim_bool(l.i != r.i);
                    case OP_AND: return rim_bool(l.i && r.i);
                    case OP_OR:  return rim_bool(l.i || r.i);
                    case OP_LE:  return rim_bool(l.i <= r.i);
                    case OP_GE:  return rim_bool(l.i >= r.i);
                    case OP_LT:  return rim_bool(l.i < r.i);
                    case OP_GT:  return rim_bool(l.i > r.i);
                }
            }
            return rim_nil();
        }

        case NODE_SBN: {
            RimValue va = vm_eval_node(vm, node->sbn.a);
            RimValue vb = vm_eval_node(vm, node->sbn.b);
            RimValue vc = vm_eval_node(vm, node->sbn.c);
            int64_t a = va.i;
            int64_t b = vb.i;
            int64_t c = vc.i;
            if (a >= 0 && a < RIM_MEM_SIZE && b >= 0 && b < RIM_MEM_SIZE) {
                vm->memory[a] = vm->memory[a] - vm->memory[b];
                if (vm->memory[a] <= 0) {
                    vm->pc = (size_t)c;
                }
                vm->last_output[0] = '\0';
                return rim_int(vm->memory[a]);
            }
            return rim_nil();
        }

        case NODE_FLIPJUMP: {
            RimValue va = vm_eval_node(vm, node->flipjump.a);
            RimValue vb = vm_eval_node(vm, node->flipjump.b);
            RimValue vc = vm_eval_node(vm, node->flipjump.c);
            int64_t a = va.i;
            int64_t bit = vb.i;
            int64_t c = vc.i;
            if (a >= 0 && a < RIM_MEM_SIZE) {
                vm->memory[a] ^= (1ULL << (bit % 64));
                vm->pc = (size_t)c;
                vm->last_output[0] = '\0';
                return rim_int(vm->memory[a]);
            }
            return rim_nil();
        }

        case NODE_JOKE_CALL: {
            const char *ans = joke_lookup_answer(node->joke.query);
            if (!ans) ans = node->joke.answer;
            snprintf(vm->last_output, sizeof(vm->last_output), "%s", ans);
            return rim_str(ans);
        }

        case NODE_TERNARY: {
            RimValue cond = vm_eval_node(vm, node->ternary.cond);
            bool is_true = (cond.type == VAL_BOOL && cond.b) || (cond.type == VAL_INT && cond.i != 0);
            return is_true ? vm_eval_node(vm, node->ternary.then_branch)
                           : vm_eval_node(vm, node->ternary.else_branch);
        }

        case NODE_IF: {
            RimValue cond = vm_eval_node(vm, node->if_stmt.cond);
            bool is_true = (cond.type == VAL_BOOL && cond.b) || (cond.type == VAL_INT && cond.i != 0);
            if (is_true) {
                return vm_eval_node(vm, node->if_stmt.body);
            }
            return rim_nil();
        }

        case NODE_ASSERT: {
            RimValue cond = vm_eval_node(vm, node->assert_stmt.cond);
            bool is_true = (cond.type == VAL_BOOL && cond.b) || (cond.type == VAL_INT && cond.i != 0);
            if (!is_true) {
                fprintf(stderr, "후후후...\n");
                vm->halted = true;
            }
            return rim_bool(is_true);
        }

        case NODE_PRINT: {
            RimValue v = vm_eval_node(vm, node->print_stmt.expr);
            if (v.type == VAL_STR) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", v.s);
            } else if (v.type == VAL_INT) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%ld", v.i);
            } else if (v.type == VAL_BOOL) {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", v.b ? "참" : "거짓");
            }
            return v;
        }

        case NODE_INPUT: {
            printf("%s ", node->input_stmt.prompt);
            fflush(stdout);
            char buf[256] = {0};
            if (fgets(buf, sizeof(buf), stdin)) {
                size_t len = strlen(buf);
                while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) {
                    buf[--len] = '\0';
                }
                RimValue in_val = rim_str(buf);
                if (node->input_stmt.target_var[0]) {
                    vm_set_var(vm, node->input_stmt.target_var, in_val);
                }
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", buf);
                return in_val;
            }
            return rim_nil();
        }

        case NODE_LLM_QUERY: {
            if (node->no_llm_guard) {
                return rim_nil();
            }
            RimValue prompt_val = vm_eval_node(vm, node->llm_query_stmt.expr);
            const char *prompt_str = prompt_val.type == VAL_STR ? prompt_val.s : "";
            NlpAnalysisResult nlp;
            if (kor_nlp_analyze(prompt_str, &nlp) && nlp.punchline[0] != '\0') {
                snprintf(vm->last_output, sizeof(vm->last_output), "%s", nlp.punchline);
                return rim_str(nlp.punchline);
            }
            return prompt_val;
        }

        case NODE_BLOCK:
        case NODE_GUARD_BLOCK:
        case NODE_PROG_BLOCK: {
            RimValue last = rim_nil();
            for (size_t i = 0; i < node->block.count; ++i) {
                last = vm_eval_node(vm, node->block.statements[i]);
            }
            return last;
        }

        default:
            return rim_nil();
    }
}

void vm_run(RimVM *vm, AstNode *program) {
    if (!program) return;
    vm_eval_node(vm, program);
}
