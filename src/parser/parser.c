/**
 * @file parser.c
 * @brief Semantic Dialogue Parser powered by Kiwi Morphological NLP Engine.
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
    return n;
}

AstNode *ast_new_block(void) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_BLOCK;
    n->block.capacity = 16;
    n->block.statements = calloc(n->block.capacity, sizeof(AstNode *));
    return n;
}

void ast_block_append(AstNode *block, AstNode *stmt) {
    if (!block || block->type != NODE_BLOCK || !stmt) return;
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
        case NODE_BLOCK:
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

    // Collect all tokens for the statement until TOK_QUESTION_END or TOK_ANSWER_END
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
        parser_advance(p);
    }

    AstNode *result = NULL;
    const char *target_sentence = q_buf;

    // 1. SBN OISC instruction
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

    // 2. FlipJump OISC instruction
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

    // 3. Kiwi Morphological NLP Engine: Evaluate question dynamically
    NlpAnalysisResult nlp;
    if (kor_nlp_analyze(target_sentence, &nlp) && nlp.punchline[0] != '\0') {
        result = ast_new_literal(rim_str(nlp.punchline));
        result->is_realtime = is_realtime;
        return result;
    }

    // Default formatted print
    result = ast_new_print(ast_new_literal(rim_str(target_sentence)));
    result->is_realtime = is_realtime;
    return result;
}

AstNode *parser_parse_program(Parser *p) {
    kor_nlp_init();
    parse_dynamic_footer_section(p->lexer.src);

    AstNode *block = ast_new_block();
    while (p->current.type != TOK_EOF) {
        AstNode *stmt = parser_parse_qa_pair(p);
        if (stmt) {
            ast_block_append(block, stmt);
        }
    }
    return block;
}
