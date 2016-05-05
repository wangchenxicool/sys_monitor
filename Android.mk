LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES	:= \
	main.cpp \
	my_popen.cpp

LOCAL_MODULE:= sys_monitor
#LOCAL_C_INCLUDES :=
#LOCAL_STATIC_LIBRARIES :=
#LOCAL_SHARED_LIBRARIES :=

APP_STL := stlport_static
LOCAL_LDLIBS    := -llog

include $(BUILD_EXECUTABLE)
