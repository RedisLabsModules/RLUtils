
#include "../redismodule.h"
#include "../rlutils.h"
#include "../rlutils_config.h"
#include "../rlutils_command_args.h"
#include "../rlutils_common.h"
#include "../utils/arr_rm_alloc.h"

#include <string.h>
#include <limits.h>

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

DefaultVal DUMMY;

typedef struct Config{
    long long LONG;
    long long LONG_NOT_CONFIGURABLE;
    double DOUBLE;
    bool BOOL;
    char* CSTR;
    RedisModuleString* REDISSTR;
}Config;

Config config = {0};

static int ConfigSet(void* ctx, RLUTILS_PRFX_ArgIterator* iter){
    return REDISMODULE_OK;
}

static int ConfigGet(RedisModuleCtx* rctx, void* ctx){
    RedisModule_ReplyWithCStr(rctx, "OK");
    return REDISMODULE_OK;
}

static int DefineConfigVars(){
    if (RLUTILS_PRFX_AddConfigVal("LONG", "LONG help msg",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .lval = &config.LONG,
                                        .lvalMin = LONG_MIN,
                                        .lvalMax = LONG_MAX,
                                        .type = LONG,
                                  },
                                  LONGDV, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("DOUBLE", "DOUBLE help msg",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .dval = &config.DOUBLE,
                                        .dvalMin = -10.0,
                                        .dvalMax = 10.0,
                                        .type = DOUBLE,
                                  },
                                  DOUBLEDV, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("BOOL", "BOOL help msg",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .bval = &config.BOOL,
                                        .type = BOOL,
                                  },
                                  BOOLDV, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("CSTR", "CSTR help msg",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .sval = &config.CSTR,
                                        .type = STR,
                                  },
                                  CSTRDV, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("REDISSTR", "REDISSTR help msg",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .rsval = &config.REDISSTR,
                                        .type = REDISSTR,
                                  },
                                  REDISSTRDV, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("LONG_NOT_CONFIGURABLE", "not configurable long value",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .lval = &config.LONG_NOT_CONFIGURABLE,
                                        .lvalMin = LONG_MIN,
                                        .lvalMax = LONG_MAX,
                                        .type = LONG,
                                  },
                                  LONGDV, false) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    if (RLUTILS_PRFX_AddConfigVal("CALLBACK", "demonstrate how to use callback with config values",
                                  (RLUTILS_PRFX_CommandVarPtr){
                                        .setCallback = ConfigSet,
                                        .getCallback = ConfigGet,
                                        .type = CALLBACK,
                                  },
                                  DUMMY, true) != REDISMODULE_OK){
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

static int TestCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){
    long long longValue;
    double doubleValue;
    char* strValue;
    RedisModuleString* rstrValue;

    long long namedLongValue;
    double namedDoubleValue;
    bool namedBoolValue;
    char* namedStrValue = "init_val";
    RedisModuleString* namedRstrValue = argv[0];

    char** arrValues = array_new(char*, 10);


    RLUTILS_PRFX_CommandArgsDef args[] = {
            RLUTILS_PRFX_CommandArgsDefLong(longValue),
            RLUTILS_PRFX_CommandArgsDefDouble(doubleValue),
            RLUTILS_PRFX_CommandArgsDefStr(strValue),
            RLUTILS_PRFX_CommandArgsDefRedisStr(rstrValue),
            RLUTILS_PRFX_CommandArgsDefDone(),
    };

    RLUTILS_PRFX_CommandNamedArgsDef namedArgs[] = {
            RLUTILS_PRFX_CommandNamedArgsDefLong("namedLongValue", namedLongValue),
            RLUTILS_PRFX_CommandNamedArgsDefDouble("namedDoubleValue", namedDoubleValue),
            RLUTILS_PRFX_CommandNamedArgsDefStr("namedStrValue", namedStrValue),
            RLUTILS_PRFX_CommandNamedArgsDefRedisStr("namedRstrValue", namedRstrValue),
            RLUTILS_PRFX_CommandNamedArgsDefBool("namedBoolValue", namedBoolValue),
            RLUTILS_PRFX_CommandNamedArgsDefDone(),
    };

    RLUTILS_PRFX_CommandArrVarPtr arrArgs = {
            .sval = &arrValues,
            .type = STR,
            .flags = 0,
    };

    if(RLUTILS_PRFX_CommandArgsParse(ctx, argv + 1, argc - 1, args, namedArgs, &arrArgs, RaiseErrorOnExtraArgsLeft) != REDISMODULE_OK){
        return REDISMODULE_OK;
    }

    RedisModule_ReplyWithArray(ctx, 10);
    RedisModule_ReplyWithLongLong(ctx, longValue);
    RedisModule_ReplyWithDouble(ctx, doubleValue);
    RedisModule_ReplyWithCStr(ctx, strValue);
    RedisModule_ReplyWithString(ctx, rstrValue);
    RedisModule_ReplyWithLongLong(ctx, namedLongValue);
    RedisModule_ReplyWithDouble(ctx, namedDoubleValue);
    RedisModule_ReplyWithCStr(ctx, namedStrValue);
    RedisModule_ReplyWithString(ctx, namedRstrValue);
    RedisModule_ReplyWithCStr(ctx, (namedBoolValue? "enabled" : "disabled"));

    RedisModule_ReplyWithArray(ctx, array_len(arrValues));
    for(size_t i = 0 ; i < array_len(arrValues) ; ++i){
        RedisModule_ReplyWithCStr(ctx, arrValues[i]);
    }

    array_free(arrValues);
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

    if (RedisModule_CreateCommand(ctx, "test", TestCmd, "readonly", 0, 0, 0) != REDISMODULE_OK) {

    }

    return REDISMODULE_OK;
}



