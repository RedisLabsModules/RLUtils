#include <stdbool.h>
#include <assert.h>
#include "rlutils_config.h"
#include "rlutils_common.h"
#include "utils/arr_rm_alloc.h"
#include "strings.h"

#define CONFIG_ARR_INIT_SIZE 10

RLUTILS_PRFX_CommandNamedArgsDef nullDef = {
        .name = NULL,
};

typedef struct RLUTILS_PRFX_ConfigValExtraInfo{
    const char* helpMsg;
    DefaultVal defaultVal;
    bool configurableAtRuntime;
}RLUTILS_PRFX_ConfigValExtraInfo;

typedef struct RLUTILS_PRFX_ConfigVals{
    RLUTILS_PRFX_CommandNamedArgsDef* defs;
    RLUTILS_PRFX_ConfigValExtraInfo* extraInfo;
}RLUTILS_PRFX_ConfigVals;

RLUTILS_PRFX_ConfigVals configVals = {0};

int RLUTILS_PRFX_AddConfigVal(const char* name, const char* helpMsg,
                              RLUTILS_PRFX_CommandVarPtr ptr,
                              DefaultVal defaultVal, bool configurableAtRuntime){
    if(!configVals.defs){
        configVals.defs = array_new(RLUTILS_PRFX_CommandNamedArgsDef, CONFIG_ARR_INIT_SIZE);
        configVals.extraInfo = array_new(RLUTILS_PRFX_ConfigValExtraInfo, CONFIG_ARR_INIT_SIZE);

        // adding nullDef, to the end of the array. nullDef indicate the end of the defs array.
        configVals.defs = array_append(configVals.defs, nullDef);
    }
    if(RLUTILS_PRFX_CommandArgsFindDef(name, configVals.defs, NULL)){
        return REDISMODULE_ERR;
    }
    RLUTILS_PRFX_CommandNamedArgsDef def = {
            .name = name,
            .arg.val = ptr,
            .arg.flags = CommandArgCopy | FreeOldValue,
    };
    RLUTILS_PRFX_ConfigValExtraInfo extraInfo = {
            .helpMsg = helpMsg,
            .defaultVal = defaultVal,
            .configurableAtRuntime = configurableAtRuntime,
    };
    configVals.defs[array_len(configVals.extraInfo)] = def;
    configVals.extraInfo = array_append(configVals.extraInfo, extraInfo);

    // always add the nullDef at the end
    configVals.defs = array_append(configVals.defs, nullDef);
    return REDISMODULE_OK;
}

typedef enum SubcommandType{
    GET,SET
}SubcommandType;

static int ReplyHelpCmd(RedisModuleCtx *ctx){
    RedisModule_ReplyWithArray(ctx, array_len(configVals.extraInfo));
    for(size_t i = 0 ; i < array_len(configVals.extraInfo) ; ++i){
        RLUTILS_PRFX_CommandNamedArgsDef* def = configVals.defs + i;
        RLUTILS_PRFX_ConfigValExtraInfo* extraInfo = configVals.extraInfo + i;
        RedisModule_ReplyWithArray(ctx, 3);// name, help, type, configurable at runtime
        RedisModule_ReplyWithCStr(ctx, def->name);
        RedisModule_ReplyWithCStr(ctx, extraInfo->helpMsg);
        if(extraInfo->configurableAtRuntime){
            RedisModule_ReplyWithCStr(ctx, "configurable at runtime : yes");
        }else{
            RedisModule_ReplyWithCStr(ctx, "configurable at runtime : no");
        }
    }
    return REDISMODULE_OK;
}

static int ConfigGet(RedisModuleCtx *ctx, RLUTILS_PRFX_CommandNamedArgsDef* def, RLUTILS_PRFX_ConfigValExtraInfo* extraInfo){
    switch(def->arg.val.type){
    case BOOL:
        return RedisModule_ReplyWithCStr(ctx, *def->arg.val.bval ? "enabled" : "disabled");
    case LONG:
        return RedisModule_ReplyWithLongLong(ctx, *def->arg.val.lval);
    case DOUBLE:
        return RedisModule_ReplyWithDouble(ctx, *def->arg.val.dval);
    case STR:
        return RedisModule_ReplyWithCStr(ctx, *def->arg.val.sval);
    case REDISSTR:
        return RedisModule_ReplyWithString(ctx, *def->arg.val.rsval);
    case CALLBACK:
        if(def->arg.val.getCallback){
            return def->arg.val.getCallback(ctx, def->arg.val.ctx);
        }else{
            return RedisModule_ReplyWithCStr(ctx, "No callback given for this config value");
        }
    default:
        assert(false);
    }
    assert(false);
    return REDISMODULE_ERR;
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

    if(argc <= 2){
        return RedisModule_WrongArity(ctx);
    }

    const char* key = RedisModule_StringPtrLen(argv[2], NULL);
    size_t index;
    RLUTILS_PRFX_CommandNamedArgsDef* def = RLUTILS_PRFX_CommandArgsFindDef(key, configVals.defs, &index);
    if(!def){
        return RedisModule_ReplyWithError(ctx, "ERR - Unknow config values");
    }
    RLUTILS_PRFX_ConfigValExtraInfo* extraInfo = configVals.extraInfo + index;

    RLUTILS_PRFX_ArgIterator iter;
    RLUTILS_PRFX_MemoryGuard mg;
    char* err;
    int retVal = REDISMODULE_OK;
    switch(subcommand){
    case GET:
        if(argc != 3){
            return RedisModule_WrongArity(ctx);
        }
        return ConfigGet(ctx, def, extraInfo);
    case SET:
        if(def->arg.val.type != BOOL){
            if(argc != 4){
                return RedisModule_WrongArity(ctx);
            }
            RLUTILS_PRFX_ArgIteratorInit(&iter, argv + 3, 1);
        }else{
            if(argc != 3){
                return RedisModule_WrongArity(ctx);
            }
            RLUTILS_PRFX_ArgIteratorInit(&iter, NULL, 0);
        }

        if(!extraInfo->configurableAtRuntime){
            return RedisModule_ReplyWithError(ctx, "ERR - configuration is not configurable at runtime");
        }

        RLUTILS_PRFX_MemoryGuardrInit(&mg);

        if(RLUTILS_PRFX_ParseArg(&def->arg, &iter, &mg, &err) != REDISMODULE_OK){
            RedisModule_ReplyWithError(ctx, err);
            RLUTILS_PRFX_MemoryGuardFreeUnits(&mg);
            retVal = REDISMODULE_ERR;
        }
        RLUTILS_PRFX_MemoryGuardFreeStructure(&mg);

        if(retVal == REDISMODULE_OK){
            RedisModule_ReplyWithSimpleString(ctx, "OK");
        }
        return retVal;
    default:
        assert(0);
    }
    assert(0);
    return REDISMODULE_ERR;
}

static void SetDefaultVals(){
    for(size_t i = 0 ; i < array_len(configVals.extraInfo) ; ++i){
        RLUTILS_PRFX_CommandNamedArgsDef* def = configVals.defs + i;
        RLUTILS_PRFX_ConfigValExtraInfo* extraInfo = configVals.extraInfo + i;
        switch(def->arg.val.type){
        case BOOL:
            *(def->arg.val.bval) = extraInfo->defaultVal.bval;
            break;
        case LONG:
            *(def->arg.val.lval) = extraInfo->defaultVal.lval;
            break;
        case DOUBLE:
            *(def->arg.val.dval) = extraInfo->defaultVal.dval;
            break;
        case STR:
            *(def->arg.val.sval) = RLUTILS_PRFX_strdup(extraInfo->defaultVal.sval);
            break;
        case REDISSTR:
            RedisModule_RetainString(NULL, extraInfo->defaultVal.rsval);
            *(def->arg.val.rsval) = extraInfo->defaultVal.rsval;
            break;
        case CALLBACK:
            break; // we do not set default values on callback
        default:
            assert(0);
        }
    }
}

int RLUTILS_PRFX_ConfigInit(RedisModuleCtx* ctx, RedisModuleString **argv, int argc){
    // we first set default values to all variables
    SetDefaultVals();

    char* err = NULL;
    if(RLUTILS_PRFX_CommandArgsParseInternal(argv, argc, NULL, configVals.defs, NULL, RaiseErrorOnUnknownArg, &err) != REDISMODULE_OK){
        RedisModule_Log(ctx, "warning", "Failed loading configuration, %s", err);
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}



