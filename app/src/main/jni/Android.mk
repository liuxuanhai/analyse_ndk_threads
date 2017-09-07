#设置当前的编译目录（Android.mk所在的目录）
LOCAL_PATH := $(call my-dir)

#清除LOCAL_XX变量（LOCAL_PATH除外）
include $(CLEAR_VARS)
#指定当前编译模块的名称
LOCAL_MODULE := NDK_THREAD
#编译模块需要的源文件
LOCAL_SRC_FILES := jnidynamic.cpp jnilog.cpp threads.cpp

#定义日志标签，该标签在mylog.h中被使用
MY_LOG_TAG := \"tag\"
#为发布版本和调试版本定义不同的日志等级
ifeq ($(APP_OPTIM),release)
    MY_LOG_LEVEL := MY_LOG_LEVEL_ERROR
else
    MY_LOG_LEVEL := MY_LOG_LEVEL_VERBOSE
endif
#在定义MY_LOG_TAG和MY_LOG_LEVEL构建系统变量时，可以将日志系统配置应用到模块中
#追加编译标记
LOCAL_CFLAGS += -DMY_LOG_TAG=$(MY_LOG_TAG)
LOCAL_CFLAGS += -DMY_LOG_LEVEL=$(MY_LOG_LEVEL)

#添加日志功能,-L参数是指定了搜索lib的路径
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)
