/*
 * rlutils.c
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#include "rlutils.h"
#include "rlutils_info.h"
#include "rlutils_common.h"
#include "rlutils_config.h"

#define CREATE_COMMAND_NAME(name) STR(RMUTILS_PRFX) "." STR(name)

int RMUTILS_PRFX_InitRLUtils(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, int version){

    RMUTILS_PRFX_moduleData.version = version;

    if(RMUTILS_PRFX_ConfigInit(argv, argc) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, CREATE_COMMAND_NAME(info), RMUTILS_PRFX_InfoCmd, "readonly", 0, 0, 0) != REDISMODULE_OK) {
        RedisModule_Log(ctx, "warning", "could not register %s command", CREATE_COMMAND_NAME(info));
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, CREATE_COMMAND_NAME(config), RMUTILS_PRFX_ConfigCmd, "readonly", 0, 0, 0) != REDISMODULE_OK) {
        RedisModule_Log(ctx, "warning", "could not register %s command", CREATE_COMMAND_NAME(config));
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
