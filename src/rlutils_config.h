/*
 * rlutils_config.h
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#ifndef SRC_RLUTILS_CONFIG_H_
#define SRC_RLUTILS_CONFIG_H_

#include "redismodule.h"
#include <stdbool.h>

typedef enum RMUTILS_PRFX_ConfigValType{
    RMUTILS_PRFX_ConfigValType_BOOL,
    RMUTILS_PRFX_ConfigValType_LONG,
    RMUTILS_PRFX_ConfigValType_DOUBLE,
    RMUTILS_PRFX_ConfigValType_CSTR,
    RMUTILS_PRFX_ConfigValType_REDISSTR,
    RMUTILS_PRFX_ConfigValType_CALLBACKS,
}RMUTILS_PRFX_ConfigValType;

typedef struct RMUTILS_PRFX_ConfigCallbacks{
    int (*updateConfig)();
}RMUTILS_PRFX_ConfigCallbacks;

int RMUTILS_PRFX_AddConfigVal(const char* name, const char* helpMsg, void* ptr, RMUTILS_PRFX_ConfigValType type, bool configurableAtRuntime);
int RMUTILS_PRFX_ConfigCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int RMUTILS_PRFX_ConfigInit(RedisModuleString **argv, int argc);

#endif /* SRC_RLUTILS_CONFIG_H_ */
