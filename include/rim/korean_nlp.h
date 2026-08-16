/**
 * @file korean_nlp.h
 * @brief Integration layer for real Korean NLP engine (Kiwi Morphological Analyzer)
 *        combined with syllable Jamo decomposition and semantic predicate classification.
 */

#ifndef RIM_KOREAN_NLP_H
#define RIM_KOREAN_NLP_H

#include "rim/common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HANGUL_SYLLABLE_BASE 0xAC00
#define HANGUL_SYLLABLE_END  0xD7A3
#define HANGUL_JUNG_COUNT    21
#define HANGUL_JONG_COUNT    28

typedef struct {
    uint32_t choseong;
    uint32_t jungseong;
    uint32_t jongseong;
    bool is_hangul;
} HangulChar;

typedef struct {
    char form[64];
    char tag[16];
} NlpMorpheme;

typedef struct {
    char raw[512];
    NlpMorpheme morphemes[64];
    size_t morpheme_count;
    char nouns[16][64];
    size_t noun_count;
    char verbs[16][64];
    size_t verb_count;
    char punchline[256]; // Dynamically synthesized punchline from NLP
} NlpAnalysisResult;

/**
 * @brief Initializes real Kiwi NLP Bridge subprocess for true morphological analysis.
 */
void kor_nlp_init(void);
void kor_nlp_shutdown(void);

/**
 * @brief Analyzes input sentence using the Kiwi Korean Morphological Analyzer.
 */
bool kor_nlp_analyze(const char *sentence, NlpAnalysisResult *out);

/**
 * @brief Unicode Hangul syllable decomposition helpers.
 */
HangulChar kor_decompose_char(uint32_t codepoint);
uint32_t kor_next_utf8_char(const char **src);
double kor_jamo_similarity(const char *s1, const char *s2);
void kor_strip_josa(const char *word, char *out, size_t out_sz);

#ifdef __cplusplus
}
#endif

#endif // RIM_KOREAN_NLP_H
