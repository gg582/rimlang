/**
 * @file parser.c
 * @brief Semantic Dialogue Parser with Functions (림하하... ~ 하하...림...) & Fine-Grained Blocks.
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

AstNode *ast_new_func_def(const char *name, AstNode *body) {
    AstNode *n = calloc(1, sizeof(AstNode));
    n->type = NODE_FUNC_DEF;
    snprintf(n->func_def.func_name, sizeof(n->func_def.func_name), "%s", name ? name : "");
    n->func_def.body = body;
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
        case NODE_FUNC_DEF:
            ast_free(node->func_def.body);
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

static void skip_newlines(Parser *p) {
    while (p->current.type == TOK_NEWLINE) {
        parser_advance(p);
    }
}

static void read_line_tokens(Parser *p, char *buf, size_t buf_sz) {
    buf[0] = '\0';
    skip_newlines(p);

    while (p->current.type != TOK_EOF && p->current.type != TOK_NEWLINE) {
        if (buf[0] != '\0') {
            size_t blen = strlen(buf);
            if (blen + 1 < buf_sz && p->current.text[0] != '.' && p->current.text[0] != '?' && p->current.text[0] != '!') {
                strcat(buf, " ");
            }
        }
        strncat(buf, p->current.text, buf_sz - strlen(buf) - 1);
        parser_advance(p);
    }
    if (p->current.type == TOK_NEWLINE) {
        parser_advance(p);
    }
}

AstNode *parser_parse_qa_pair(Parser *p) {
    skip_newlines(p);
    if (p->current.type == TOK_EOF) return NULL;

    char line1[512] = {0};
    char line2[512] = {0};

    // Check for "림하하..." 함수 정의 블록 (종료: "하하..림..." or "하하...림...")
    if (strncmp(p->current.text, "림하하...", 9) == 0) {
        parser_advance(p);
        read_line_tokens(p, line1, sizeof(line1)); // function name
        AstNode *body_block = ast_new_block();
        while (p->current.type != TOK_EOF) {
            skip_newlines(p);
            if (strstr(p->current.text, "하하..림") || strstr(p->current.text, "하하...림")) {
                parser_advance(p);
                break;
            }
            AstNode *inner = parser_parse_qa_pair(p);
            if (inner) {
                ast_block_append(body_block, inner);
            }
        }
        return ast_new_func_def(line1, body_block);
    }

    // Check for "림..." 출력 지시자
    if (strcmp(p->current.text, "림...") == 0 || (strcmp(p->current.text, "림") == 0 && (strcmp(p->peek.text, "...") == 0 || strcmp(p->peek.text, "....") == 0))) {
        if (strcmp(p->current.text, "림") == 0) {
            parser_advance(p);
            parser_advance(p);
        } else {
            parser_advance(p);
        }
        read_line_tokens(p, line1, sizeof(line1));
        return ast_new_print(ast_new_literal(rim_str(line1)));
    }

    // 1. Read first line
    read_line_tokens(p, line1, sizeof(line1));

    // 2. Read paired second line
    skip_newlines(p);
    if (p->current.type != TOK_EOF &&
        strncmp(p->current.text, "교주님", 6) != 0 &&
        strncmp(p->current.text, "크흡", 6) != 0 &&
        strncmp(p->current.text, "흡...흐흡", 9) != 0 &&
        strncmp(p->current.text, "림하하", 9) != 0 &&
        !strstr(p->current.text, "하하..림") &&
        !strstr(p->current.text, "하하...림") &&
        !strstr(line1, "흘려서") && !strstr(line1, "후라이팬")) {
        
        read_line_tokens(p, line2, sizeof(line2));
    }

    // (A) 문답 구문2가 있는 경우 -> 구문2 반환
    if (line2[0] != '\0') {
        return ast_new_literal(rim_str(line2));
    }

    // (B) 단일 구문 처리
    // 1. "교주님...?" 질문식
    if (strstr(line1, "교주님") && strstr(line1, "?")) {
        return ast_new_literal(rim_str("교주님...?"));
    }

    // 2. "흡..." 입력문
    if (strstr(line1, "흡...") && !strstr(line1, "흐흡")) {
        return ast_new_input("흡...", "ans");
    }

    // 3. SBN OISC 명령어
    if (strstr(line1, "흘려서") || strstr(line1, "흘려")) {
        int64_t a = 0, b = 0, c = 0;
        const char *tok1 = line1;
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
        return ast_new_sbn(ast_new_literal(rim_int(a)), ast_new_literal(rim_int(b)), ast_new_literal(rim_int(c)));
    }

    // 4. FlipJump OISC 명령어
    if (strstr(line1, "후라이팬") || strstr(line1, "뒤집")) {
        int64_t a = 0, b = 0, c = 0;
        const char *tok1 = line1;
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
        return ast_new_flipjump(ast_new_literal(rim_int(a)), ast_new_literal(rim_int(b)), ast_new_literal(rim_int(c)));
    }

    // 5. 질문식일 경우에만 형태소/LLM 오라클 질의, 단어/변수 호출식은 literal로 평가
    if (strstr(line1, "?")) {
        NlpAnalysisResult nlp;
        if (kor_nlp_analyze(line1, &nlp) && nlp.punchline[0] != '\0') {
            return ast_new_literal(rim_str(nlp.punchline));
        }
    }

    return ast_new_literal(rim_str(line1));
}

AstNode *parser_parse_program(Parser *p) {
    kor_nlp_init();

    AstNode *root_block = ast_new_block();
    AstNode *active_block = root_block;

    while (p->current.type != TOK_EOF) {
        skip_newlines(p);
        if (p->current.type == TOK_EOF) break;

        if (strncmp(p->current.text, "크흡...", strlen("크흡...")) == 0) {
            AstNode *guard = ast_new_guard_block();
            ast_block_append(root_block, guard);
            active_block = guard;
            parser_advance(p);
            continue;
        }

        if (strcmp(p->current.text, "교주님...") == 0 && p->current.type != TOK_QUESTION_END) {
            AstNode *prog = ast_new_prog_block();
            ast_block_append(root_block, prog);
            active_block = prog;
            parser_advance(p);
            continue;
        }

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
