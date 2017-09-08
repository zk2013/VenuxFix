LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= libdexposed
LOCAL_MODULE_TAGS := optional

# only system
LOCAL_LDLIBS :=-llog -lcutils  -L$(LOCAL_PATH)
# any lib
# http://blog.163.com/seven_7_one/blog/static/16260641220144992630878/

LOCAL_SRC_FILES:= dexposed.cpp
LOCAL_CFLAGS += -UNDEBUG -D_DEBUG -fpermissive
include $(BUILD_SHARED_LIBRARY)