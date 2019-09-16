
#ifndef SRC_RLUTILS_COMMAND_ARGS_H_
#define SRC_RLUTILS_COMMAND_ARGS_H_

#include <stdbool.h>
#include <stddef.h>
#include "redismodule.h"
#include "memory/rlutils_memory.h"

typedef struct RLUTILS_PRFX_ArgIterator RLUTILS_PRFX_ArgIterator;

typedef enum RLUTILS_PRFX_CommandArgsFlag{
    CommandArgMendatory = 0x01,
    CommandArgCopy = 0x02,
    FreeOldValue = 0x04,
}RLUTILS_PRFX_CommandArgsFlag;

typedef enum RLUTILS_PRFX_ArgsParsingFlag{
    RaiseErrorOnUnknownArg = 0x01,
}RLUTILS_PRFX_ArgsParsingFlag;

typedef enum RLUTILS_PRFX_CommandArgsType{
    BOOL, LONG, DOUBLE, STR, REDISSTR, CALLBACK,
}RLUTILS_PRFX_CommandArgsType;

typedef struct RLUTILS_PRFX_CommandVarPtr{
    union{
        struct{
            long long* lval;
            long long lvalMax;
            long long lvalMin;
        };
        struct{
            double* dval;
            double dvalMax;
            double dvalMin;
        };
        bool* bval;
        char** sval;
        RedisModuleString** rsval;
        struct{
            void* ctx;
            int (*setCallback)(void* ctx, RLUTILS_PRFX_ArgIterator* iter);
            int (*getCallback)(RedisModuleCtx* rctx, void* ctx);
        };
    };
    RLUTILS_PRFX_CommandArgsType type;
}RLUTILS_PRFX_CommandVarPtr;

typedef struct RLUTILS_PRFX_CommandArgsDef{
    const char* name;
    RLUTILS_PRFX_CommandVarPtr val;
    RLUTILS_PRFX_CommandArgsFlag flags;
}RLUTILS_PRFX_CommandArgsDef;

typedef struct RLUTILS_PRFX_ArgIterator{
    RedisModuleString** argv;
    size_t argc;
    size_t location;
}RLUTILS_PRFX_ArgIterator;

RLUTILS_PRFX_CommandArgsDef* RLUTILS_PRFX_CommandArgsFindDef(const char* name, RLUTILS_PRFX_CommandArgsDef* defs, size_t* index);
int RLUTILS_PRFX_CommandArgsParseInternal(RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* defs, RLUTILS_PRFX_ArgsParsingFlag flags, char** err);
int RLUTILS_PRFX_ParseArg(RLUTILS_PRFX_CommandArgsDef* def, RLUTILS_PRFX_ArgIterator* iter, RLUTILS_PRFX_MemoryGuard* mg, char** err);
int RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* def, RLUTILS_PRFX_ArgsParsingFlag flags);
RedisModuleString* RLUTILS_PRFX_ArgIteratorNext(RLUTILS_PRFX_ArgIterator* iter);
void RLUTILS_PRFX_ArgIteratorInit(RLUTILS_PRFX_ArgIterator* iter, RedisModuleString** argv, size_t argc);

#endif /* SRC_RLUTILS_COMMAND_ARGS_H_ */
