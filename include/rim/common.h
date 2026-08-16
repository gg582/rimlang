/**
 * @file common.h
 * @brief Global type definitions, constants, and utility helpers for RimLang.
 */

#ifndef RIM_COMMON_H
#define RIM_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define RIM_MEM_SIZE 65536
#define RIM_MAX_STR_LEN 1024
#define RIM_MAX_WORKERS 8

/**
 * @brief Primitive runtime value types.
 */
typedef enum {
    VAL_INT,
    VAL_STR,
    VAL_BOOL,
    VAL_NIL
} RimValType;

/**
 * @brief Unified value container supporting integers, strings, and booleans.
 */
typedef struct {
    RimValType type;
    union {
        int64_t i;
        char s[RIM_MAX_STR_LEN];
        bool b;
    };
} RimValue;

/**
 * @brief Constructs an integer RimValue.
 */
static inline RimValue rim_int(int64_t val) {
    RimValue v;
    v.type = VAL_INT;
    v.i = val;
    return v;
}

/**
 * @brief Constructs a string RimValue.
 */
static inline RimValue rim_str(const char *val) {
    RimValue v;
    v.type = VAL_STR;
    strncpy(v.s, val ? val : "", sizeof(v.s) - 1);
    v.s[sizeof(v.s) - 1] = '\0';
    return v;
}

/**
 * @brief Constructs a boolean RimValue.
 */
static inline RimValue rim_bool(bool val) {
    RimValue v;
    v.type = VAL_BOOL;
    v.b = val;
    return v;
}

/**
 * @brief Constructs a Nil RimValue.
 */
static inline RimValue rim_nil(void) {
    RimValue v;
    v.type = VAL_NIL;
    v.i = 0;
    return v;
}

#endif // RIM_COMMON_H
