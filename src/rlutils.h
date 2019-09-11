/*
 * rlutils.h
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#ifndef RLUTILS_H_
#define RLUTILS_H_

#include "redismodule.h"

typedef long long (*RMUTILS_PRFX_NumOfKeysCallback)();
typedef int (*RMUTILS_PRFX_DumpKeysCallback)(RedisModuleCtx* ctx);
typedef int (*RMUTILS_PRFX_ExtraInfoCallback)(RedisModuleCtx* ctx);

typedef struct RMUTILS_PRFX_ModuleData{
    int version;
    RMUTILS_PRFX_NumOfKeysCallback numOfKeysCallback;
    RMUTILS_PRFX_DumpKeysCallback dumpKeysCallback;
    RMUTILS_PRFX_ExtraInfoCallback extraInfoCallback;
}RMUTILS_PRFX_ModuleData;

extern RMUTILS_PRFX_ModuleData RMUTILS_PRFX_moduleData;

int RMUTILS_PRFX_InitRLUtils(RedisModuleCtx* ctx, int version);

#endif /* RLUTILS_H_ */
