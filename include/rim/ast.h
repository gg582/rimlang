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
    NODE_PRINT,       // Expression or string output
    NODE_BLOCK,       // QA pair sequence block
    NODE_NOP
} NodeType;

typedef enum {
    PUN_OP_SUBSTRING, // 케이크가 비명을 지르면 -> "케이크" - "이크" = "케" or sound extraction "이크"
    PUN_OP_CONCAT,    // 소 + 서울 -> "소설"
    PUN_OP_RHYME,     // 카라멜 + 마끼야또 -> "카라멜마끼야또"
    PUN_OP_ANTONYM    // 통모짜핫도그 -> 요즘잘자쿨냥이
} PunComputeOp;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQ,
    OP_NEQ,
    OP_AND,
    OP_OR,
    OP_LE,
    OP_GE,
    OP_LT,
    OP_GT
} BinaryOp;

typedef struct AstNode AstNode;

struct AstNode {
    NodeType type;
    int line;
    bool is_realtime; // True when annotated with '이제'
    union {
        RimValue literal;
        char var_name[64];
        struct {
            char var_name[64];
            AstNode *expr;
        } set_stmt;
        struct {
            BinaryOp op;
            AstNode *left;
            AstNode *right;
        } binop;
        struct {
            AstNode *expr;
        } unary_not;
        struct {
            PunComputeOp op;
            char arg1[128];
            char arg2[128];
        } pun_compute;
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
        } print_stmt;
        struct {
            AstNode **statements;
            size_t count;
            size_t capacity;
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
AstNode *ast_new_block(void);
void ast_block_append(AstNode *block, AstNode *stmt);
void ast_free(AstNode *node);

#ifdef __cplusplus
}
#endif

#endif // RIM_AST_H
