#include "rlutils_command_args.h"
#include "redismodule.h"

#include "memory/rlutils_memory.h"
#include "utils/arr_rm_alloc.h"

#include <string.h>
#include <assert.h>

#define ArgsMask uint64_t

typedef struct RLUTILS_PRFX_ArgIterator{
    RedisModuleString** argv;
    size_t argc;
    size_t location;
}RLUTILS_PRFX_ArgIterator;

static void RLUTILS_PRFX_ArgIteratorInit(RLUTILS_PRFX_ArgIterator* iter, RedisModuleString** argv, size_t argc){
    *iter = (RLUTILS_PRFX_ArgIterator){
            .argv = argv,
            .argc = argc,
            .location = 0,
    };
}

RedisModuleString* RLUTILS_PRFX_ArgIteratorNext(RLUTILS_PRFX_ArgIterator* iter){
    if(iter->location >= iter->argc){
        return NULL;
    }
    return iter->argv[iter->location++];
}

int RLUTILS_PRFX_ArgIteratorNextLong(RLUTILS_PRFX_ArgIterator* iter, long long* val){
    RedisModuleString* temp = RLUTILS_PRFX_ArgIteratorNext(iter);
    if(!temp){
        return REDISMODULE_ERR;
    }
    return RedisModule_StringToLongLong(temp, val);
}

int RLUTILS_PRFX_ArgIteratorNextDouble(RLUTILS_PRFX_ArgIterator* iter, double* val){
    RedisModuleString* temp = RLUTILS_PRFX_ArgIteratorNext(iter);
    if(!temp){
        return REDISMODULE_ERR;
    }
    return RedisModule_StringToDouble(temp, val);
}

const char* RLUTILS_PRFX_ArgIteratorNextCStr(RLUTILS_PRFX_ArgIterator* iter){
    RedisModuleString* temp = RLUTILS_PRFX_ArgIteratorNext(iter);
    if(!temp){
        return NULL;
    }
    return RedisModule_StringPtrLen(temp, NULL);
}

static RLUTILS_PRFX_CommandArgsDef* FindArgDef(const char* name, RLUTILS_PRFX_CommandArgsDef* defs, size_t* index){
    if(index) *index = 0;
    for(RLUTILS_PRFX_CommandArgsDef* curr = defs ; curr->name ; curr++){
        if(strcmp(curr->name, name) == 0){
            return curr;
        }
        if(index) ++(*index);
    }
    return NULL;
}

static int ParseArg(RLUTILS_PRFX_CommandArgsDef* def, RLUTILS_PRFX_ArgIterator* iter, RLUTILS_PRFX_MemoryGuard* mg){
    switch(def->val.type){
    case BOOL:
        *(def->val.bval) = true;
        return REDISMODULE_OK;
    case LONG:
        return RLUTILS_PRFX_ArgIteratorNextLong(iter, def->val.lval);
    case DOUBLE:
        return RLUTILS_PRFX_ArgIteratorNextDouble(iter, def->val.dval);
    case STR:
        *(def->val.sval) = (char*)RLUTILS_PRFX_ArgIteratorNextCStr(iter);
        if(!(*(def->val.sval))){
            return REDISMODULE_ERR;
        }
        if (def->flags & CommandArgCopy){
            *(def->val.sval) = RLUTILS_PRFX_strdup(*(def->val.sval));
            RLUTILS_PRFX_MemoryGuardAddPtr(mg, *(def->val.sval));
        }
        return REDISMODULE_OK;
    case REDISSTR:
        *(def->val.rsval) = RLUTILS_PRFX_ArgIteratorNext(iter);
        if(!(*(def->val.rsval))){
            return REDISMODULE_ERR;
        }
        if (def->flags & CommandArgCopy){
            RedisModule_RetainString(NULL, *(def->val.rsval));
            RLUTILS_PRFX_MemoryGuardAddRedisString(mg, *(def->val.rsval));
        }
        return REDISMODULE_OK;
    case CALLBACK:
        return def->val.callback(iter);
    default:
        assert(false);
    }
    return REDISMODULE_ERR;
}

static ArgsMask GetMendatoryArgsMask(RLUTILS_PRFX_CommandArgsDef* defs){
    ArgsMask mask = 0;
    size_t index = 0;
    for(RLUTILS_PRFX_CommandArgsDef* curr = defs ; curr->name ; curr++){
        if(curr->flags & CommandArgMendatory){
            mask |= 1 << index;
        }
    }

    return mask;
}

#define SET_ERR(e, ...) \
        RLUTILS_PRFX_asprintf(&err, e, ##__VA_ARGS__); \
        goto error;

static size_t FindMissingArgIndex(ArgsMask mendatoryArgsMask){
    size_t index = 0;
    while(!(mendatoryArgsMask & 1)){
        ++index;
        assert(index < 64);
        mendatoryArgsMask = mendatoryArgsMask >> 1;
    }
    return index;
}

int RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* defs, RLUTILS_PRFX_ArgsParsingFlag flags){
    RLUTILS_PRFX_ArgIterator iter;
    RLUTILS_PRFX_MemoryGuard mg;

    char* err = NULL;
    int retVal = REDISMODULE_OK;

    RLUTILS_PRFX_ArgIteratorInit(&iter, argv, argc);
    RLUTILS_PRFX_MemoryGuardrInit(&mg);

    ArgsMask mendatoryArgsMask = GetMendatoryArgsMask(defs);

    RedisModuleString* curr;
    while((curr = RLUTILS_PRFX_ArgIteratorNext(&iter))){
        const char* argName = RedisModule_StringPtrLen(curr, NULL);
        size_t index;
        RLUTILS_PRFX_CommandArgsDef* def = FindArgDef(argName, defs, &index);

        if (!def && (flags & RaiseErrorOnUnknownArg)){
            SET_ERR("Unknown argument given : %s", def->name);
        }

        if(ParseArg(def, &iter, &mg) != REDISMODULE_OK){
            SET_ERR("Could not parse value for argument %s", def->name);
        }

        mendatoryArgsMask &= ~(1 << index);
    }

    if(mendatoryArgsMask){
        size_t missingIndex = FindMissingArgIndex(mendatoryArgsMask);
        SET_ERR("Mandatory arguments was not set : %s", defs[missingIndex].name);
    }

error:
    if (err){
        RedisModule_ReplyWithError(ctx, err);
        RLUTILS_PRFX_MemoryGuardFreeUnits(&mg);
        RLUTILS_PRFX_free(err);
    }
    RLUTILS_PRFX_MemoryGuardFreeStructure(&mg);
    return retVal;
}


