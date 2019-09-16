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

RedisModuleString* RLUTILS_PRFX_ArgIteratorCurr(RLUTILS_PRFX_ArgIterator* iter){
    if(iter->location >= iter->argc){
        return NULL;
    }
    return iter->argv[iter->location];
}

RedisModuleString* RLUTILS_PRFX_ArgIteratorNext(RLUTILS_PRFX_ArgIterator* iter){
    RedisModuleString* curr = RLUTILS_PRFX_ArgIteratorCurr(iter);
    if(curr){
        ++iter->location;
    }
    return curr;
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

RLUTILS_PRFX_CommandNamedArgsDef* RLUTILS_PRFX_CommandArgsFindDef(const char* name, RLUTILS_PRFX_CommandNamedArgsDef* defs, size_t* index){
    if(index) *index = 0;
    for(RLUTILS_PRFX_CommandNamedArgsDef* curr = defs ; curr->name ; curr++){
        if(strcmp(curr->name, name) == 0){
            return curr;
        }
        if(index) ++(*index);
    }
    return NULL;
}

#define SET_ERR(buff, e, ...) \
        RLUTILS_PRFX_asprintf(buff, e, ##__VA_ARGS__); \

int RLUTILS_PRFX_ParseArg(RLUTILS_PRFX_CommandArgsDef* arg, RLUTILS_PRFX_ArgIterator* iter, RLUTILS_PRFX_MemoryGuard* mg, char** err){
    long long lval;
    double dval;
    char* sval;
    RedisModuleString* rsval;
    switch(arg->val.type){
    case BOOL:
        *(arg->val.bval) = true;
        return REDISMODULE_OK;
    case LONG:
        if(RLUTILS_PRFX_ArgIteratorNextLong(iter, &lval) != REDISMODULE_OK){
            SET_ERR(err, "failing to parse long value for argument");
            return REDISMODULE_ERR;
        }
        if(lval < arg->val.lvalMin || lval > arg->val.lvalMax){
            SET_ERR(err, "given value is out of range, value : %ld, range: %ld - %ld", lval, arg->val.lvalMin, arg->val.lvalMax);
            return REDISMODULE_ERR;
        }
        *arg->val.lval = lval;
        return REDISMODULE_OK;
    case DOUBLE:
        if(RLUTILS_PRFX_ArgIteratorNextDouble(iter, &dval) != REDISMODULE_OK){
            SET_ERR(err, "failing to parse double value for argument");
            return REDISMODULE_ERR;
        }
        if(dval < arg->val.dvalMin || dval > arg->val.dvalMax){
            SET_ERR(err, "given value is out of range, value : %lf, range: %lf - %lf", dval, arg->val.dvalMin, arg->val.dvalMax);
            return REDISMODULE_ERR;
        }
        *arg->val.dval = dval;
        return REDISMODULE_OK;
    case STR:
        sval = (char*)RLUTILS_PRFX_ArgIteratorNextCStr(iter);
        if(!sval){
            SET_ERR(err, "no value given to argument");
            return REDISMODULE_ERR;
        }
        if (arg->flags & FreeOldValue){
            RLUTILS_PRFX_free(*(arg->val.sval));
        }
        *(arg->val.sval) = sval;
        if (arg->flags & CommandArgCopy){
            *(arg->val.sval) = RLUTILS_PRFX_strdup(*(arg->val.sval));
            RLUTILS_PRFX_MemoryGuardAddPtr(mg, *(arg->val.sval));
        }
        return REDISMODULE_OK;
    case REDISSTR:
        rsval = RLUTILS_PRFX_ArgIteratorNext(iter);
        if(!rsval){
            SET_ERR(err, "no value given to argument");
            return REDISMODULE_ERR;
        }
        if (arg->flags & FreeOldValue){
            RedisModule_FreeString(NULL, *(arg->val.rsval));
        }
        *(arg->val.rsval) = rsval;
        if (arg->flags & CommandArgCopy){
            RedisModule_RetainString(NULL, *(arg->val.rsval));
            RLUTILS_PRFX_MemoryGuardAddRedisString(mg, *(arg->val.rsval));
        }
        return REDISMODULE_OK;
    case CALLBACK:
        return arg->val.setCallback(arg->val.ctx, iter);
    default:
        assert(false);
    }
    return REDISMODULE_ERR;
}

static ArgsMask GetMendatoryArgsMask(RLUTILS_PRFX_CommandNamedArgsDef* defs){
    ArgsMask mask = 0;
    size_t index = 0;
    for(RLUTILS_PRFX_CommandNamedArgsDef* curr = defs ; curr->name ; curr++){
        if(curr->arg.flags & CommandArgMendatory){
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

static int ParseArgs(RLUTILS_PRFX_CommandArgsDef* args, RLUTILS_PRFX_MemoryGuard* mg, RLUTILS_PRFX_ArgIterator* iter, char** err){
    if(!args){
        return REDISMODULE_OK;
    }
    while(args->val.lval){
        char* parsingErr = NULL;
        assert(args->val.type != BOOL && "bool val only supported with named args");
        if(RLUTILS_PRFX_ParseArg(args, iter, mg, &parsingErr) != REDISMODULE_OK){
            SET_ERR(err, "Failed setting argument value, %s", parsingErr);
            RLUTILS_PRFX_free(parsingErr);
            return REDISMODULE_ERR;
        }
        ++args;
    }
    return REDISMODULE_OK;
}

static int ParseArrayArgs(RLUTILS_PRFX_CommandArrVarPtr* arrArgs, RLUTILS_PRFX_MemoryGuard* mg,
                   RLUTILS_PRFX_ArgIterator* iter, char** err){
    if(!arrArgs){
        return REDISMODULE_OK;
    }

    if(arrArgs->type == CALLBACK){
        return arrArgs->setCallback(arrArgs->ctx, iter);
    }else{
        union{
            long long lval;
            double dval;
            bool bval;
            char* sval;
            RedisModuleString* rsval;
        } val;
        RLUTILS_PRFX_CommandArgsDef ptr = {
                .val.lval = &val.lval,
                .val.type = arrArgs->type,
                .flags = arrArgs->flags,
        };

        char* parsingErr = NULL;
        if(RLUTILS_PRFX_ParseArg(&ptr, iter, mg, &parsingErr) != REDISMODULE_OK){
            SET_ERR(err, "Failed setting argument value, %s", parsingErr);
            RLUTILS_PRFX_free(parsingErr);
            return REDISMODULE_ERR;
        }

        switch(arrArgs->type){
        case BOOL:
            *(arrArgs->bval) = array_append(*(arrArgs->bval), val.bval);
            break;
        case LONG:
            *(arrArgs->lval) = array_append(*(arrArgs->lval), val.lval);
            break;
        case DOUBLE:
            *(arrArgs->dval) = array_append(*(arrArgs->dval), val.dval);
            break;
        case STR:
            *(arrArgs->sval) = array_append(*(arrArgs->sval), val.sval);
            break;
        case REDISSTR:
            *(arrArgs->rsval) = array_append(*(arrArgs->rsval), val.rsval);
            break;
        default:
            assert(0);
        }
    }

    return REDISMODULE_OK;
}

static int ParseNamedArgs(RLUTILS_PRFX_CommandNamedArgsDef* namedArgs, RLUTILS_PRFX_MemoryGuard* mg,
                   RLUTILS_PRFX_ArgIterator* iter, char** err, RLUTILS_PRFX_ArgsParsingFlag flags){
    if(!namedArgs){
        return REDISMODULE_OK;
    }

    RedisModuleString* curr = NULL;
    ArgsMask mendatoryArgsMask = GetMendatoryArgsMask(namedArgs);

    // parse named args
    while((curr = RLUTILS_PRFX_ArgIteratorCurr(iter))){
        const char* argName = RedisModule_StringPtrLen(curr, NULL);
        size_t index;
        RLUTILS_PRFX_CommandNamedArgsDef* def = RLUTILS_PRFX_CommandArgsFindDef(argName, namedArgs, &index);

        if (!def){
            if(flags & RaiseErrorOnUnknownArg){
                SET_ERR(err, "Unknown argument given %s", argName);
                return REDISMODULE_ERR;
            }else{
                // found an unknown named argument move to the last parsing stage
                break;
            }
        }

        // advance the iterator before continue parsing
        RLUTILS_PRFX_ArgIteratorNext(iter);

        char* parsingErr = NULL;
        if(RLUTILS_PRFX_ParseArg(&def->arg, iter, mg, &parsingErr) != REDISMODULE_OK){
            SET_ERR(err, "Failed setting argument '%s' value, %s", argName, parsingErr);
            RLUTILS_PRFX_free(parsingErr);
            return REDISMODULE_ERR;
        }

        mendatoryArgsMask &= ~(1 << index);
    }

    if(mendatoryArgsMask){
        size_t missingIndex = FindMissingArgIndex(mendatoryArgsMask);
        SET_ERR(err, "Mandatory arguments was not set : %s", namedArgs[missingIndex].name);
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

int RLUTILS_PRFX_CommandArgsParseInternal(RedisModuleString** argv, size_t argc,
                                          RLUTILS_PRFX_CommandArgsDef* args,
                                          RLUTILS_PRFX_CommandNamedArgsDef* namedArgs,
                                          RLUTILS_PRFX_CommandArrVarPtr* arrPtr,
                                          RLUTILS_PRFX_ArgsParsingFlag flags, char** err){
#define GOTO_ERROR() \
    retVal = REDISMODULE_ERR; \
    goto error;

    RLUTILS_PRFX_ArgIterator iter;
    RLUTILS_PRFX_MemoryGuard mg;

    int retVal = REDISMODULE_OK;

    RLUTILS_PRFX_ArgIteratorInit(&iter, argv, argc);
    RLUTILS_PRFX_MemoryGuardrInit(&mg);

    // parsing args
    if(ParseArgs(args, &mg, &iter, err) != REDISMODULE_OK){
        GOTO_ERROR();
    }

    if(ParseNamedArgs(namedArgs, &mg, &iter, err, flags) != REDISMODULE_OK){
        GOTO_ERROR();
    }

    if(ParseArrayArgs(arrPtr, &mg, &iter, err) != REDISMODULE_OK){
        GOTO_ERROR();
    }

    if(RLUTILS_PRFX_ArgIteratorNext(&iter) && (flags & RaiseErrorOnExtraArgsLeft)){
        SET_ERR(err, "Argument without parsing def is given");
        GOTO_ERROR();
    }

error:
    if (*err){
        RLUTILS_PRFX_MemoryGuardFreeUnits(&mg);
    }
    RLUTILS_PRFX_MemoryGuardFreeStructure(&mg);
    return retVal;
}

int RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc,
                                  RLUTILS_PRFX_CommandArgsDef* args,
                                  RLUTILS_PRFX_CommandNamedArgsDef* namedArgs,
                                  RLUTILS_PRFX_CommandArrVarPtr* arrPtr,
                                  RLUTILS_PRFX_ArgsParsingFlag flags){
    char* err;
    if(RLUTILS_PRFX_CommandArgsParseInternal(argv, argc, args, namedArgs, arrPtr, flags, &err) != REDISMODULE_OK){
        RedisModule_ReplyWithError(ctx, err);
        RLUTILS_PRFX_free(err);
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}


