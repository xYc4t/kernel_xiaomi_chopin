#ifndef __KSU_H_APK_V2_SIGN
#define __KSU_H_APK_V2_SIGN

#include <linux/types.h>
#include "ksu.h"

bool ksu_is_manager_apk(char *path);

bool is_dynamic_manager_apk(char *path, int *signature_index);

#endif
