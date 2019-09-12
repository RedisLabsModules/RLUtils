
#include "../redismodule.h"
#include "../rlutils.h"
#include "../rlutils_config.h"

#define VERSION 1

typedef struct Config{
    long long longValConfigurableAtRuntime;
}Config;

Config config = {0};

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){

    if (RedisModule_Init(ctx, "example", VERSION, VERSION) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    RLUTILS_PRFX_AddConfigVal("longValConfigurableAtRuntime",
                              "some var can be set on runtime",
                              &config.longValConfigurableAtRuntime,
                              RLUTILS_PRFX_ConfigValType_LONG, true);

    RLUTILS_PRFX_InitRLUtils(ctx, argv, argc, VERSION);

    return REDISMODULE_OK;
}



