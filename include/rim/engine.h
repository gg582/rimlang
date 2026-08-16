/**
 * @file engine.h
 * @brief POSIX multi-process worker pool and parallel evaluation engine.
 */

#ifndef RIM_ENGINE_H
#define RIM_ENGINE_H

#include "rim/common.h"
#include "rim/ast.h"
#include "rim/runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int num_workers;
    int task_pipes[RIM_MAX_WORKERS][2]; // Parent writes, Worker reads
    int res_pipes[RIM_MAX_WORKERS][2];  // Worker writes, Parent reads
    pid_t worker_pids[RIM_MAX_WORKERS];
} RimEngine;

void engine_init(RimEngine *engine, int num_workers);
void engine_eval_program(RimEngine *engine, RimVM *vm, AstNode *program);
void engine_shutdown(RimEngine *engine);

#ifdef __cplusplus
}
#endif

#endif // RIM_ENGINE_H
