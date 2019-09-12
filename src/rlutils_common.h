/*
 * common.h
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#ifndef SRC_RLUTILS_COMMON_H_
#define SRC_RLUTILS_COMMON_H_

#include "rlutils.h"

#define str(s) #s
#define STR(var) str(var)
#define RedisModule_ReplyWithCStr(ctx, str) RedisModule_ReplyWithStringBuffer(ctx, str, strlen(str))

#endif /* SRC_RLUTILS_COMMON_H_ */
