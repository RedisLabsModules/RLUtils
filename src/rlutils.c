#include "rlutils.h"
#include "rlutils_info.h"
#include "rlutils_common.h"
#include "rlutils_config.h"

#define CREATE_COMMAND_NAME(name) STR(RLUTILS_PRFX) "." STR(name)

RLUTILS_PRFX_ModuleData RLUTILS_PRFX_moduleData = {0};

int RLUTILS_PRFX_InitRLUtils(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, int version){

    RedisModule_Log(ctx, "notice", "RLUtils prefix: %s", STR(RLUTILS_PRFX));

    RLUTILS_PRFX_moduleData.version = version;

    if(RLUTILS_PRFX_ConfigInit(argv, argc) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, CREATE_COMMAND_NAME(info), RLUTILS_PRFX_InfoCmd, "readonly", 0, 0, 0) != REDISMODULE_OK) {
        RedisModule_Log(ctx, "warning", "could not register %s command", CREATE_COMMAND_NAME(info));
        return REDISMODULE_ERR;
    }

    if (RedisModule_CreateCommand(ctx, CREATE_COMMAND_NAME(config), RLUTILS_PRFX_ConfigCmd, "readonly", 0, 0, 0) != REDISMODULE_OK) {
        RedisModule_Log(ctx, "warning", "could not register %s command", CREATE_COMMAND_NAME(config));
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}
