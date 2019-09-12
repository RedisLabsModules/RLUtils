#include "rlutils_info.h"
#include "rlutils_common.h"
#include "rlutils.h"

#include <string.h>

static int ReplyInfoCmd(RedisModuleCtx *ctx){
    RedisModule_ReplyWithArray(ctx, 6);
    RedisModule_ReplyWithCStr(ctx, "Name");
    RedisModule_ReplyWithCStr(ctx, STR(RMUTILS_PRFX));
    RedisModule_ReplyWithCStr(ctx, "Version");
    RedisModule_ReplyWithLongLong(ctx, RMUTILS_PRFX_moduleData.version);
    RedisModule_ReplyWithCStr(ctx, "NumOfKeys");
    if(RMUTILS_PRFX_moduleData.numOfKeysCallback){
        RedisModule_ReplyWithLongLong(ctx, RMUTILS_PRFX_moduleData.numOfKeysCallback());
    }else{
        RedisModule_ReplyWithCStr(ctx, "Unavailable");
    }
    return REDISMODULE_OK;
}

int RMUTILS_PRFX_InfoCmd(RedisModuleCtx *ctx, RedisModuleString **argv, int argc){
    if (argc == 1){
        return ReplyInfoCmd(ctx);
    }

    if (argc == 2){
        const char* arg = RedisModule_StringPtrLen(argv[1], NULL);

        if(strcmp(arg, "keys") == 0){
            if(RMUTILS_PRFX_moduleData.dumpKeysCallback){
                return RMUTILS_PRFX_moduleData.dumpKeysCallback(ctx);
            }
            return RedisModule_ReplyWithError(ctx, "Num keys callback is not available");
        }

        if(strcmp(arg, "extra") == 0){
            if(RMUTILS_PRFX_moduleData.extraInfoCallback){
                return RMUTILS_PRFX_moduleData.extraInfoCallback(ctx);
            }
            return RedisModule_ReplyWithError(ctx, "Extra info callback is not available");
        }
    }

    // invalid arguments
    return RedisModule_WrongArity(ctx);
}

