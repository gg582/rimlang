/**
 * @file lexer.c
 * @brief Tokenizer for RimLang Korean Esoteric Language.
 */

#include "rim/lexer.h"

void lexer_init(Lexer *l, const char *src) {
    l->src = src ? src : "";
    l->cursor = 0;
    l->line = 1;
}

static void skip_spaces_and_comments(Lexer *l) {
    while (l->src[l->cursor]) {
        char c = l->src[l->cursor];
        if (c == ' ' || c == '\t' || c == '\r') {
            l->cursor++;
        } else if (c == '#' || (c == '/' && l->src[l->cursor+1] == '/')) {
            while (l->src[l->cursor] && l->src[l->cursor] != '\n') {
                l->cursor++;
            }
        } else {
            break;
        }
    }
}

Token lexer_next(Lexer *l) {
    skip_spaces_and_comments(l);
    Token tok;
    memset(&tok, 0, sizeof(tok));
    tok.line = l->line;

    if (!l->src[l->cursor]) {
        tok.type = TOK_EOF;
        return tok;
    }

    if (l->src[l->cursor] == '\n') {
        tok.type = TOK_NEWLINE;
        tok.text[0] = '\n';
        tok.text[1] = '\0';
        l->line++;
        l->cursor++;
        return tok;
    }

    const char *curr = &l->src[l->cursor];

    // Check for block keywords (교주님..., 크흡..., 흡...흐흡...)
    if (strncmp(curr, "교주님...", strlen("교주님...")) == 0 && curr[strlen("교주님...")] != '?') {
        tok.type = TOK_IDENT;
        strcpy(tok.text, "교주님...");
        l->cursor += strlen("교주님...");
        return tok;
    }
    if (strncmp(curr, "크흡...", strlen("크흡...")) == 0) {
        tok.type = TOK_IDENT;
        strcpy(tok.text, "크흡...");
        l->cursor += strlen("크흡...");
        return tok;
    }
    if (strncmp(curr, "흡...흐흡...", strlen("흡...흐흡...")) == 0) {
        tok.type = TOK_IDENT;
        strcpy(tok.text, "흡...흐흡...");
        l->cursor += strlen("흡...흐흡...");
        return tok;
    }

    // String literals
    if (curr[0] == '"') {
        tok.type = TOK_STRING;
        l->cursor++;
        size_t idx = 0;
        while (l->src[l->cursor] && l->src[l->cursor] != '"' && idx < sizeof(tok.text) - 1) {
            if (l->src[l->cursor] == '\\' && l->src[l->cursor+1]) {
                l->cursor++;
            }
            tok.text[idx++] = l->src[l->cursor++];
        }
        if (l->src[l->cursor] == '"') l->cursor++;
        tok.text[idx] = '\0';
        return tok;
    }

    // Integer literals
    if (isdigit((unsigned char)curr[0]) || (curr[0] == '-' && isdigit((unsigned char)curr[1]))) {
        tok.type = TOK_INT;
        char *endptr;
        tok.int_val = strtoll(curr, &endptr, 10);
        size_t len = endptr - curr;
        strncpy(tok.text, curr, len);
        tok.text[len] = '\0';
        l->cursor += len;
        return tok;
    }

    // Identifiers and Korean words (including punctuation inside sentences)
    tok.type = TOK_IDENT;
    size_t idx = 0;
    while (l->src[l->cursor] && !isspace((unsigned char)l->src[l->cursor]) && l->src[l->cursor] != '\n') {
        if (idx < sizeof(tok.text) - 1) {
            tok.text[idx++] = l->src[l->cursor];
        }
        l->cursor++;
    }
    tok.text[idx] = '\0';

    if (strstr(tok.text, "?")) {
        tok.type = TOK_QUESTION_END;
    }
    return tok;
}

Token lexer_peek(Lexer *l) {
    Lexer copy = *l;
    return lexer_next(&copy);
}
