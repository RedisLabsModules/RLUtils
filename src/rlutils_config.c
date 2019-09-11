/*
 * rlutils_config.c
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */
#include <stdbool.h>
#include <assert.h>
#include "rlutils_config.h"
#include "utils/arr_rm_alloc.h"
#include "strings.h"

#define CONFIG_ARR_INIT_SIZE 10

typedef struct RMUTILS_PRFX_ConfigVal{
    const char* name;
    void* ptr;
    RMUTILS_PRFX_ConfigValType type;
    bool configurableAtRuntime;
}RMUTILS_PRFX_ConfigVal;

RMUTILS_PRFX_ConfigVal* configVals = NULL;

static RMUTILS_PRFX_ConfigVal* FindConfigValByName(const char* name){
    for(size_t i = 0 ; i < array_len(configVals) ; ++i){
        RMUTILS_PRFX_ConfigVal* val = configVals + i;
        if(strcmp(name, val->name) == 0){
            return val;
        }
    }
    return NULL;
}

static int ConfigSet(RMUTILS_PRFX_ConfigVal* configVal, RedisModuleString* val){
    switch(configVal->type){
    case RMUTILS_PRFX_ConfigValType_BOOL:
        *((bool*)configVal->ptr) = true;
        return REDISMODULE_OK;
    case RMUTILS_PRFX_ConfigValType_LONG:
        return RedisModule_StringToLongLong(val, configVal->ptr);
    case RMUTILS_PRFX_ConfigValType_DOUBLE:
        return RedisModule_StringToDouble(val, configVal->ptr);
    case RMUTILS_PRFX_ConfigValType_CSTR:
        *((char**)configVal->ptr) = RMUTILS_PRFX_strdup(RedisModule_StringPtrLen(val, NULL));
        return REDISMODULE_OK;
    case RMUTILS_PRFX_ConfigValType_REDISSTR:
        RedisModule_RetainString(val);
        *((RedisModuleString**)configVal->ptr) = val;
        return REDISMODULE_OK;
    case RMUTILS_PRFX_ConfigValType_CALLBACKS:
        // todo : implement
        return REDISMODULE_OK;
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

static int ConfigGet(RedisModuleCtx *ctx, RMUTILS_PRFX_ConfigVal* configVal){
    switch(configVal->type){
    case RMUTILS_PRFX_ConfigValType_LONG:
        return RedisModule_ReplyWithCStr(ctx, (*((bool*)configVal->ptr) ? "enabled" : "disabled"));
    case RMUTILS_PRFX_ConfigValType_LONG:
        return RedisModule_ReplyWithLongLong(ctx, *((long long*)configVal->ptr));
    case RMUTILS_PRFX_ConfigValType_DOUBLE:
        return RedisModule_ReplyWithDouble(ctx, *((double*)configVal->ptr));
    case RMUTILS_PRFX_ConfigValType_CSTR:
        return RedisModule_ReplyWithCStr(ctx, *((char**)configVal->ptr));
    case RMUTILS_PRFX_ConfigValType_REDISSTR:
        return RedisModule_ReplyWithString(ctx, *((RedisModuleString**)configVal->ptr));
    case RMUTILS_PRFX_ConfigValType_CALLBACKS:
        return RedisModule_ReplyWithCStr(ctx, "callback function");
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

int RMUTILS_PRFX_AddConfigVal(const char* name, void* ptr, RMUTILS_PRFX_ConfigValType type, bool configurableAtRuntime){
    if(!configVals){
        configVals = array_new(RMUTILS_PRFX_ConfigVal, CONFIG_ARR_INIT_SIZE);
    }
    RMUTILS_PRFX_ConfigVal val = {
            .name = name,
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

int RMUTILS_PRFX_ConfigCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){
    if(argc < 3){
        return RedisModule_WrongArity(ctx);
    }

    const char* cmd = RedisModule_StringPtrLen(argv[1], NULL);
    SubcommandType subcommand;

    if(strcasecmp(cmd, "get") == 0){
        subcommand = GET;
    }else if(strcasecmp(cmd, "set") == 0){
        subcommand = GET;
    }else{
        return RedisModule_ReplyWithError(ctx, "subcommand not available");
    }

    RMUTILS_PRFX_ConfigVal* configVal = FindConfigValByName(argv[2]);
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

        if(configVal->type != RMUTILS_PRFX_ConfigValType_BOOL){
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
        return ConfigSet(ctx, configVal, val);
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

int RMUTILS_PRFX_ConfigInit(RedisModuleString **argv, int argc){
    if(argc % 2 != 0 || argc == 0){
        return REDISMODULE_ERR;
    }
    for(size_t i = 0 ; i < argc / 2 ; i += 2){
        RMUTILS_PRFX_ConfigVal* configVal = FindConfigValByName(argv[i]);
        if(ConfigSet(configVal, argv[i + 1]) != REDISMODULE_OK){
            return REDISMODULE_ERR;
        }
    }
    return REDISMODULE_OK;
}



