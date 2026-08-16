/**
 * @file joke_table.c
 * @brief Dynamic in-source rule table with substring matching.
 */

#include "rim/joke_table.h"
#include "rim/korean_nlp.h"
#include <ctype.h>

static DynamicSymbolMap s_dynamic_maps[RIM_MAX_DYNAMIC_MAPS];
static size_t s_dynamic_map_count = 0;

void joke_clear_dynamic_maps(void) {
    s_dynamic_map_count = 0;
}

static void clean_key(const char *src, char *dst, size_t dst_size) {
    size_t d = 0;
    for (size_t i = 0; src[i] && d < dst_size - 1; ++i) {
        if (src[i] != '?' && src[i] != '.' && src[i] != ' ' && src[i] != '\r' && src[i] != '\n') {
            dst[d++] = src[i];
        }
    }
    dst[d] = '\0';
}

void joke_add_dynamic_map(const char *key, const char *value) {
    if (!key || !value) return;
    char clean_k[128];
    clean_key(key, clean_k, sizeof(clean_k));

    for (size_t i = 0; i < s_dynamic_map_count; ++i) {
        if (strcmp(s_dynamic_maps[i].key, clean_k) == 0) {
            snprintf(s_dynamic_maps[i].value, sizeof(s_dynamic_maps[i].value), "%s", value);
            return;
        }
    }
    if (s_dynamic_map_count < RIM_MAX_DYNAMIC_MAPS) {
        snprintf(s_dynamic_maps[s_dynamic_map_count].key, sizeof(s_dynamic_maps[s_dynamic_map_count].key), "%s", clean_k);
        snprintf(s_dynamic_maps[s_dynamic_map_count].value, sizeof(s_dynamic_maps[s_dynamic_map_count].value), "%s", value);
        s_dynamic_map_count++;
    }
}

const char *joke_resolve_dynamic_symbol(const char *key) {
    if (!key) return NULL;
    char clean_k[128];
    clean_key(key, clean_k, sizeof(clean_k));

    for (size_t i = 0; i < s_dynamic_map_count; ++i) {
        if (strcmp(s_dynamic_maps[i].key, clean_k) == 0 ||
            strstr(clean_k, s_dynamic_maps[i].key) != NULL ||
            strstr(s_dynamic_maps[i].key, clean_k) != NULL) {
            return s_dynamic_maps[i].value;
        }
    }
    return NULL;
}

void joke_resolve_symbol_recursive(const char *key, char *out, size_t out_sz) {
    if (!key || !out || out_sz == 0) return;
    const char *curr = key;
    size_t v_count = 0;

    while (curr && v_count < 32) {
        v_count++;
        const char *next = joke_resolve_dynamic_symbol(curr);
        if (!next || strcmp(next, curr) == 0) {
            break;
        }
        curr = next;
    }
    snprintf(out, out_sz, "%s", curr);
}

const char *joke_lookup_answer(const char *question) {
    return joke_resolve_dynamic_symbol(question);
}

size_t joke_get_count(void) {
    return s_dynamic_map_count;
}
