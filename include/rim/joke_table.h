/**
 * @file joke_table.h
 * @brief Subculture pun dictionary and dynamic pun-symbol mapping table.
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

/**
 * @brief Clears and registers dynamic mappings defined in the source footer (~).
 */
void joke_clear_dynamic_maps(void);
void joke_add_dynamic_map(const char *key, const char *value);
const char *joke_resolve_dynamic_symbol(const char *key);
void joke_resolve_symbol_recursive(const char *key, char *out, size_t out_sz);

/**
 * @brief Searches for a predefined joke answer using exact or fuzzy Korean matching.
 */
const char *joke_lookup_answer(const char *question);

/**
 * @brief Validates if question and answer match the expected punchline pair.
 */
bool joke_is_valid_pair(const char *question, const char *answer);

#ifdef __cplusplus
}
#endif

#endif // RIM_JOKE_TABLE_H
