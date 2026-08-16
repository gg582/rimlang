/**
 * @file joke_table.c
 * @brief Subculture pun dictionary and dynamic pun-symbol mapping implementation for RimLang.
 */

#include "rim/joke_table.h"
#include "rim/korean_nlp.h"

static DynamicSymbolMap s_dynamic_maps[RIM_MAX_DYNAMIC_MAPS];
static size_t s_dynamic_map_count = 0;

void joke_clear_dynamic_maps(void) {
    s_dynamic_map_count = 0;
}

void joke_add_dynamic_map(const char *key, const char *value) {
    if (!key || !value) return;
    for (size_t i = 0; i < s_dynamic_map_count; ++i) {
        if (strcmp(s_dynamic_maps[i].key, key) == 0) {
            snprintf(s_dynamic_maps[i].value, sizeof(s_dynamic_maps[i].value), "%s", value);
            return;
        }
    }
    if (s_dynamic_map_count < RIM_MAX_DYNAMIC_MAPS) {
        snprintf(s_dynamic_maps[s_dynamic_map_count].key, sizeof(s_dynamic_maps[s_dynamic_map_count].key), "%s", key);
        snprintf(s_dynamic_maps[s_dynamic_map_count].value, sizeof(s_dynamic_maps[s_dynamic_map_count].value), "%s", value);
        s_dynamic_map_count++;
    }
}

const char *joke_resolve_dynamic_symbol(const char *key) {
    if (!key) return NULL;
    for (size_t i = 0; i < s_dynamic_map_count; ++i) {
        if (strcmp(s_dynamic_maps[i].key, key) == 0) {
            return s_dynamic_maps[i].value;
        }
    }
    return NULL;
}

void joke_resolve_symbol_recursive(const char *key, char *out, size_t out_sz) {
    if (!key || !out || out_sz == 0) return;
    const char *curr = key;
    const char *visited[32];
    size_t v_count = 0;

    while (curr && v_count < 32) {
        visited[v_count++] = curr;
        const char *next = joke_resolve_dynamic_symbol(curr);
        if (!next || strcmp(next, curr) == 0) {
            break;
        }
        curr = next;
    }
    snprintf(out, out_sz, "%s", curr);
}

static JokeEntry s_joke_table[] = {
    {
        .canonical_question = "케이크가 지르는 비명",
        .canonical_answer = "이크",
        .description = "Cake screaming: Eek"
    },
    {
        .canonical_question = "빵 중에 가장 예의가 없는 빵",
        .canonical_answer = "메론빵",
        .description = "Impolite bread: Melon bread sticking tongue out"
    },
    {
        .canonical_question = "통모짜핫도그의 반대말",
        .canonical_answer = "요즘잘자쿨냥이",
        .description = "Opposite of Mozzarella Hotdog"
    },
    {
        .canonical_question = "왕이 넘어지면",
        .canonical_answer = "킹콩",
        .description = "King falling: King-Kong"
    },
    {
        .canonical_question = "소가 서울에 가면",
        .canonical_answer = "소설",
        .description = "Cow in Seoul: Novel (So-seol)"
    },
    {
        .canonical_question = "바나나가 웃으면",
        .canonical_answer = "바나나킥",
        .description = "Banana laughing: Banana kick"
    },
    {
        .canonical_question = "이렇게 막기야",
        .canonical_answer = "카라멜마끼야또",
        .description = "Blocking like this: Caramel Macchiato"
    },
    {
        .canonical_question = "도라지를 먹으면",
        .canonical_answer = "너도랐지",
        .description = "Eating Doraji: You went crazy?"
    },
    {
        .canonical_question = "백지 백치",
        .canonical_answer = "흑지상치",
        .description = "White paper idiot -> Black paper lettuce"
    },
    {NULL, NULL, NULL}
};

const char *joke_lookup_answer(const char *question) {
    if (!question) return NULL;

    // Check dynamic maps first
    for (size_t i = 0; i < s_dynamic_map_count; ++i) {
        if (strstr(question, s_dynamic_maps[i].key)) {
            static char resolved[128];
            joke_resolve_symbol_recursive(s_dynamic_maps[i].key, resolved, sizeof(resolved));
            return resolved;
        }
    }

    for (int i = 0; s_joke_table[i].canonical_question != NULL; ++i) {
        if (strstr(question, s_joke_table[i].canonical_question)) {
            return s_joke_table[i].canonical_answer;
        }
        if (kor_jamo_similarity(question, s_joke_table[i].canonical_question) > 0.65) {
            return s_joke_table[i].canonical_answer;
        }
    }
    return NULL;
}

bool joke_is_valid_pair(const char *question, const char *answer) {
    const char *ans = joke_lookup_answer(question);
    if (!ans) return false;
    return strstr(answer, ans) != NULL;
}
