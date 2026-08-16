/**
 * @file korean_synonym.c
 * @brief Implementation of verbal synonym checks and token-level phonetic substitution.
 */

#include "rim/korean_synonym.h"
#include <ctype.h>

void kor_normalize_josa(const char *src, char *dst, size_t dst_size) {
    kor_strip_josa(src, dst, dst_size);
}

bool kor_match_suffix(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t l1 = strlen(str);
    size_t l2 = strlen(suffix);
    if (l1 < l2) return false;
    return strcmp(str + (l1 - l2), suffix) == 0;
}

bool kor_is_add_verb(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "더하") || strstr(verb, "더한") || strstr(verb, "더하면") ||
           strstr(verb, "합치") || strstr(verb, "합치면") || strstr(verb, "보태") ||
           strstr(verb, "보태면") || strstr(verb, "더해") || strstr(verb, "증가");
}

bool kor_is_sub_verb(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "빼") || strstr(verb, "뺀") || strstr(verb, "빼면") ||
           strstr(verb, "덜어") || strstr(verb, "덜어내") || strstr(verb, "깎") ||
           strstr(verb, "감소");
}

bool kor_is_mul_verb(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "곱하") || strstr(verb, "곱한") || strstr(verb, "곱하면") ||
           strstr(verb, "배가") || strstr(verb, "불리") ||
           strstr(verb, "또 지르면") || strstr(verb, "또 막기야") ||
           strstr(verb, "비벼먹으면") || strstr(verb, "또 서울에") ||
           strstr(verb, "곁들여");
}

bool kor_is_div_verb(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "나누") || strstr(verb, "나눈") || strstr(verb, "나누면") ||
           strstr(verb, "쪼개") || strstr(verb, "분할");
}

bool kor_is_sbn_flow(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "흘려서") || strstr(verb, "흘려") || strstr(verb, "흘리면") ||
           strstr(verb, "흘림");
}

bool kor_is_flip_verb(const char *verb) {
    if (!verb) return false;
    return strstr(verb, "뒤집") || strstr(verb, "뒤집으면") || strstr(verb, "뒤집어") ||
           strstr(verb, "반전");
}

typedef struct {
    const char *en;
    const char *ko;
} PhoneticEntry;

static const PhoneticEntry s_phonetic_table[] = {
    {"not", "낫"},
    {"just", "저스트"},
    {"a", "어"},
    {"scythe", "낫"},
    {"alone", "혼자"},
    {"cake", "케이크"},
    {"scream", "비명"},
    {"bread", "빵"},
    {"banana", "바나나"},
    {"hotdog", "핫도그"},
    {"laugh", "웃음"},
    {"smile", "웃음"},
    {"kick", "킥"}
};

void kor_eng_phonetic_substitute(const char *src, char *dst, size_t dst_size) {
    if (!src || !dst || dst_size == 0) return;
    dst[0] = '\0';

    const char *p = src;
    char word[64];

    while (*p) {
        if (isspace((unsigned char)*p) || ispunct((unsigned char)*p)) {
            size_t dlen = strlen(dst);
            if (dlen + 1 < dst_size) {
                dst[dlen] = *p;
                dst[dlen + 1] = '\0';
            }
            p++;
            continue;
        }

        if (isalpha((unsigned char)*p)) {
            size_t wlen = 0;
            while (isalpha((unsigned char)*p) && wlen < sizeof(word) - 1) {
                word[wlen++] = tolower((unsigned char)*p);
                p++;
            }
            word[wlen] = '\0';

            const char *sub = word;
            for (size_t i = 0; i < sizeof(s_phonetic_table) / sizeof(s_phonetic_table[0]); ++i) {
                if (strcmp(word, s_phonetic_table[i].en) == 0) {
                    sub = s_phonetic_table[i].ko;
                    break;
                }
            }

            size_t cur_len = strlen(dst);
            size_t sub_len = strlen(sub);
            if (cur_len + sub_len < dst_size) {
                strcat(dst, sub);
            }
        } else {
            size_t dlen = strlen(dst);
            if (dlen + 1 < dst_size) {
                dst[dlen] = *p;
                dst[dlen + 1] = '\0';
            }
            p++;
        }
    }
}
