#include <stdlib.h>
#include <jni.h>
#include<android/log.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "dexposed.h"

// com.venustv.dexposedj
#define DEXPOSED_CLASS "com/venustv/dexposedj/DexposedBridge"

#define TAG "zj" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

jclass dexposedClass = NULL;
ClassObject* objectArrayClass = NULL;

typedef void (*PTR_dvmCallMethod)(void* self, const Method* method, Object* obj,
    JValue* pResult, ...);
PTR_dvmCallMethod dvmCallMethod = NULL;

typedef ArrayObject* (*PTR_dvmAllocArrayByClass)(ClassObject* arrayClass,
    size_t length, int allocFlags);
PTR_dvmAllocArrayByClass  dvmAllocArrayByClass = NULL;

typedef ClassObject* (*PTR_dvmFindArrayClass)(const char* descriptor, Object* loader);
PTR_dvmFindArrayClass dvmFindArrayClass= NULL;

typedef Method* (*PTR_dvmSlotToMethod)(ClassObject* clazz, int slot);
PTR_dvmSlotToMethod dvmSlotToMethod = NULL;

typedef void (*PTR_dvmThrowIllegalArgumentException)(const char *);
PTR_dvmThrowIllegalArgumentException  dvmThrowIllegalArgumentException = NULL;

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

Method* dexposedHandleHookedMethod = NULL;

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX  92
#endif

extern "C" int property_get(const char *key, char *value, const char *default_value);


//_Z19dvmCheckClassAccessPK11ClassObjectS1_
uintptr_t dvmCheckClassAccess = 0;

//_Z19dvmCheckFieldAccessPK11ClassObjectPK5Field
uintptr_t dvmCheckFieldAccess = 0;

//_Z16dvmInSamePackagePK11ClassObjectS1_
uintptr_t dvmInSamePackage  = 0;

//_Z20dvmCheckMethodAccessPK11ClassObjectPK6Method
uintptr_t dvmCheckMethodAccess = 0;

void init_check_func(int dvm_handle) {
 dvmInSamePackage = dlsym(dvm_handle, "_Z16dvmInSamePackagePK11ClassObjectS1_");
   dvmCheckFieldAccess = dlsym(dvm_handle, "_Z19dvmCheckFieldAccessPK11ClassObjectPK5Field");
    dvmCheckClassAccess = dlsym(dvm_handle, "_Z19dvmCheckClassAccessPK11ClassObjectS1_");

    dvmCheckMethodAccess = dlsym(dvm_handle, "_Z20dvmCheckMethodAccessPK11ClassObjectPK6Method");

    dvmThrowIllegalArgumentException = (PTR_dvmThrowIllegalArgumentException)dlsym(dvm_handle, "_Z32dvmThrowIllegalArgumentExceptionPKc");
    dvmSlotToMethod = (PTR_dvmSlotToMethod)dlsym(dvm_handle, "_Z15dvmSlotToMethodP11ClassObjecti");
    dvmAllocArrayByClass = (PTR_dvmAllocArrayByClass)dlsym(dvm_handle, "dvmAllocArrayByClass");
    dvmFindArrayClass = (PTR_dvmFindArrayClass)dlsym(dvm_handle, "_Z17dvmFindArrayClassPKcP6Object");
    dvmCallMethod = (PTR_dvmCallMethod)dlsym(dvm_handle, "_Z13dvmCallMethodP6ThreadPK6MethodP6ObjectP6JValuez");
}

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

int dvm_handle = (int)dlopen("libdvm.so",RTLD_NOW);
init_check_func(dvm_handle);

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

#define PAGESIZE (4096)

static void replaceAsm(uintptr_t function, unsigned const char* newCode, size_t len) {
#ifdef __arm__
    function = function & ~1;
#endif
    uintptr_t pageStart = function & ~(PAGESIZE-1);
    size_t pageProtectSize = PAGESIZE;
    if (function+len > pageStart+pageProtectSize)
        pageProtectSize += PAGESIZE;

    mprotect((void*)pageStart, pageProtectSize, PROT_READ | PROT_WRITE | PROT_EXEC);
   //   LOGI("replaceAsm 1");
    memcpy((void*)function, newCode, len);
    mprotect((void*)pageStart, pageProtectSize, PROT_READ | PROT_EXEC);
 //LOGI("replaceAsm 2");
    __clear_cache((void*)function, (void*)(function+len));
}

static void patchReturnTrue(uintptr_t function) {
// direct return true
// why crash
//return ;
#ifdef __arm__

 //MOVS	R0, #1
 //BX	LR
    unsigned const char asmReturnTrueThumb[] = { 0x01, 0x20, 0x70, 0x47 };

   // mov r0,1
    //  bx lr
    unsigned const char asmReturnTrueArm[] = { 0x01, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 };

    LOGI("patchReturnTrue %08x", function);
    if (function & 1)
    {
    LOGI("patchReturnTrue arm thumb");
     replaceAsm(function, asmReturnTrueThumb, sizeof(asmReturnTrueThumb));
    }
    else
    {
        replaceAsm(function, asmReturnTrueArm, sizeof(asmReturnTrueArm));
        LOGI("patchReturnTrue arm arm");
    }

#else
  //  unsigned const char asmReturnTrueX86[] = { 0x31, 0xC0, 0x40, 0xC3 };
   // replaceAsm(function, asmReturnTrueX86, sizeof(asmReturnTrueX86));
#endif
}

__inline__ bool dvmIsStaticMethod(const Method* method) {
    return (method->accessFlags & ACC_STATIC) != 0;
}

static void dexposedCallHandler(const u4* args, void* pResult, const Method* method, void* self);

static inline bool dexposedIsHooked(const Method* method) {
    return (method->nativeFunc == &dexposedCallHandler);
}

static void dexposedCallHandler(const u4* args, void* pResult, const Method* method, void* self) {

    LOGI("dexposedCallHandler called");
    if (!dexposedIsHooked(method)) {
        dvmThrowIllegalArgumentException("could not find Dexposed original method - how did you even get here?");
        return;
    }

    DexposedHookInfo* hookInfo = (DexposedHookInfo*) method->insns;
    Method* original = (Method*) hookInfo;
    Object* originalReflected = hookInfo->reflectedMethod;
    Object* additionalInfo = hookInfo->additionalInfo;

    // convert/box arguments
    const char* desc = &method->shorty[1]; // [0] is the return type.
    Object* thisObject = NULL;
    size_t srcIndex = 0;
    size_t dstIndex = 0;

    // for non-static methods determine the "this" pointer
    if (!dvmIsStaticMethod(original)) {
        thisObject = (Object*) args[0];
        srcIndex++;
    }

    ArrayObject* argsArray = dvmAllocArrayByClass(objectArrayClass, strlen(method->shorty) - 1, ALLOC_DEFAULT);
    if (argsArray == NULL) {
        return;
    }
     // call the Java handler function
    JValue result;
    dvmCallMethod(self, dexposedHandleHookedMethod, NULL, &result,
        originalReflected, (int) original, additionalInfo, thisObject, argsArray);
}

static void com_taobao_android_dexposed_DexposedBridge_hookMethodNative(JNIEnv* env, jclass clazz, jobject reflectedMethodIndirect,
            jobject declaredClassIndirect, jint slot, jobject additionalInfoIndirect) {
        LOGI("com_taobao_android_dexposed_DexposedBridge_hookMethodNative called");

         if (declaredClassIndirect == NULL || reflectedMethodIndirect == NULL) {
               dvmThrowIllegalArgumentException("method and declaredClass must not be null");
                return;
            }
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

      keepLoadingDexposed = keepLoadingDexposed && dexposedInitMemberOffsets(env);
        if (!keepLoadingDexposed)
            return false;
  LOGD("has run here");

 // disable some access checks
 if (dvmCheckClassAccess)
    patchReturnTrue((uintptr_t) dvmCheckClassAccess);
else
    LOGD("dvmCheckClassAccess = NULL");
 LOGD("dvmCheckClassAccess = %08x",dvmCheckClassAccess);

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

 env->ExceptionClear();

    dexposedClass = env->FindClass(DEXPOSED_CLASS);
dexposedClass = reinterpret_cast<jclass>(env->NewGlobalRef(dexposedClass));
 if (dexposedClass == NULL) {
        LOGE("Error while loading Dexposed class '%s':\n", DEXPOSED_CLASS);
        dvmLogExceptionStackTrace();
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
        dvmLogExceptionStackTrace();
        env->ExceptionClear();
        keepLoadingDexposed = false;
        return false;
    }

    objectArrayClass = dvmFindArrayClass("[Ljava/lang/Object;", NULL);
    if (objectArrayClass == NULL) {
        LOGE("Error while loading Object[] class");
        dvmLogExceptionStackTrace();
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
 keepLoadingDexposed = isRunningDalvik();
 keepLoadingDexposed = dexposedOnVmCreated(env, NULL);

  initNative(env, NULL);

    return JNI_VERSION_1_6;
 }