/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) node definitions for RimLang.
 */

#ifndef RIM_AST_H
#define RIM_AST_H

#include "rim/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NODE_LITERAL,
    NODE_VAR,
    NODE_SET,
    NODE_BINOP,       // +, -, *, /, ==, !=, AND, OR
    NODE_UNARY_NOT,   // ~의 반대말은? (Logical Negation via Opposites)
    NODE_PUN_COMPUTE, // Morphological / Phonological pun computation expression
    NODE_SBN,         // a, b, c : mem[a] -= mem[b]; if (mem[a] <= 0) jump c
    NODE_FLIPJUMP,    // a, b, c : mem[a] ^= (1 << b); jump c
    NODE_JOKE_CALL,   // Predefined joke query and punchline
    NODE_TERNARY,     // 낯.가.림 (Condition ? X : Y)
    NODE_IF,          // Conditional statement (~ㄹ 때, ~면, ~니까)
    NODE_ASSERT,      // Assert listener (낫 저스트 어 낫)
    NODE_PRINT,       // Expression or string output ("림...")
    NODE_INPUT,       // Interactive user input ("흡...")
    NODE_GUARD_BLOCK, // "크흡..." ~ "흡...흐흡..." 결정론 가드 블록 (LLM 평가 완전 차단)
    NODE_PROG_BLOCK,  // "교주님..." ~ "흡...흐흡..." 결정론적 산식 실행 블록
    NODE_LLM_QUERY,   // Turing string passed to LLM oracle for solution
    NODE_BLOCK,       // Sequence block
    NODE_NOP
} NodeType;

typedef enum {
    PUN_OP_SUBSTRING,
    PUN_OP_CONCAT,
    PUN_OP_RHYME,
    PUN_OP_ANTONYM
} PunComputeOp;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_EQ, OP_NEQ, OP_AND, OP_OR,
    OP_LE, OP_GE, OP_LT, OP_GT
} BinaryOp;

typedef struct AstNode AstNode;

struct AstNode {
    NodeType type;
    int line;
    bool is_realtime;
    bool no_llm_guard; // true when enclosed in 크흡... ~ 흡...흐흡...

    union {
        RimValue literal;
        char var_name[64];

        struct {
            char var_name[64];
            AstNode *expr;
        } set_stmt;

        struct {
            AstNode *expr;
        } unary_not;

        struct {
            PunComputeOp op;
            char arg1[128];
            char arg2[128];
        } pun_compute;

        struct {
            BinaryOp op;
            AstNode *left;
            AstNode *right;
        } binop;

        struct {
            AstNode *a;
            AstNode *b;
            AstNode *c;
        } sbn;

        struct {
            AstNode *a;
            AstNode *b;
            AstNode *c;
        } flipjump;

        struct {
            char query[256];
            char answer[256];
        } joke;

        struct {
            AstNode *cond;
            AstNode *then_branch;
            AstNode *else_branch;
        } ternary;

        struct {
            AstNode *cond;
            AstNode *body;
        } if_stmt;

        struct {
            AstNode *cond;
            char msg[256];
        } assert_stmt;

        struct {
            AstNode *expr;
            char prefix[32]; // e.g. "림..."
        } print_stmt;

        struct {
            char prompt[64]; // e.g. "흡..."
            char target_var[64];
        } input_stmt;

        struct {
            AstNode *expr;
        } llm_query_stmt;

        struct {
            AstNode **statements;
            size_t count;
            size_t capacity;
            bool is_guarded; // true if LLM bypass is strictly enforced
        } block;
    };
};

AstNode *ast_new_literal(RimValue val);
AstNode *ast_new_var(const char *name);
AstNode *ast_new_set(const char *name, AstNode *expr);
AstNode *ast_new_binop(BinaryOp op, AstNode *left, AstNode *right);
AstNode *ast_new_unary_not(AstNode *expr);
AstNode *ast_new_pun_compute(PunComputeOp op, const char *arg1, const char *arg2);
AstNode *ast_new_sbn(AstNode *a, AstNode *b, AstNode *c);
AstNode *ast_new_flipjump(AstNode *a, AstNode *b, AstNode *c);
AstNode *ast_new_joke(const char *query, const char *answer);
AstNode *ast_new_ternary(AstNode *cond, AstNode *then_b, AstNode *else_b);
AstNode *ast_new_if(AstNode *cond, AstNode *body);
AstNode *ast_new_assert(AstNode *cond, const char *msg);
AstNode *ast_new_print(AstNode *expr);
AstNode *ast_new_input(const char *prompt, const char *target_var);
AstNode *ast_new_llm_query(AstNode *expr);
AstNode *ast_new_block(void);
AstNode *ast_new_guard_block(void);
AstNode *ast_new_prog_block(void);
void ast_block_append(AstNode *block, AstNode *stmt);
void ast_free(AstNode *node);

#ifdef __cplusplus
}
#endif

#endif // RIM_AST_H
