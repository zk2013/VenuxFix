//
// Created by zhengkai on 2017/9/8.
//

#ifndef VENUXFIX_DEXPOSED_H
#define VENUXFIX_DEXPOSED_H

#define DEXPOSED_VERSION "51"
#define PLATFORM_SDK_VERSION 9


// here are the definitions of the modes and offsets
enum dexposedOffsetModes {
    MEMBER_OFFSET_MODE_WITH_JIT,
    MEMBER_OFFSET_MODE_NO_JIT,
};

static dexposedOffsetModes offsetMode;
const char* dexposedOffsetModesDesc[] = {
    "WITH_JIT",
    "NO_JIT",
};

#endif //VENUXFIX_DEXPOSED_H
