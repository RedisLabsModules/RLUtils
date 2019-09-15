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

typedef union DefaultVal{
    bool bval;
    long long lval;
    double dval;
    char* sval;
    RedisModuleString* rsval;
}DefaultVal;

typedef enum RLUTILS_PRFX_ConfigValType{
    RLUTILS_PRFX_ConfigValType_BOOL,
    RLUTILS_PRFX_ConfigValType_LONG,
    RLUTILS_PRFX_ConfigValType_DOUBLE,
    RLUTILS_PRFX_ConfigValType_CSTR,
    RLUTILS_PRFX_ConfigValType_REDISSTR,
    RLUTILS_PRFX_ConfigValType_CALLBACKS,
}RLUTILS_PRFX_ConfigValType;

typedef struct RLUTILS_PRFX_ConfigCallbacks{
    int (*updateConfig)();
}RLUTILS_PRFX_ConfigCallbacks;

int RLUTILS_PRFX_AddConfigVal(const char* name, const char* helpMsg, void* ptr, DefaultVal defaultVal, RLUTILS_PRFX_ConfigValType type, bool configurableAtRuntime);
int RLUTILS_PRFX_ConfigCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int RLUTILS_PRFX_ConfigInit(RedisModuleCtx* ctx, RedisModuleString **argv, int argc);

#endif /* SRC_RLUTILS_CONFIG_H_ */
