/**
 * @file korean_synonym.h
 * @brief Semantic synonym mapping and verbal morphology rules for RimLang.
 */

#ifndef RIM_KOREAN_SYNONYM_H
#define RIM_KOREAN_SYNONYM_H

#include "rim/common.h"
#include "rim/korean_nlp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Normalizes Korean noun particles.
 */
void kor_normalize_josa(const char *src, char *dst, size_t dst_size);

/**
 * @brief Checks if a string ends with the specified suffix.
 */
bool kor_match_suffix(const char *str, const char *suffix);

/**
 * @brief Tests if the verb corresponds to addition.
 */
bool kor_is_add_verb(const char *verb);

/**
 * @brief Tests if the verb corresponds to subtraction.
 */
bool kor_is_sub_verb(const char *verb);

/**
 * @brief Tests if the verb corresponds to multiplication.
 */
bool kor_is_mul_verb(const char *verb);

/**
 * @brief Tests if the verb corresponds to division.
 */
bool kor_is_div_verb(const char *verb);

/**
 * @brief Tests if the verb represents SBN OISC flow.
 */
bool kor_is_sbn_flow(const char *verb);

/**
 * @brief Tests if the verb represents FlipJump bit inversion.
 */
bool kor_is_flip_verb(const char *verb);

/**
 * @brief Translates phonetic loanwords to standardized English/tokens (e.g. 낫 저스트 어 낫).
 */
void kor_eng_phonetic_substitute(const char *src, char *dst, size_t dst_size);

#ifdef __cplusplus
}
#endif

#endif // RIM_KOREAN_SYNONYM_H
