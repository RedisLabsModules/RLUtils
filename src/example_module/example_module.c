
#include "../redismodule.h"
#include "../rlutils.h"
#include "../rlutils_config.h"
#include "../rlutils_common.h"

#include <string.h>

#define VERSION 1

DefaultVal LONGDV = {
        .lval = 1,
};

DefaultVal LONG_NOT_CONFIGURABLEDV = {
        .lval = 1,
};

DefaultVal DOUBLEDV = {
        .dval = 1.0,
};

DefaultVal BOOLDV = {
        .bval = false,
};

DefaultVal CSTRDV = {
        .sval = "defaultVal",
};

DefaultVal REDISSTRDV = {
        .rsval = NULL,
};

typedef struct Config{
    long long LONG;
    long long LONG_NOT_CONFIGURABLE;
    double DOUBLE;
    bool BOOL;
    char* CSTR;
    RedisModuleString* REDISSTR;
}Config;

Config config = {0};

#define CONFIG_VAL(type) if (RLUTILS_PRFX_AddConfigVal(STR(type), \
                                                       STR(type)" help msg", \
                                                       &config.type, \
                                                       type ## DV, \
                                                       RLUTILS_PRFX_ConfigValType_ ## type, \
                                                       true) != REDISMODULE_OK){ \
                             return REDISMODULE_ERR; \
                          }

static int DefineConfigVars(){
    CONFIG_VAL(LONG);
    CONFIG_VAL(DOUBLE);
    CONFIG_VAL(BOOL);
    CONFIG_VAL(CSTR);
    CONFIG_VAL(REDISSTR);

    // create a not configurable at runtime value
    if (RLUTILS_PRFX_AddConfigVal("LONG_NOT_CONFIGURABLE", "not configurable long value",
                                  &config.LONG_NOT_CONFIGURABLE, LONGDV,
                                  RLUTILS_PRFX_ConfigValType_LONG, false) != REDISMODULE_OK){ \
        return REDISMODULE_ERR; \
    }

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){

    if (RedisModule_Init(ctx, "example", VERSION, VERSION) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    REDISSTRDV.rsval = RedisModule_CreateString(NULL, "defaultRedisVal", strlen("defaultRedisVal"));

    DefineConfigVars();

    if(RLUTILS_PRFX_InitRLUtils(ctx, argv, argc, VERSION) != REDISMODULE_OK){
        RedisModule_Log(ctx, "warning", "failed to initialize RLUtils");
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}



