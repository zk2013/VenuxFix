#include <stdlib.h>
#include <jni.h>
#include<android/log.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <pthread.h>
#include "dexposed.h"

// com.venustv.dexposedj
#define DEXPOSED_CLASS "com/venustv/dexposedj/DexposedBridge"
#define DEXPOSED_ADDITIONAL_CLASS "com/venustv/dexposedj/DexposedBridge$AdditionalHookInfo"

#define TAG "zj" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

jclass dexposedClass = NULL;
jclass additionalhookinfo_class = NULL;

Method* dexposedHandleHookedMethod = NULL;

bool keepLoadingDexposed = false;
int RUNNING_PLATFORM_SDK_VERSION = 0;

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX  92
#endif

extern "C" int property_get(const char *key, char *value, const char *default_value);

// art begin -----------------------------
static bool is_started_ = false;
static pthread_key_t pthread_key_self_ = NULL;
typedef Object * (*PTR_ThreadDecodeJObject)(void* Thread, jobject obj);
PTR_ThreadDecodeJObject ThreadDecodeJObject = NULL;
// art end -------------------------------

void init_check_func(int dvm_handle) {
    is_started_ =  *(bool*)(dlsym(dvm_handle, "_ZN3art6Thread11is_started_E"));
    LOGI("is_started_ = %d",is_started_ );
    pthread_key_self_ = *(pthread_key_t*)(dlsym(dvm_handle, "_ZN3art6Thread17pthread_key_self_E"));
    LOGI("pthread_key_self_ = %08x",pthread_key_self_ );

    ThreadDecodeJObject = (PTR_ThreadDecodeJObject)dlsym(dvm_handle, "_ZNK3art6Thread13DecodeJObjectEP8_jobject");
    LOGI("ThreadDecodeJObject = %08x",ThreadDecodeJObject );
}

extern "C" jobject com_taobao_android_dexposed_DexposedBridge_invokeOriginalMethodNative(
JNIEnv* env, jclass, jobject java_method, jint, jobject, jobject,
jobject thiz, jobject args) {
    LOGI("com_taobao_android_dexposed_DexposedBridge_invokeOriginalMethodNative");
    return NULL;
}

extern "C" jobject com_taobao_android_dexposed_DexposedBridge_invokeSuperNative(
JNIEnv* env, jclass, jobject thiz, jobject args, jobject java_method, jobject, jobject,
jint slot, jboolean check) {
      LOGI("com_taobao_android_dexposed_DexposedBridge_invokeOriginalMethodNative");
      return NULL;
}

void initTypePointers() {
    const char *error;
 char sdk[PROPERTY_VALUE_MAX];

 property_get("ro.build.version.sdk", sdk, "0");
LOGI("sdk = %s",sdk );
 RUNNING_PLATFORM_SDK_VERSION = atoi(sdk);

    dlerror();

     if (RUNNING_PLATFORM_SDK_VERSION >= 18) {
        int dvm_handle = (int)dlopen("libart.so",RTLD_NOW);
        init_check_func(dvm_handle);
/*
             dvmThreadSelf = (PTR_dvmThreadSelf)dlsym(dvm_handle, "_Z13dvmThreadSelfv");
             if ((error = dlerror()) != NULL || NULL == dvmThreadSelf ) {
                            LOGE("Could not find address for function dvmThreadSelf: %s", error);
                        }//*/

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

__inline__ bool dvmIsStaticMethod(const Method* method) {
    return (method->accessFlags & ACC_STATIC) != 0;
}

static void* Art_Thread_Current() {
    // We rely on Thread::Current returning NULL for a detached thread, so it's not obvious
    // that we can replace this with a direct %fs access on x86.
    if (!is_started_) {
      return NULL;
    } else {
      void* thread = pthread_getspecific(pthread_key_self_);
      return thread;
    }
}

static void com_taobao_android_dexposed_DexposedBridge_hookMethodNative(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
            jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect) {
         LOGI("com_taobao_android_dexposed_DexposedBridge_hookMethodNative called");

         if (declaredClassIndirect == NULL || reflectedMethodIndirect == NULL) {
                return;
            }
        Object* obj = ThreadDecodeJObject(Art_Thread_Current(), declaredClassIndirect);
        LOGI("declaredClass %08x", obj);

/*
        ClassObject* declaredClass = (ClassObject*) dvmDecodeIndirectRef(dvmThreadSelf(), declaredClassIndirect);
        if (declaredClass->descriptor != NULL ) {
        // Lcom/venustv/venuxfix/test/Cat;
            LOGI("declaredClass->descriptor %s",declaredClass->descriptor);
        }

        Method* method = dvmSlotToMethod(declaredClass, slot);
        if (method == NULL) {
         //   dvmThrowNoSuchMethodError("could not get internal representation for method");
         dvmThrowIllegalArgumentException("could not get internal representation for method");
            return;
        }
        LOGI("declaredClass->method %s",method->name);
        if (dexposedIsHooked(method)) {
            // already hooked
            LOGI("declaredClass->method %s already hooked",method->name);
            return;
        }
        LOGI("declaredClass->method %s not hooked",method->name);

         // Save a copy of the original method and other hook info
        DexposedHookInfo* hookInfo = (DexposedHookInfo*) calloc(1, sizeof(DexposedHookInfo));
        memcpy(hookInfo, method, sizeof(hookInfo->originalMethodStruct));
        hookInfo->reflectedMethod = dvmDecodeIndirectRef(dvmThreadSelf(), env->NewGlobalRef(reflectedMethodIndirect));
        hookInfo->additionalInfo = dvmDecodeIndirectRef(dvmThreadSelf(), env->NewGlobalRef(additionalInfoIndirect));
         // Replace method with our own code
        SET_METHOD_FLAG(method, ACC_NATIVE);
        method->nativeFunc = &dexposedCallHandler;

        method->insns = (const u2*) hookInfo;
        method->registersSize = method->insSize;
        method->outsSize = 0;

        /*
        if (PTR_gDvmJit != NULL) {
              // reset JIT cache
              MEMBER_VAL(PTR_gDvmJit, DvmJitGlobals, codeCacheFull) = true;
          }//*/
   }

static const JNINativeMethod dexposedMethods[] = {
    {"hookMethodNative", "(Ljava/lang/reflect/Member;Ljava/lang/Class;ILjava/lang/Object;)V", (void*)com_taobao_android_dexposed_DexposedBridge_hookMethodNative},
    { "invokeOriginalMethodNative","(Ljava/lang/reflect/Member;I[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;",
    							(void*) com_taobao_android_dexposed_DexposedBridge_invokeOriginalMethodNative },
   { "invokeSuperNative", "(Ljava/lang/Object;[Ljava/lang/Object;Ljava/lang/reflect/Member;Ljava/lang/Class;[Ljava/lang/Class;Ljava/lang/Class;I)Ljava/lang/Object;",
    				(void*) com_taobao_android_dexposed_DexposedBridge_invokeSuperNative},
};

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

static int register_com_taobao_android_dexposed_DexposedBridge(JNIEnv* env) {
    return env->RegisterNatives(dexposedClass, dexposedMethods, NELEM(dexposedMethods));
}

/**
 * called in JNI_OnLoad , className id NULL
 */
bool dexposedOnVmCreated(JNIEnv* env, const char* className) {
    bool ret = true;

      //keepLoadingDexposed = keepLoadingDexposed && dexposedInitMemberOffsets(env);
      //  if (!keepLoadingDexposed)
      //      return false;
  LOGD("has run here");
//int * ptr_xx = NULL;
//*ptr_xx = 9;
 // disable some access checks
// if (dvmCheckClassAccess)
  //  patchReturnTrue((uintptr_t) dvmCheckClassAccess);
//else
   // LOGD("dvmCheckClassAccess = NULL");
 //LOGD("dvmCheckClassAccess = %08x",dvmCheckClassAccess);

/*
 if (dvmCheckFieldAccess)
    patchReturnTrue((uintptr_t) &dvmCheckFieldAccess);
else
    LOGD("dvmCheckFieldAccess = NULL");

   if (dvmInSamePackage)
    patchReturnTrue((uintptr_t) &dvmInSamePackage);
else
    LOGD("dvmInSamePackage = NULL");

  if (dvmCheckMethodAccess)
    patchReturnTrue((uintptr_t) &dvmCheckMethodAccess);
else
    LOGD("dvmCheckMethodAccess = NULL");*/

// env->ExceptionClear();

    dexposedClass = env->FindClass(DEXPOSED_CLASS);
dexposedClass = reinterpret_cast<jclass>(env->NewGlobalRef(dexposedClass));
 if (dexposedClass == NULL) {
        LOGE("Error while loading Dexposed class '%s':\n", DEXPOSED_CLASS);
     //   dvmLogExceptionStackTrace();
        env->ExceptionClear();
        return false;
    }

	additionalhookinfo_class = env->FindClass(DEXPOSED_ADDITIONAL_CLASS);
    additionalhookinfo_class = reinterpret_cast<jclass>(env->NewGlobalRef(additionalhookinfo_class));
    if (additionalhookinfo_class == NULL) {
        LOGE("Error while loading Dexposed class '%s':\n", DEXPOSED_ADDITIONAL_CLASS);
        env->ExceptionClear();
        return false;
    }

    LOGI("Found Dexposed class '%s', now initializing\n", DEXPOSED_CLASS);
    if (register_com_taobao_android_dexposed_DexposedBridge(env) != JNI_OK) {
        LOGE("Could not register natives for '%s'\n", DEXPOSED_CLASS);
        return false;
    }

    return true;

}

static jboolean initNative(JNIEnv* env, jclass clazz) {

    dexposedHandleHookedMethod = (Method*) env->GetStaticMethodID(dexposedClass, "handleHookedMethod",
        "(Ljava/lang/reflect/Member;ILjava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    if (dexposedHandleHookedMethod == NULL) {
        LOGE("ERROR: could not find method %s.handleHookedMethod(Member, int, Object, Object, Object[])\n", DEXPOSED_CLASS);
        env->ExceptionClear();
        keepLoadingDexposed = false;
        return false;
    }
     return true;
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
 //keepLoadingDexposed = isRunningDalvik();
 keepLoadingDexposed = dexposedOnVmCreated(env, NULL);

 // initNative(env, NULL);

    return JNI_VERSION_1_6;
 }