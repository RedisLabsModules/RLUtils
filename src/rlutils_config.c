#include <stdbool.h>
#include <assert.h>
#include "rlutils_config.h"
#include "rlutils_common.h"
#include "utils/arr_rm_alloc.h"
#include "strings.h"

#define CONFIG_ARR_INIT_SIZE 10

typedef struct RLUTILS_PRFX_ConfigVal{
    const char* name;
    const char* helpMsg;
    void* ptr;
    RLUTILS_PRFX_ConfigValType type;
    bool configurableAtRuntime;
}RLUTILS_PRFX_ConfigVal;

RLUTILS_PRFX_ConfigVal* configVals = NULL;

static RLUTILS_PRFX_ConfigVal* FindConfigValByName(const char* name){
    for(size_t i = 0 ; i < array_len(configVals) ; ++i){
        RLUTILS_PRFX_ConfigVal* val = configVals + i;
        if(strcmp(name, val->name) == 0){
            return val;
        }
    }
    return NULL;
}

static int ConfigSet(RLUTILS_PRFX_ConfigVal* configVal, RedisModuleString* val){
    switch(configVal->type){
    case RLUTILS_PRFX_ConfigValType_BOOL:
        *((bool*)configVal->ptr) = true;
        return REDISMODULE_OK;
    case RLUTILS_PRFX_ConfigValType_LONG:
        return RedisModule_StringToLongLong(val, configVal->ptr);
    case RLUTILS_PRFX_ConfigValType_DOUBLE:
        return RedisModule_StringToDouble(val, configVal->ptr);
    case RLUTILS_PRFX_ConfigValType_CSTR:
        *((char**)configVal->ptr) = RLUTILS_PRFX_strdup(RedisModule_StringPtrLen(val, NULL));
        return REDISMODULE_OK;
    case RLUTILS_PRFX_ConfigValType_REDISSTR:
        RedisModule_RetainString(NULL, val);
        *((RedisModuleString**)configVal->ptr) = val;
        return REDISMODULE_OK;
    case RLUTILS_PRFX_ConfigValType_CALLBACKS:
        // todo : implement
        return REDISMODULE_OK;
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

static int ConfigGet(RedisModuleCtx *ctx, RLUTILS_PRFX_ConfigVal* configVal){
    switch(configVal->type){
    case RLUTILS_PRFX_ConfigValType_BOOL:
        return RedisModule_ReplyWithCStr(ctx, (*((bool*)configVal->ptr) ? "enabled" : "disabled"));
    case RLUTILS_PRFX_ConfigValType_LONG:
        return RedisModule_ReplyWithLongLong(ctx, *((long long*)configVal->ptr));
    case RLUTILS_PRFX_ConfigValType_DOUBLE:
        return RedisModule_ReplyWithDouble(ctx, *((double*)configVal->ptr));
    case RLUTILS_PRFX_ConfigValType_CSTR:
        return RedisModule_ReplyWithCStr(ctx, *((char**)configVal->ptr));
    case RLUTILS_PRFX_ConfigValType_REDISSTR:
        return RedisModule_ReplyWithString(ctx, *((RedisModuleString**)configVal->ptr));
    case RLUTILS_PRFX_ConfigValType_CALLBACKS:
        return RedisModule_ReplyWithCStr(ctx, "callback function");
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

int RLUTILS_PRFX_AddConfigVal(const char* name, const char* helpMsg, void* ptr, RLUTILS_PRFX_ConfigValType type, bool configurableAtRuntime){
    if(!configVals){
        configVals = array_new(RLUTILS_PRFX_ConfigVal, CONFIG_ARR_INIT_SIZE);
    }
    RLUTILS_PRFX_ConfigVal val = {
            .name = name,
            .helpMsg = helpMsg,
            .ptr = ptr,
            .type = type,
            .configurableAtRuntime = configurableAtRuntime,
    };
    configVals = array_append(configVals, val);
    return REDISMODULE_OK;
}

typedef enum SubcommandType{
    GET,SET
}SubcommandType;

static int ReplyHelpCmd(RedisModuleCtx *ctx){
    RedisModule_ReplyWithArray(ctx, array_len(configVals));
    for(size_t i = 0 ; i < array_len(configVals) ; ++i){
        RLUTILS_PRFX_ConfigVal* val = configVals + i;
        RedisModule_ReplyWithArray(ctx, 3);// name, help, type, configurable at runtime
        RedisModule_ReplyWithCStr(ctx, val->name);
        RedisModule_ReplyWithCStr(ctx, val->helpMsg);
        if(val->configurableAtRuntime){
            RedisModule_ReplyWithCStr(ctx, "configurable at runtime : yes");
        }else{
            RedisModule_ReplyWithCStr(ctx, "configurable at runtime : no");
        }
    }
    return REDISMODULE_OK;
}

int RLUTILS_PRFX_ConfigCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){
    if(argc < 2){
        return RedisModule_WrongArity(ctx);
    }

    const char* cmd = RedisModule_StringPtrLen(argv[1], NULL);
    SubcommandType subcommand;

    if(strcasecmp(cmd, "get") == 0){
        subcommand = GET;
    }else if(strcasecmp(cmd, "set") == 0){
        subcommand = SET;
    }else if(strcasecmp(cmd, "help") == 0){
        if(argc != 2){
            return RedisModule_WrongArity(ctx);
        }
        return ReplyHelpCmd(ctx);
    }else{
        return RedisModule_ReplyWithError(ctx, "subcommand not available");
    }

    const char* key = RedisModule_StringPtrLen(argv[2], NULL);
    RLUTILS_PRFX_ConfigVal* configVal = FindConfigValByName(key);
    if(!configVal){
        RedisModule_ReplyWithError(ctx, "ERR - Unknow config values");
    }

    RedisModuleString* val = NULL;
    switch(subcommand){
    case GET:
        if(argc != 3){
            return RedisModule_WrongArity(ctx);
        }
        return ConfigGet(ctx, configVal);
    case SET:

        if(configVal->type != RLUTILS_PRFX_ConfigValType_BOOL){
            if(argc != 4){
                return RedisModule_WrongArity(ctx);
            }
            val = argv[3];
        }else{
            if(argc != 3){
                return RedisModule_WrongArity(ctx);
            }
        }
        if(!configVal->configurableAtRuntime){
            RedisModule_ReplyWithError(ctx, "ERR - configuration is not configurable at runtime");
        }
        if(ConfigSet(configVal, val) != REDISMODULE_OK){
            return RedisModule_ReplyWithError(ctx, "ERR - could not set configuration");
        }
        return RedisModule_ReplyWithSimpleString(ctx, "OK");
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

int RLUTILS_PRFX_ConfigInit(RedisModuleString **argv, int argc){
    size_t i = 0;
    while(i < argc){
        const char* key = RedisModule_StringPtrLen(argv[i], NULL);
        RLUTILS_PRFX_ConfigVal* configVal = FindConfigValByName(key);
        RedisModuleString* val = NULL;
        if(configVal->type == RLUTILS_PRFX_ConfigValType_BOOL){
            ++i;
            if(i < argc){
                return REDISMODULE_ERR;
            }
            val = argv[i];

        }
        if(ConfigSet(configVal, val) != REDISMODULE_OK){
            return REDISMODULE_ERR;
        }
        ++i;
    }
    return REDISMODULE_OK;
}



