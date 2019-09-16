
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
    RaiseErrorOnExtraArgsLeft = 0x02,
}RLUTILS_PRFX_ArgsParsingFlag;

typedef enum RLUTILS_PRFX_CommandArgsType{
    BOOL, LONG, DOUBLE, STR, REDISSTR, CALLBACK,
}RLUTILS_PRFX_CommandArgsType;

typedef struct RLUTILS_PRFX_CommandArrVarPtr{
    union{
        long long **lval;
        double **dval;
        bool **bval;
        char ***sval;
        RedisModuleString ***rsval;
        struct{
            void* ctx;
            int (*setCallback)(void* ctx, RLUTILS_PRFX_ArgIterator* iter);
        };
    };
    RLUTILS_PRFX_CommandArgsType type;
    RLUTILS_PRFX_CommandArgsFlag flags;
}RLUTILS_PRFX_CommandArrVarPtr;

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

#define RLUTILS_PRFX_CommandArgsDefDone() {.val.lval = NULL}
#define RLUTILS_PRFX_CommandArgsDefLong(longVal) {.val.lval = &longVal, .val.lvalMax = LONG_MAX, .val.lvalMin = LONG_MIN, .val.type = LONG, .flags = 0}
#define RLUTILS_PRFX_CommandArgsDefDouble(doubleVal) {.val.dval = &doubleVal, .val.dvalMax = LONG_MAX, .val.dvalMin = LONG_MIN, .val.type = DOUBLE, .flags = 0}
#define RLUTILS_PRFX_CommandArgsDefStr(strVal) {.val.sval = &strVal, .val.type = STR, .flags = 0}
#define RLUTILS_PRFX_CommandArgsDefRedisStr(strVal) {.val.rsval = &strVal, .val.type = REDISSTR, .flags = 0}
typedef struct RLUTILS_PRFX_CommandArgsDef{
    RLUTILS_PRFX_CommandVarPtr val;
    RLUTILS_PRFX_CommandArgsFlag flags;
}RLUTILS_PRFX_CommandArgsDef;


#define RLUTILS_PRFX_CommandNamedArgsDefDone() {.name = NULL}
#define RLUTILS_PRFX_CommandNamedArgsDefLong(n, longVal) {.name = n, .arg.val.lval = &longVal, .arg.val.lvalMax = LONG_MAX, .arg.val.lvalMin = LONG_MIN, .arg.val.type = LONG, .arg.flags = 0}
#define RLUTILS_PRFX_CommandNamedArgsDefDouble(n, doubleVal) {.name = n, .arg.val.dval = &doubleVal, .arg.val.dvalMax = LONG_MAX, .arg.val.dvalMin = LONG_MIN, .arg.val.type = DOUBLE, .arg.flags = 0}
#define RLUTILS_PRFX_CommandNamedArgsDefStr(n, strVal) {.name = n, .arg.val.sval = &strVal, .arg.val.type = STR, .arg.flags = 0}
#define RLUTILS_PRFX_CommandNamedArgsDefRedisStr(n, strVal) {.name = n, .arg.val.rsval = &strVal, .arg.val.type = REDISSTR, .arg.flags = 0}
#define RLUTILS_PRFX_CommandNamedArgsDefBool(n, boolVal) {.name = n, .arg.val.bval = &boolVal, .arg.val.type = BOOL, .arg.flags = 0}
typedef struct RLUTILS_PRFX_CommandNamedArgsDef{
    const char* name;
    RLUTILS_PRFX_CommandArgsDef arg;
}RLUTILS_PRFX_CommandNamedArgsDef;

typedef struct RLUTILS_PRFX_ArgIterator{
    RedisModuleString** argv;
    size_t argc;
    size_t location;
}RLUTILS_PRFX_ArgIterator;

RLUTILS_PRFX_CommandNamedArgsDef*
RLUTILS_PRFX_CommandArgsFindDef(const char* name, RLUTILS_PRFX_CommandNamedArgsDef* defs, size_t* index);
int
RLUTILS_PRFX_CommandArgsParseInternal(RedisModuleString** argv, size_t argc,
                                      RLUTILS_PRFX_CommandArgsDef* args,
                                      RLUTILS_PRFX_CommandNamedArgsDef* namedArgs,
                                      RLUTILS_PRFX_CommandArrVarPtr* arrPtr,
                                      RLUTILS_PRFX_ArgsParsingFlag flags, char** err);

int
RLUTILS_PRFX_ParseArg(RLUTILS_PRFX_CommandArgsDef* arg, RLUTILS_PRFX_ArgIterator* iter,
                      RLUTILS_PRFX_MemoryGuard* mg, char** err);

int
RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc,
                              RLUTILS_PRFX_CommandArgsDef* args,
                              RLUTILS_PRFX_CommandNamedArgsDef* namedArgs,
                              RLUTILS_PRFX_CommandArrVarPtr* arrPtr,
                              RLUTILS_PRFX_ArgsParsingFlag flags);

RedisModuleString* RLUTILS_PRFX_ArgIteratorCurr(RLUTILS_PRFX_ArgIterator* iter);

RedisModuleString* RLUTILS_PRFX_ArgIteratorNext(RLUTILS_PRFX_ArgIterator* iter);

void RLUTILS_PRFX_ArgIteratorInit(RLUTILS_PRFX_ArgIterator* iter,
                                  RedisModuleString** argv, size_t argc);

#endif /* SRC_RLUTILS_COMMAND_ARGS_H_ */
