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

static void skip_whitespace(Lexer *l) {
    while (l->src[l->cursor]) {
        char c = l->src[l->cursor];
        if (c == '\n') {
            l->line++;
            l->cursor++;
        } else if (isspace((unsigned char)c)) {
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
    skip_whitespace(l);
    Token tok;
    memset(&tok, 0, sizeof(tok));
    tok.line = l->line;

    if (!l->src[l->cursor]) {
        tok.type = TOK_EOF;
        return tok;
    }

    const char *curr = &l->src[l->cursor];

    // Check for question terminators (...? or ?) first
    if (strncmp(curr, "...?", 4) == 0) {
        tok.type = TOK_QUESTION_END;
        strcpy(tok.text, "...?");
        l->cursor += 4;
        return tok;
    }
    if (curr[0] == '?') {
        tok.type = TOK_QUESTION_END;
        strcpy(tok.text, "?");
        l->cursor += 1;
        return tok;
    }

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

    // Check for punchline laughter & punctuation terminators
    const char *terminators[] = {
        "후후후....", "푸흐흐흐....", "푸흡...", "푸훗.", "풉.", "....", "..."
    };
    for (size_t i = 0; i < sizeof(terminators)/sizeof(terminators[0]); ++i) {
        size_t len = strlen(terminators[i]);
        if (strncmp(curr, terminators[i], len) == 0) {
            tok.type = TOK_ANSWER_END;
            strncpy(tok.text, terminators[i], sizeof(tok.text) - 1);
            l->cursor += len;
            return tok;
        }
    }

    if (curr[0] == ':') {
        tok.type = TOK_COLON;
        tok.text[0] = ':';
        l->cursor++;
        return tok;
    }
    if (curr[0] == ',') {
        tok.type = TOK_COMMA;
        tok.text[0] = ',';
        l->cursor++;
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

    // Identifiers and Korean words
    tok.type = TOK_IDENT;
    size_t idx = 0;
    while (l->src[l->cursor] && !isspace((unsigned char)l->src[l->cursor])) {
        char c = l->src[l->cursor];
        if (c == '?' || c == ':' || c == ',' || c == '"') break;
        if (c == '.' && l->src[l->cursor+1] == '.' && l->src[l->cursor+2] == '.') break;
        if (idx < sizeof(tok.text) - 1) {
            tok.text[idx++] = c;
        }
        l->cursor++;
    }
    tok.text[idx] = '\0';
    return tok;
}

Token lexer_peek(Lexer *l) {
    Lexer copy = *l;
    return lexer_next(&copy);
}
