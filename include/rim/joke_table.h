/**
 * @file joke_table.h
 * @brief Subculture pun dictionary and dynamic in-source rule table.
 */

#ifndef RIM_JOKE_TABLE_H
#define RIM_JOKE_TABLE_H

#include "rim/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *canonical_question;
    const char *canonical_answer;
    const char *description;
} JokeEntry;

typedef struct {
    char key[128];
    char value[128];
} DynamicSymbolMap;

#define RIM_MAX_DYNAMIC_MAPS 256

void joke_clear_dynamic_maps(void);
void joke_add_dynamic_map(const char *key, const char *value);
const char *joke_resolve_dynamic_symbol(const char *key);
void joke_resolve_symbol_recursive(const char *key, char *out, size_t out_sz);
const char *joke_lookup_answer(const char *question);
size_t joke_get_count(void);

#ifdef __cplusplus
}
#endif

#endif // RIM_JOKE_TABLE_H
