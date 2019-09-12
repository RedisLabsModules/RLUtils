
#include "../redismodule.h"
#include "../rlutils.h"
#include "../rlutils_config.h"
#include "../rlutils_common.h"

#define VERSION 1

typedef struct Config{
    long long LONG;
    bool BOOL;
    char* CSTR;
    RedisModuleString* REDISSTR;
}Config;

Config config = {0};

#define CONFIG_VAL(type) if (RLUTILS_PRFX_AddConfigVal(STR(type), \
                                                       STR(type)" help msg", \
                                                       &config.type, \
                                                       RLUTILS_PRFX_ConfigValType_ ## type, \
                                                       true) != REDISMODULE_OK){ \
                             return REDISMODULE_ERR; \
                          }

static int DefineConfigVars(){
    CONFIG_VAL(LONG);
    CONFIG_VAL(BOOL);
    CONFIG_VAL(CSTR);
    CONFIG_VAL(REDISSTR);

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){

    if (RedisModule_Init(ctx, "example", VERSION, VERSION) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    DefineConfigVars();

    RLUTILS_PRFX_InitRLUtils(ctx, argv, argc, VERSION);

    return REDISMODULE_OK;
}



