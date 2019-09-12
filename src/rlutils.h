/*
 * rlutils.h
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#ifndef RLUTILS_H_
#define RLUTILS_H_

#include "redismodule.h"

typedef long long (*RLUTILS_PRFX_NumOfKeysCallback)();
typedef int (*RLUTILS_PRFX_DumpKeysCallback)(RedisModuleCtx* ctx);
typedef int (*RLUTILS_PRFX_ExtraInfoCallback)(RedisModuleCtx* ctx);

typedef struct RLUTILS_PRFX_ModuleData{
    int version;
    RLUTILS_PRFX_NumOfKeysCallback numOfKeysCallback;
    RLUTILS_PRFX_DumpKeysCallback dumpKeysCallback;
    RLUTILS_PRFX_ExtraInfoCallback extraInfoCallback;
}RLUTILS_PRFX_ModuleData;

extern RLUTILS_PRFX_ModuleData RLUTILS_PRFX_moduleData;

int RLUTILS_PRFX_InitRLUtils(RedisModuleCtx* ctx, RedisModuleString** argv, size_t argc, int version);

#endif /* RLUTILS_H_ */
