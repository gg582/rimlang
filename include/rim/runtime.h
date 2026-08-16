/**
 * @file runtime.h
 * @brief Turing-complete OISC virtual machine and environment state for RimLang.
 */

#ifndef RIM_RUNTIME_H
#define RIM_RUNTIME_H

#include "rim/common.h"
#include "rim/ast.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[64];
    RimValue val;
} RimVar;

/**
 * @brief The virtual machine state structure.
 */
typedef struct {
    int64_t memory[RIM_MEM_SIZE];
    RimVar vars[256];
    size_t var_count;
    size_t pc;
    bool halted;
    char last_output[RIM_MAX_STR_LEN];
} RimVM;

void vm_init(RimVM *vm);
void vm_set_var(RimVM *vm, const char *name, RimValue val);
RimValue vm_get_var(RimVM *vm, const char *name);
RimValue vm_eval_node(RimVM *vm, AstNode *node);
void vm_run(RimVM *vm, AstNode *program);

#ifdef __cplusplus
}
#endif

#endif // RIM_RUNTIME_H
