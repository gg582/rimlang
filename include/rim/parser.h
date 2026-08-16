/**
 * @file parser.h
 * @brief QA-pair dialogue parser and sentence interpreter for RimLang.
 */

#ifndef RIM_PARSER_H
#define RIM_PARSER_H

#include "rim/ast.h"
#include "rim/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Lexer lexer;
    Token current;
    Token peek;
} Parser;

void parser_init(Parser *p, const char *src);
AstNode *parser_parse_program(Parser *p);
AstNode *parser_parse_qa_pair(Parser *p);

#ifdef __cplusplus
}
#endif

#endif // RIM_PARSER_H
