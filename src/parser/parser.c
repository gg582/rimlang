/**
 * @file parser.c
 * @brief Semantic Dialogue Parser with Guard Blocks & LLM control.
 */

#include "rim/parser.h"
#include "rim/korean_synonym.h"
#include "rim/korean_nlp.h"
#include "rim/joke_table.h"

AstNode *ast_new_literal(RimValue val) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_LITERAL;
    n->literal = val;
    return n;
}

AstNode *ast_new_var(const char *name) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_VAR;
    snprintf(n->var_name, sizeof(n->var_name), "%s", name ? name : "");
    return n;
}

AstNode *ast_new_set(const char *name, AstNode *expr) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_SET;
    snprintf(n->set_stmt.var_name, sizeof(n->set_stmt.var_name), "%s", name ? name : "");
    n->set_stmt.expr = expr;
    return n;
}

AstNode *ast_new_binop(BinaryOp op, AstNode *left, AstNode *right) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_BINOP;
    n->binop.op = op;
    n->binop.left = left;
    n->binop.right = right;
    return n;
}

AstNode *ast_new_unary_not(AstNode *expr) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_UNARY_NOT;
    n->unary_not.expr = expr;
    return n;
}

AstNode *ast_new_pun_compute(PunComputeOp op, const char *arg1, const char *arg2) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_PUN_COMPUTE;
    n->pun_compute.op = op;
    snprintf(n->pun_compute.arg1, sizeof(n->pun_compute.arg1), "%s", arg1 ? arg1 : "");
    snprintf(n->pun_compute.arg2, sizeof(n->pun_compute.arg2), "%s", arg2 ? arg2 : "");
    return n;
}

AstNode *ast_new_sbn(AstNode *a, AstNode *b, AstNode *c) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_SBN;
    n->sbn.a = a;
    n->sbn.b = b;
    n->sbn.c = c;
    return n;
}

AstNode *ast_new_flipjump(AstNode *a, AstNode *b, AstNode *c) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_FLIPJUMP;
    n->flipjump.a = a;
    n->flipjump.b = b;
    n->flipjump.c = c;
    return n;
}

AstNode *ast_new_joke(const char *query, const char *answer) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_JOKE_CALL;
    snprintf(n->joke.query, sizeof(n->joke.query), "%s", query ? query : "");
    snprintf(n->joke.answer, sizeof(n->joke.answer), "%s", answer ? answer : "");
    return n;
}

AstNode *ast_new_ternary(AstNode *cond, AstNode *then_b, AstNode *else_b) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_TERNARY;
    n->ternary.cond = cond;
    n->ternary.then_branch = then_b;
    n->ternary.else_branch = else_b;
    return n;
}

AstNode *ast_new_if(AstNode *cond, AstNode *body) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_IF;
    n->if_stmt.cond = cond;
    n->if_stmt.body = body;
    return n;
}

AstNode *ast_new_assert(AstNode *cond, const char *msg) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_ASSERT;
    n->assert_stmt.cond = cond;
    snprintf(n->assert_stmt.msg, sizeof(n->assert_stmt.msg), "%s", msg ? msg : "");
    return n;
}

AstNode *ast_new_print(AstNode *expr) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_PRINT;
    n->print_stmt.expr = expr;
    strcpy(n->print_stmt.prefix, "림...");
    return n;
}

AstNode *ast_new_input(const char *prompt, const char *target_var) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_INPUT;
    snprintf(n->input_stmt.prompt, sizeof(n->input_stmt.prompt), "%s", prompt ? prompt : "흡...");
    snprintf(n->input_stmt.target_var, sizeof(n->input_stmt.target_var), "%s", target_var ? target_var : "");
    return n;
}

AstNode *ast_new_llm_query(AstNode *expr) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_LLM_QUERY;
    n->llm_query_stmt.expr = expr;
    return n;
}

AstNode *ast_new_block(void) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_BLOCK;
    n->block.capacity = 16;
    n->block.statements = calloc(n->block.capacity, sizeof(AstNode *));
    return n;
}

AstNode *ast_new_guard_block(void) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_GUARD_BLOCK;
    n->no_llm_guard = true;
    n->block.is_guarded = true;
    n->block.capacity = 16;
    n->block.statements = calloc(n->block.capacity, sizeof(AstNode *));
    return n;
}

AstNode *ast_new_prog_block(void) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_PROG_BLOCK;
    n->block.capacity = 16;
    n->block.statements = calloc(n->block.capacity, sizeof(AstNode *));
    return n;
}

void ast_block_append(AstNode *block, AstNode *stmt) {
    if (!block || !stmt) return;
    if (block->block.count >= block->block.capacity) {
        block->block.capacity *= 2;
        block->block.statements = realloc(block->block.statements, block->block.capacity * sizeof(AstNode *));
    }
    block->block.statements[block->block.count++] = stmt;
}

void ast_free(AstNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_SET:
            ast_free(node->set_stmt.expr);
            break;
        case NODE_UNARY_NOT:
            ast_free(node->unary_not.expr);
            break;
        case NODE_BINOP:
            ast_free(node->binop.left);
            ast_free(node->binop.right);
            break;
        case NODE_SBN:
            ast_free(node->sbn.a);
            ast_free(node->sbn.b);
            ast_free(node->sbn.c);
            break;
        case NODE_FLIPJUMP:
            ast_free(node->flipjump.a);
            ast_free(node->flipjump.b);
            ast_free(node->flipjump.c);
            break;
        case NODE_TERNARY:
            ast_free(node->ternary.cond);
            ast_free(node->ternary.then_branch);
            ast_free(node->ternary.else_branch);
            break;
        case NODE_IF:
            ast_free(node->if_stmt.cond);
            ast_free(node->if_stmt.body);
            break;
        case NODE_ASSERT:
            ast_free(node->assert_stmt.cond);
            break;
        case NODE_PRINT:
            ast_free(node->print_stmt.expr);
            break;
        case NODE_LLM_QUERY:
            ast_free(node->llm_query_stmt.expr);
            break;
        case NODE_BLOCK:
        case NODE_GUARD_BLOCK:
        case NODE_PROG_BLOCK:
            for (size_t i = 0; i < node->block.count; ++i) {
                ast_free(node->block.statements[i]);
            }
            free(node->block.statements);
            break;
        default:
            break;
    }
    free(node);
}

void parser_init(Parser *p, const char *src) {
    lexer_init(&p->lexer, src);
    p->current = lexer_next(&p->lexer);
    p->peek = lexer_next(&p->lexer);
}

static void parser_advance(Parser *p) {
    p->current = p->peek;
    p->peek = lexer_next(&p->lexer);
}

static void parse_dynamic_footer_section(const char *src) {
    joke_clear_dynamic_maps();
    const char *sep = strstr(src, "~~~~~");
    if (!sep) sep = strstr(src, "~~~");
    if (!sep) return;

    sep += 3;
    while (*sep == '~' || *sep == ' ' || *sep == '\r' || *sep == '\n') sep++;

    char line_buf[256];
    const char *p = sep;
    while (*p) {
        size_t idx = 0;
        while (*p && *p != '\n' && *p != '\r' && idx < sizeof(line_buf) - 1) {
            line_buf[idx++] = *p++;
        }
        line_buf[idx] = '\0';
        while (*p == '\n' || *p == '\r') p++;

        if (line_buf[0] == '#' || (line_buf[0] == '/' && line_buf[1] == '/')) {
            continue;
        }

        if (line_buf[0]) {
            char key[128] = {0};
            char val[128] = {0};
            char *p_is = strstr(line_buf, "은");
            if (!p_is) p_is = strstr(line_buf, "는");
            if (!p_is) p_is = strstr(line_buf, "->");
            if (!p_is) p_is = strstr(line_buf, ":");

            if (p_is) {
                size_t klen = p_is - line_buf;
                strncpy(key, line_buf, klen);
                key[klen] = '\0';

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
                while (vlen > 0 && (val[vlen-1] == '.' || val[vlen-1] == ' ' || val[vlen-1] == '\r')) {
                    val[--vlen] = '\0';
                }

                size_t key_len = strlen(key);
                while (key_len > 0 && (key[key_len-1] == ' ')) key[--key_len] = '\0';

                joke_add_dynamic_map(key, val);
            }
        }
    }
}

AstNode *parser_parse_qa_pair(Parser *p) {
    if (p->current.type == TOK_EOF) return NULL;

    if (strncmp(p->current.text, "~~~", 3) == 0) {
        while (p->current.type != TOK_EOF) {
            parser_advance(p);
        }
        return NULL;
    }

    char q_buf[512] = {0};
    bool is_realtime = false;
    bool is_print_line = false;

    if (strcmp(p->current.text, "림") == 0 && (strcmp(p->peek.text, "...") == 0 || strcmp(p->peek.text, "....") == 0)) {
        is_print_line = true;
        parser_advance(p);
        parser_advance(p);
    }

    int start_line = p->current.line;

    while (p->current.type != TOK_EOF) {
        if (strncmp(p->current.text, "~~~", 3) == 0) {
            while (p->current.type != TOK_EOF) parser_advance(p);
            break;
        }
        if (strcmp(p->current.text, "이제") == 0) {
            is_realtime = true;
        }
        if (strlen(q_buf) + strlen(p->current.text) + 2 < sizeof(q_buf)) {
            if (q_buf[0] != '\0') strcat(q_buf, " ");
            strcat(q_buf, p->current.text);
        }
        if (p->current.type == TOK_QUESTION_END || p->current.type == TOK_ANSWER_END) {
            parser_advance(p);
            break;
        }
        if (p->peek.line > start_line) {
            parser_advance(p);
            break;
        }
        parser_advance(p);
    }

    AstNode *result = NULL;
    const char *target_sentence = q_buf;

    // 1. "림..." 출력문
    if (is_print_line) {
        result = ast_new_print(ast_new_literal(rim_str(target_sentence)));
        result->is_realtime = is_realtime;
        return result;
    }

    // 2. "교주님...?" 질문식
    if (strstr(target_sentence, "교주님") && (strstr(target_sentence, "?") || strstr(target_sentence, "...?"))) {
        result = ast_new_literal(rim_str("교주님...?"));
        result->is_realtime = is_realtime;
        return result;
    }

    // 3. "흡..." 입력문
    if (strstr(target_sentence, "흡...") && !strstr(target_sentence, "흐흡")) {
        result = ast_new_input("흡...", "ans");
        result->is_realtime = is_realtime;
        return result;
    }

    // 4. SBN OISC 명령어
    if (strstr(target_sentence, "흘려서") || strstr(target_sentence, "흘려")) {
        int64_t a = 0, b = 0, c = 0;
        const char *tok1 = target_sentence;
        while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
        if (*tok1) {
            a = strtoll(tok1, (char **)&tok1, 10);
            while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
            if (*tok1) {
                b = strtoll(tok1, (char **)&tok1, 10);
                while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
                if (*tok1) {
                    c = strtoll(tok1, (char **)&tok1, 10);
                }
            }
        }
        result = ast_new_sbn(ast_new_literal(rim_int(a)), ast_new_literal(rim_int(b)), ast_new_literal(rim_int(c)));
        result->is_realtime = is_realtime;
        return result;
    }

    // 5. FlipJump OISC 명령어
    if (strstr(target_sentence, "후라이팬") || strstr(target_sentence, "뒤집")) {
        int64_t a = 0, b = 0, c = 0;
        const char *tok1 = target_sentence;
        while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
        if (*tok1) {
            a = strtoll(tok1, (char **)&tok1, 10);
            while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
            if (*tok1) {
                b = strtoll(tok1, (char **)&tok1, 10);
                while (*tok1 && !isdigit((unsigned char)*tok1)) tok1++;
                if (*tok1) {
                    c = strtoll(tok1, (char **)&tok1, 10);
                }
            }
        }
        result = ast_new_flipjump(ast_new_literal(rim_int(a)), ast_new_literal(rim_int(b)), ast_new_literal(rim_int(c)));
        result->is_realtime = is_realtime;
        return result;
    }

    // 6. Kiwi 형태소 및 LLM 질의 연동
    NlpAnalysisResult nlp;
    if (kor_nlp_analyze(target_sentence, &nlp) && nlp.punchline[0] != '\0') {
        result = ast_new_literal(rim_str(nlp.punchline));
        result->is_realtime = is_realtime;
        return result;
    }

    result = ast_new_print(ast_new_literal(rim_str(target_sentence)));
    result->is_realtime = is_realtime;
    return result;
}

AstNode *parser_parse_program(Parser *p) {
    kor_nlp_init();
    parse_dynamic_footer_section(p->lexer.src);

    AstNode *root_block = ast_new_block();
    AstNode *active_block = root_block;

    while (p->current.type != TOK_EOF) {
        // "크흡..." 시작 블록 감지
        if (strncmp(p->current.text, "크흡...", strlen("크흡...")) == 0) {
            AstNode *guard = ast_new_guard_block();
            ast_block_append(root_block, guard);
            active_block = guard;
            parser_advance(p);
            continue;
        }

        // "교주님..." 시작 블록 감지 (단, 교주님...? 질문식이 아닌 경우)
        if (strcmp(p->current.text, "교주님...") == 0 && p->current.type != TOK_QUESTION_END) {
            AstNode *prog = ast_new_prog_block();
            ast_block_append(root_block, prog);
            active_block = prog;
            parser_advance(p);
            continue;
        }

        // "흡...흐흡..." 블록 종료 감지
        if (strstr(p->current.text, "흐흡") || strstr(p->current.text, "흡...흐흡")) {
            active_block = root_block;
            parser_advance(p);
            continue;
        }

        AstNode *stmt = parser_parse_qa_pair(p);
        if (stmt) {
            if (active_block->no_llm_guard) {
                stmt->no_llm_guard = true;
            }
            ast_block_append(active_block, stmt);
        }
    }
    return root_block;
}
