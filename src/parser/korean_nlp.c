/**
 * @file korean_nlp.c
 * @brief Subprocess IPC integration with Kiwi Korean Morphological Analyzer
 *        and Hangul Jamo phonological decomposition.
 */

#include "rim/korean_nlp.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static int s_nlp_in_pipe[2] = {-1, -1};  // Parent writes, Python reads
static int s_nlp_out_pipe[2] = {-1, -1}; // Python writes, Parent reads
static pid_t s_nlp_pid = -1;
static bool s_nlp_ready = false;

// Initial consonants (초성 19자)
static const char *CHOSEONG_TBL[] = {
    "ㄱ", "ㄲ", "ㄴ", "ㄷ", "ㄸ", "ㄹ", "ㅁ", "ㅂ", "ㅃ", "ㅅ",
    "ㅆ", "ㅇ", "ㅈ", "ㅉ", "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"
};

// Medial vowels (중성 21자)
static const char *JUNGSEONG_TBL[] = {
    "ㅏ", "ㅐ", "ㅑ", "ㅒ", "ㅓ", "ㅔ", "ㅕ", "ㅖ", "ㅗ", "ㅘ",
    "ㅙ", "ㅚ", "ㅛ", "ㅜ", "ㅝ", "ㅞ", "ㅟ", "ㅠ", "ㅡ", "ㅢ", "ㅣ"
};

// Final consonants (종성 28자)
static const char *JONGSEONG_TBL[] = {
    "", "ㄱ", "ㄲ", "ㄳ", "ㄴ", "ㄵ", "ㄶ", "ㄷ", "ㄹ", "ㄺ",
    "ㄻ", "ㄼ", "ㄽ", "ㄾ", "ㄿ", "ㅀ", "ㅁ", "ㅂ", "ㅄ", "ㅅ",
    "ㅆ", "ㅇ", "ㅈ", "ㅊ", "ㅋ", "ㅌ", "ㅍ", "ㅎ"
};

uint32_t kor_next_utf8_char(const char **src) {
    if (!src || !*src || !**src) return 0;
    const unsigned char *s = (const unsigned char *)*src;
    uint32_t cp = 0;
    int bytes = 0;

    if (s[0] < 0x80) {
        cp = s[0];
        bytes = 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        bytes = 2;
    } else if ((s[0] & 0xF0) == 0xE0) {
        cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        bytes = 3;
    } else if ((s[0] & 0xF8) == 0xF0) {
        cp = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        bytes = 4;
    } else {
        cp = s[0];
        bytes = 1;
    }

    *src += bytes;
    return cp;
}

HangulChar kor_decompose_char(uint32_t codepoint) {
    HangulChar hc;
    memset(&hc, 0, sizeof(hc));

    if (codepoint >= HANGUL_SYLLABLE_BASE && codepoint <= HANGUL_SYLLABLE_END) {
        uint32_t offset = codepoint - HANGUL_SYLLABLE_BASE;
        hc.jongseong = offset % HANGUL_JONG_COUNT;
        hc.jungseong = (offset / HANGUL_JONG_COUNT) % HANGUL_JUNG_COUNT;
        hc.choseong  = (offset / HANGUL_JONG_COUNT) / HANGUL_JUNG_COUNT;
        hc.is_hangul = true;
    }
    return hc;
}

void kor_strip_josa(const char *word, char *out, size_t out_sz) {
    if (!word || !out || out_sz == 0) return;
    snprintf(out, out_sz, "%s", word);

    size_t len = strlen(out);
    if (len >= 6) {
        const char *tail6 = out + (len - 6);
        if (strcmp(tail6, "에서는") == 0 || strcmp(tail6, "으로는") == 0) {
            out[len - 6] = '\0';
            return;
        }
        if (strcmp(tail6, "에서") == 0 || strcmp(tail6, "으로") == 0 ||
            strcmp(tail6, "에게") == 0 || strcmp(tail6, "부터") == 0 ||
            strcmp(tail6, "까지") == 0) {
            out[len - 6] = '\0';
            return;
        }
    }
    if (len >= 3) {
        const char *tail3 = out + (len - 3);
        if (strcmp(tail3, "은") == 0 || strcmp(tail3, "는") == 0 ||
            strcmp(tail3, "이") == 0 || strcmp(tail3, "가") == 0 ||
            strcmp(tail3, "을") == 0 || strcmp(tail3, "를") == 0 ||
            strcmp(tail3, "에") == 0 || strcmp(tail3, "로") == 0 ||
            strcmp(tail3, "와") == 0 || strcmp(tail3, "과") == 0 ||
            strcmp(tail3, "도") == 0 || strcmp(tail3, "만") == 0) {
            out[len - 3] = '\0';
            return;
        }
    }
}

static void decompose_to_jamo_string(const char *src, char *dst, size_t dst_sz) {
    dst[0] = '\0';
    const char *p = src;
    while (*p) {
        uint32_t cp = kor_next_utf8_char(&p);
        HangulChar hc = kor_decompose_char(cp);
        if (hc.is_hangul) {
            strncat(dst, CHOSEONG_TBL[hc.choseong], dst_sz - strlen(dst) - 1);
            strncat(dst, JUNGSEONG_TBL[hc.jungseong], dst_sz - strlen(dst) - 1);
            if (hc.jongseong > 0) {
                strncat(dst, JONGSEONG_TBL[hc.jongseong], dst_sz - strlen(dst) - 1);
            }
        } else {
            char buf[5] = {0};
            if (cp < 0x80) {
                buf[0] = (char)cp;
            }
            strncat(dst, buf, dst_sz - strlen(dst) - 1);
        }
    }
}

double kor_jamo_similarity(const char *s1, const char *s2) {
    char j1[512] = {0};
    char j2[512] = {0};
    decompose_to_jamo_string(s1, j1, sizeof(j1));
    decompose_to_jamo_string(s2, j2, sizeof(j2));

    size_t len1 = strlen(j1);
    size_t len2 = strlen(j2);
    if (len1 == 0 && len2 == 0) return 1.0;
    if (len1 == 0 || len2 == 0) return 0.0;

    size_t matches = 0;
    size_t min_len = len1 < len2 ? len1 : len2;
    for (size_t i = 0; i < min_len; ++i) {
        if (j1[i] == j2[i]) matches++;
    }
    return (double)(2 * matches) / (double)(len1 + len2);
}

static const char *find_python_executable(void) {
    static const char *candidates[] = {
        "lang/venv/bin/python3",
        "lang/venv/bin/python",
        "venv/bin/python3",
        "venv/bin/python",
        "python3",
        "python",
        NULL
    };
    for (int i = 0; candidates[i] != NULL; ++i) {
        if (candidates[i][0] != '/' && strchr(candidates[i], '/')) {
            if (access(candidates[i], X_OK) == 0) {
                return candidates[i];
            }
        }
    }
    return "python3";
}

void kor_nlp_init(void) {
    if (s_nlp_ready) return;

    if (pipe(s_nlp_in_pipe) < 0 || pipe(s_nlp_out_pipe) < 0) {
        return;
    }

    const char *python_bin = find_python_executable();

    s_nlp_pid = fork();
    if (s_nlp_pid == 0) {
        close(s_nlp_in_pipe[1]);
        close(s_nlp_out_pipe[0]);
        dup2(s_nlp_in_pipe[0], STDIN_FILENO);
        dup2(s_nlp_out_pipe[1], STDOUT_FILENO);
        close(s_nlp_in_pipe[0]);
        close(s_nlp_out_pipe[1]);

        execlp(python_bin, python_bin, "util/kiwi_bridge.py", NULL);
        _exit(1);
    } else if (s_nlp_pid > 0) {
        close(s_nlp_in_pipe[0]);
        close(s_nlp_out_pipe[1]);
        s_nlp_ready = true;
    }
}

void kor_nlp_shutdown(void) {
    if (!s_nlp_ready) return;
    close(s_nlp_in_pipe[1]);
    close(s_nlp_out_pipe[0]);
    if (s_nlp_pid > 0) {
        kill(s_nlp_pid, SIGTERM);
        waitpid(s_nlp_pid, NULL, 0);
    }
    s_nlp_ready = false;
}

bool kor_nlp_analyze(const char *sentence, NlpAnalysisResult *out) {
    if (!sentence || !out) return false;
    memset(out, 0, sizeof(NlpAnalysisResult));
    snprintf(out->raw, sizeof(out->raw), "%s", sentence);

    if (!s_nlp_ready) {
        kor_nlp_init();
    }

    if (!s_nlp_ready) return false;

    char send_buf[1024];
    snprintf(send_buf, sizeof(send_buf), "%s\n", sentence);
    if (write(s_nlp_in_pipe[1], send_buf, strlen(send_buf)) <= 0) {
        return false;
    }

    char recv_buf[4096];
    size_t r_idx = 0;
    while (r_idx < sizeof(recv_buf) - 1) {
        char ch;
        ssize_t n = read(s_nlp_out_pipe[0], &ch, 1);
        if (n <= 0) break;
        if (ch == '\n') break;
        recv_buf[r_idx++] = ch;
    }
    recv_buf[r_idx] = '\0';

    char *p_punch = strstr(recv_buf, "\"punchline\": \"");
    if (p_punch) {
        p_punch += strlen("\"punchline\": \"");
        char punch_buf[256] = {0};
        size_t p_idx = 0;
        while (*p_punch && *p_punch != '"' && p_idx < sizeof(punch_buf) - 1) {
            punch_buf[p_idx++] = *p_punch++;
        }
        snprintf(out->punchline, sizeof(out->punchline), "%s", punch_buf);
    }

    char *p_nouns = strstr(recv_buf, "\"nouns\": [");
    if (p_nouns) {
        p_nouns += strlen("\"nouns\": [");
        while (*p_nouns && *p_nouns != ']' && out->noun_count < 16) {
            if (*p_nouns == '"') {
                p_nouns++;
                char noun_buf[64] = {0};
                size_t n_idx = 0;
                while (*p_nouns && *p_nouns != '"' && n_idx < sizeof(noun_buf) - 1) {
                    noun_buf[n_idx++] = *p_nouns++;
                }
                if (*p_nouns == '"') p_nouns++;
                if (noun_buf[0]) {
                    snprintf(out->nouns[out->noun_count++], sizeof(out->nouns[0]), "%s", noun_buf);
                }
            } else {
                p_nouns++;
            }
        }
    }

    char *p_verbs = strstr(recv_buf, "\"verbs\": [");
    if (p_verbs) {
        p_verbs += strlen("\"verbs\": [");
        while (*p_verbs && *p_verbs != ']' && out->verb_count < 16) {
            if (*p_verbs == '"') {
                p_verbs++;
                char verb_buf[64] = {0};
                size_t v_idx = 0;
                while (*p_verbs && *p_verbs != '"' && v_idx < sizeof(verb_buf) - 1) {
                    verb_buf[v_idx++] = *p_verbs++;
                }
                if (*p_verbs == '"') p_verbs++;
                if (verb_buf[0]) {
                    snprintf(out->verbs[out->verb_count++], sizeof(out->verbs[0]), "%s", verb_buf);
                }
            } else {
                p_verbs++;
            }
        }
    }

    return true;
}
