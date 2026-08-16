/**
 * @file lexer.h
 * @brief UTF-8 and punchline-aware lexical analyzer for RimLang.
 */

#ifndef RIM_LEXER_H
#define RIM_LEXER_H

#include "rim/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TOK_EOF,
    TOK_QUESTION_END,  // ...? or ?
    TOK_ANSWER_END,    // . or ... or 푸훗. or 푸흡... or 후후후....
    TOK_IDENT,
    TOK_INT,
    TOK_STRING,
    TOK_COLON,
    TOK_COMMA
} TokenType;

typedef struct {
    TokenType type;
    char text[256];
    int64_t int_val;
    int line;
} Token;

typedef struct {
    const char *src;
    size_t cursor;
    int line;
} Lexer;

void lexer_init(Lexer *l, const char *src);
Token lexer_next(Lexer *l);
Token lexer_peek(Lexer *l);

#ifdef __cplusplus
}
#endif

#endif // RIM_LEXER_H
