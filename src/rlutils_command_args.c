#include "rlutils_command_args.h"
#include "redismodule.h"

#include "memory/rlutils_memory.h"
#include "utils/arr_rm_alloc.h"

#include <string.h>
#include <assert.h>

#define ArgsMask uint64_t

void RLUTILS_PRFX_ArgIteratorInit(RLUTILS_PRFX_ArgIterator* iter, RedisModuleString** argv, size_t argc){
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

RLUTILS_PRFX_CommandArgsDef* RLUTILS_PRFX_CommandArgsFindDef(const char* name, RLUTILS_PRFX_CommandArgsDef* defs, size_t* index){
    if(index) *index = 0;
    for(RLUTILS_PRFX_CommandArgsDef* curr = defs ; curr->name ; curr++){
        if(strcmp(curr->name, name) == 0){
            return curr;
        }
        if(index) ++(*index);
    }
    return NULL;
}

#define SET_ERR(buff, e, ...) \
        RLUTILS_PRFX_asprintf(buff, e, ##__VA_ARGS__); \

int RLUTILS_PRFX_ParseArg(RLUTILS_PRFX_CommandArgsDef* def, RLUTILS_PRFX_ArgIterator* iter, RLUTILS_PRFX_MemoryGuard* mg, char** err){
    long long lval;
    double dval;
    char* sval;
    RedisModuleString* rsval;
    switch(def->val.type){
    case BOOL:
        *(def->val.bval) = true;
        return REDISMODULE_OK;
    case LONG:
        if(RLUTILS_PRFX_ArgIteratorNextLong(iter, &lval) != REDISMODULE_OK){
            SET_ERR(err, "failing to parse long value for argument %s", def->name);
            return REDISMODULE_ERR;
        }
        if(lval < def->val.lvalMin || lval > def->val.lvalMax){
            SET_ERR(err, "given value is out of range, argument: %s, value : %ld, range: %ld - %ld", def->name, lval, def->val.lvalMin, def->val.lvalMax);
            return REDISMODULE_ERR;
        }
        *def->val.lval = lval;
        return REDISMODULE_OK;
    case DOUBLE:
        if(RLUTILS_PRFX_ArgIteratorNextDouble(iter, &dval) != REDISMODULE_OK){
            SET_ERR(err, "failing to parse double value for argument %s", def->name);
            return REDISMODULE_ERR;
        }
        if(dval < def->val.dvalMin || dval > def->val.dvalMax){
            SET_ERR(err, "given value is out of range, argument: %s, value : %lf, range: %lf - %lf", def->name, dval, def->val.dvalMin, def->val.dvalMax);
            return REDISMODULE_ERR;
        }
        *def->val.dval = dval;
        return REDISMODULE_OK;
    case STR:
        sval = (char*)RLUTILS_PRFX_ArgIteratorNextCStr(iter);
        if(!sval){
            SET_ERR(err, "no value given to argument %s", def->name);
            return REDISMODULE_ERR;
        }
        if (def->flags & FreeOldValue){
            RLUTILS_PRFX_free(*(def->val.sval));
        }
        *(def->val.sval) = sval;
        if (def->flags & CommandArgCopy){
            *(def->val.sval) = RLUTILS_PRFX_strdup(*(def->val.sval));
            RLUTILS_PRFX_MemoryGuardAddPtr(mg, *(def->val.sval));
        }
        return REDISMODULE_OK;
    case REDISSTR:
        rsval = RLUTILS_PRFX_ArgIteratorNext(iter);
        if(!rsval){
            SET_ERR(err, "no value given to argument %s", def->name);
            return REDISMODULE_ERR;
        }
        if (def->flags & FreeOldValue){
            RedisModule_FreeString(NULL, *(def->val.rsval));
        }
        *(def->val.rsval) = rsval;
        if (def->flags & CommandArgCopy){
            RedisModule_RetainString(NULL, *(def->val.rsval));
            RLUTILS_PRFX_MemoryGuardAddRedisString(mg, *(def->val.rsval));
        }
        return REDISMODULE_OK;
    case CALLBACK:
        return def->val.setCallback(def->val.ctx, iter);
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

static size_t FindMissingArgIndex(ArgsMask mendatoryArgsMask){
    size_t index = 0;
    while(!(mendatoryArgsMask & 1)){
        ++index;
        assert(index < 64);
        mendatoryArgsMask = mendatoryArgsMask >> 1;
    }
    return index;
}

int RLUTILS_PRFX_CommandArgsParseInternal(RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* defs, RLUTILS_PRFX_ArgsParsingFlag flags, char** err){
#define GOTO_ERROR() \
    retVal = REDISMODULE_ERR; \
    goto error;

    RLUTILS_PRFX_ArgIterator iter;
    RLUTILS_PRFX_MemoryGuard mg;

    int retVal = REDISMODULE_OK;

    RLUTILS_PRFX_ArgIteratorInit(&iter, argv, argc);
    RLUTILS_PRFX_MemoryGuardrInit(&mg);

    ArgsMask mendatoryArgsMask = GetMendatoryArgsMask(defs);

    RedisModuleString* curr;
    while((curr = RLUTILS_PRFX_ArgIteratorNext(&iter))){
        const char* argName = RedisModule_StringPtrLen(curr, NULL);
        size_t index;
        RLUTILS_PRFX_CommandArgsDef* def = RLUTILS_PRFX_CommandArgsFindDef(argName, defs, &index);

        if (!def && (flags & RaiseErrorOnUnknownArg)){
            SET_ERR(err, "Unknown argument given %s", argName);
            GOTO_ERROR();
        }

        if(RLUTILS_PRFX_ParseArg(def, &iter, &mg, err) != REDISMODULE_OK){
            GOTO_ERROR();
        }

        mendatoryArgsMask &= ~(1 << index);
    }

    if(mendatoryArgsMask){
        size_t missingIndex = FindMissingArgIndex(mendatoryArgsMask);
        SET_ERR(err, "Mandatory arguments was not set : %s", defs[missingIndex].name);
        GOTO_ERROR();
    }

error:
    if (*err){
        RLUTILS_PRFX_MemoryGuardFreeUnits(&mg);
    }
    RLUTILS_PRFX_MemoryGuardFreeStructure(&mg);
    return retVal;
}

int RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* defs, RLUTILS_PRFX_ArgsParsingFlag flags){
    char* err;
    if(RLUTILS_PRFX_CommandArgsParseInternal(argv, argc, defs, flags, &err) != REDISMODULE_OK){
        RedisModule_ReplyWithError(ctx, err);
        RLUTILS_PRFX_free(err);
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}


