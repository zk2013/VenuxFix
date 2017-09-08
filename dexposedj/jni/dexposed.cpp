#include <stdlib.h>
#include <jni.h>
#include<android/log.h>
#include <dlfcn.h>
#include "dexposed.h"

#define TAG "zj" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

 typedef void (*PTR_dvmLogExceptionStackTrace)(void) ;
PTR_dvmLogExceptionStackTrace dvmLogExceptionStackTrace = NULL;

typedef void* (*PTR_dvmThreadSelf)();
PTR_dvmThreadSelf dvmThreadSelf = NULL;

typedef void* (*PTR_dvmDecodeIndirectRef)(void* self, void* jobj);
PTR_dvmDecodeIndirectRef dvmDecodeIndirectRef = NULL;

bool keepLoadingDexposed = false;
void (*PTR_atrace_set_tracing_enabled)(bool) = NULL;
int RUNNING_PLATFORM_SDK_VERSION = 0;

void* PTR_gDvmJit = NULL;
size_t arrayContentsOffset = 0;

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX  92
#endif

extern "C" int property_get(const char *key, char *value, const char *default_value);

void initTypePointers() {
    const char *error;
 char sdk[PROPERTY_VALUE_MAX];

 property_get("ro.build.version.sdk", sdk, "0");
LOGI("sdk = %s",sdk );
 RUNNING_PLATFORM_SDK_VERSION = atoi(sdk);

    dlerror();

     if (RUNNING_PLATFORM_SDK_VERSION >= 18) {
     *(void **) (&PTR_atrace_set_tracing_enabled) = dlsym(RTLD_DEFAULT, "atrace_set_tracing_enabled");
             if ((error = dlerror()) != NULL) {
                 LOGE("Could not find address for function atrace_set_tracing_enabled: %s", error);
             }
             dvmLogExceptionStackTrace = (PTR_dvmLogExceptionStackTrace)dlsym(RTLD_DEFAULT, "dvmLogExceptionStackTrace");
           if ((error = dlerror()) != NULL || NULL == dvmLogExceptionStackTrace ) {
                 LOGE("Could not find address for function dvmLogExceptionStackTrace: %s", error);
                }

int dvm_handle = (int)dlopen("libdvm.so",0);

               dvmThreadSelf = (PTR_dvmThreadSelf)dlsym(dvm_handle, "_Z13dvmThreadSelfv");
             if ((error = dlerror()) != NULL || NULL == dvmThreadSelf ) {
                            LOGE("Could not find address for function dvmThreadSelf: %s", error);
                        }
              dvmDecodeIndirectRef = (PTR_dvmDecodeIndirectRef)dlsym(dvm_handle, "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject");
                          if ((error = dlerror()) != NULL || NULL == dvmDecodeIndirectRef  ) {
                                         LOGE("Could not find address for function dvmDecodeIndirectRef: %s", error);
                                     }
     }
}

void dexposedInfo() {
    char release[PROPERTY_VALUE_MAX];
    char sdk[PROPERTY_VALUE_MAX];
    char manufacturer[PROPERTY_VALUE_MAX];
    char model[PROPERTY_VALUE_MAX];
    char rom[PROPERTY_VALUE_MAX];
    char fingerprint[PROPERTY_VALUE_MAX];


    property_get("ro.build.version.release", release, "n/a");
    property_get("ro.build.version.sdk", sdk, "n/a");
    property_get("ro.product.manufacturer", manufacturer, "n/a");
    property_get("ro.product.model", model, "n/a");
    property_get("ro.build.display.id", rom, "n/a");
    property_get("ro.build.fingerprint", fingerprint, "n/a");


    LOGI("Starting Dexposed binary version %s, compiled for SDK %d\n", DEXPOSED_VERSION, PLATFORM_SDK_VERSION);
    LOGD("Phone: %s (%s), Android version %s (SDK %s)\n", model, manufacturer, release, sdk);
    LOGD("ROM: %s\n", rom);
    LOGD("Build fingerprint: %s\n", fingerprint);
}

bool isRunningDalvik() {
    if (RUNNING_PLATFORM_SDK_VERSION < 19)
        return true;

    char runtime[PROPERTY_VALUE_MAX];
    property_get("persist.sys.dalvik.vm.lib", runtime, "");
    if (strcmp(runtime, "libdvm.so") != 0) {
        LOGE("Unsupported runtime library %s, setting to libdvm.so", runtime);
        return false;
    } else {
        LOGI("runtime library is %s", runtime);
    	return true;
    }
}

static bool dexposedInitMemberOffsets(JNIEnv* env)
{
    int ret = true;

    PTR_gDvmJit = dlsym(RTLD_DEFAULT, "gDvmJit");
    if (PTR_gDvmJit == NULL) {
            offsetMode = MEMBER_OFFSET_MODE_NO_JIT;
        } else {
            offsetMode = MEMBER_OFFSET_MODE_WITH_JIT;
     }
     LOGD("Using structure member offsets for mode %s", dexposedOffsetModesDesc[offsetMode]);

    // detect offset of ArrayObject->contents
    jintArray dummyArray = env->NewIntArray(1);


    if (dummyArray == NULL) {
        LOGE("Could allocate int array for testing");
        dvmLogExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

    jint* dummyArrayElements = env->GetIntArrayElements(dummyArray, NULL);
      LOGD("qaxdw");
      void* xx = dvmThreadSelf();
LOGD("dwdwdwe");
    arrayContentsOffset = (size_t)dummyArrayElements - (size_t)dvmDecodeIndirectRef(xx, dummyArray);
    LOGD("xxxx");
    env->ReleaseIntArrayElements(dummyArray,dummyArrayElements, 0);
    env->DeleteLocalRef(dummyArray);

    if (arrayContentsOffset < 12 || arrayContentsOffset > 128) {
        LOGE("Detected strange offset %d of ArrayObject->contents", arrayContentsOffset);
        return false;
    }
    else {
            LOGI("Detected  offset %d of ArrayObject->contents", arrayContentsOffset);
    }
    return ret;
}

/**
 * called in JNI_OnLoad , className id NULL
 */
bool dexposedOnVmCreated(JNIEnv* env, const char* className) {
    bool ret = true;

      keepLoadingDexposed = keepLoadingDexposed && dexposedInitMemberOffsets(env);
        if (!keepLoadingDexposed)
            return false;

    return ret;
}
extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    initTypePointers();
dexposedInfo();
 keepLoadingDexposed = isRunningDalvik();
 keepLoadingDexposed = dexposedOnVmCreated(env, NULL);

    return JNI_VERSION_1_6;
 }