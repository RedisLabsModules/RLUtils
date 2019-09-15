
#ifndef SRC_RLUTILS_COMMAND_ARGS_H_
#define SRC_RLUTILS_COMMAND_ARGS_H_

#include <stdbool.h>
#include "redismodule.h"

typedef struct RLUTILS_PRFX_ArgIterator RLUTILS_PRFX_ArgIterator;

typedef enum RLUTILS_PRFX_CommandArgsFlag{
    CommandArgMendatory = 0x01,
    CommandArgCopy = 0x02,
}RLUTILS_PRFX_CommandArgsFlag;

typedef enum RLUTILS_PRFX_ArgsParsingFlag{
    RaiseErrorOnUnknownArg = 0x01,
}RLUTILS_PRFX_ArgsParsingFlag;

typedef enum RLUTILS_PRFX_CommandArgsType{
    BOOL, LONG, DOUBLE, STR, REDISSTR, CALLBACK,
}RLUTILS_PRFX_CommandArgsType;

typedef struct RLUTILS_PRFX_CommandVarPtr{
    union{
        long long* lval;
        double* dval;
        bool* bval;
        char** sval;
        RedisModuleString** rsval;
        int (*callback)(RLUTILS_PRFX_ArgIterator* iter);
    };
    RLUTILS_PRFX_CommandArgsType type;
}RLUTILS_PRFX_CommandVarPtr;

typedef struct RLUTILS_PRFX_CommandArgsDef{
    const char* name;
    RLUTILS_PRFX_CommandVarPtr val;
    RLUTILS_PRFX_CommandArgsFlag flags;
}RLUTILS_PRFX_CommandArgsDef;

int RLUTILS_PRFX_CommandArgsParse(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, RLUTILS_PRFX_CommandArgsDef* def, RLUTILS_PRFX_ArgsParsingFlag flags);
RedisModuleString* RLUTILS_PRFX_ArgIteratorNext(RLUTILS_PRFX_ArgIterator* iter);

#endif /* SRC_RLUTILS_COMMAND_ARGS_H_ */
