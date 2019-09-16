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
#include "rlutils_command_args.h"

typedef union DefaultVal{
    bool bval;
    long long lval;
    double dval;
    char* sval;
    RedisModuleString* rsval;
}DefaultVal;

int RLUTILS_PRFX_AddConfigVal(const char* name, const char* helpMsg, RLUTILS_PRFX_CommandVarPtr ptr, DefaultVal defaultVal, bool configurableAtRuntime);
int RLUTILS_PRFX_ConfigCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
int RLUTILS_PRFX_ConfigInit(RedisModuleCtx* ctx, RedisModuleString **argv, int argc);

#endif /* SRC_RLUTILS_CONFIG_H_ */
