/**
 * Created by zyh on 2017/9/7.
**/
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "mylog.h"
#include "threads.h"

//方法ID被缓存
static jmethodID gOnNativeMessage = NULL;
//java虚拟机接口指针
static JavaVM* gVm = NULL;
//对象的全局引用
static jobject gObj = NULL;
//互斥实例
static pthread_mutex_t mutex;

void native_init(JNIEnv *env, jobject obj){
    //初始化互斥
    if(0!=pthread_mutex_init(&mutex,NULL)){
        //获取异常
        jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
        //抛出异常
        env->ThrowNew(exceptionClazz,"Unable to initialize mutex");
        goto exit;
    }
    //如果对象的全局引用未设置
    if(NULL == gObj){
        //为对象创建一个新的全局引用
        gObj = env->NewGlobalRef(obj);
        if(NULL == gObj){
            goto exit;
        }
    }
    //如果方法ID没被缓存
    if(NULL == gOnNativeMessage){
        //从对象中获取
        jclass clazz = env->GetObjectClass(obj);

        //获取回调方法ID
        gOnNativeMessage = env->GetMethodID(clazz,"onNativeMessage","(Ljava/lang/String;)V");

        //如果方法没有找到
        if(NULL == gOnNativeMessage){
            //获取异常
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");

            //抛出异常
            env->ThrowNew(exceptionClazz,"Unable to find method");
        }
    }
    exit:
        return;
}

//当不再使用时，需要正确的删除全局引入，否则会发生内存泄露
void native_free(JNIEnv *env, jobject obj){
    //如果对象的全局引用未设置
    if(NULL != gObj){
        //删除全局引用
        env->DeleteGlobalRef(gObj);
        gObj = NULL;
    }
    //销毁互斥锁
    if(0!=pthread_mutex_destroy(&mutex)){
        //获取异常类
        jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
        //抛出异常
        env->ThrowNew(exceptionClazz,"Unable to destroy mutex");
    }
}

void native_worker(JNIEnv *env, jobject obj, jint id, jint iterations){
    //锁定互斥锁
    if(0!= pthread_mutex_lock(&mutex)){
        //获取异常类
        jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
        //抛出异常
        env->ThrowNew(exceptionClazz,"Unable to lock mutex");
        goto exit;
    }
    //循环给定的迭代数
    for(jint i=0;i<iterations;i++){
        //准备消息
        char message[26];
        sprintf(message,"Worker %d: Iteration %d", id, i);

        //来自C字符串的消息
        jstring messageString = env->NewStringUTF(message);

        //调用原生消息方法
        env->CallVoidMethod(obj,gOnNativeMessage, messageString);

        //检查是否产生异常
        if(NULL != env->ExceptionOccurred())
            break;

        //睡眠一秒
        sleep(1);
    }
    //解锁互斥锁
    if(0!=pthread_mutex_unlock(&mutex)){
        //获取异常类
        jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
        //抛出异常
        env->ThrowNew(exceptionClazz,"Unable to unlock mutex");
    }
    exit:
        return;
}

//原生worker线程参数
struct NativeWorkerArgs{
    jint id;
    jint iterations;
};

//为原生Worker线程添加启动程序
static void* native_worker_thread(void* args){
    JNIEnv* env = NULL;
    //将当前线程附加到java虚拟机上,并且获得JNIEnv接口指针
    if(0 == gVm->AttachCurrentThread(&env,NULL)){
        //获取原生worker线程参数
        NativeWorkerArgs* nativeWorkerArgs = (NativeWorkerArgs*)args;
        //在线程上下文中运行原生worker
        native_worker(env,gObj,nativeWorkerArgs->id,nativeWorkerArgs->iterations);
        //释放原生worker线程参数
        delete nativeWorkerArgs;
        //从Java虚拟机中分离当前线程
        gVm->DetachCurrentThread();
    }
    return (void*) 1;
}

void posix_threads(JNIEnv *env, jobject obj, jint threads,jint iterations){
    MY_LOG_DEBUG("threads=%d iterations=%d", threads, iterations);
    //为每一个worker创建一个POSIX线程
    for(jint i=0;i<threads;i++){
        //原生worker线程参数
        NativeWorkerArgs* nativeWorkerArgs = new NativeWorkerArgs();
        nativeWorkerArgs->id = i;
        nativeWorkerArgs->iterations = iterations;

        //线程名柄
        pthread_t thread;
        //创建一个新线程
        int result = pthread_create(&thread,NULL,native_worker_thread,(void*)nativeWorkerArgs);
        if(0 != result){
            MY_LOG_DEBUG("exception result=%d", result);
            //获取异常类
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            //抛出异常
            env->ThrowNew(exceptionClazz,"Unableto create thread");
        }
    }
}

//使一个函数等待线程线程终止
void hold_posix_threads(JNIEnv *env, jobject obj, jint threads,jint iterations){
    MY_LOG_DEBUG("threads=%d iterations=%d", threads, iterations);
    //线程句柄
    pthread_t* handles = new pthread_t[threads];
    //为每一个worker创建一个POSIX线程
    for(jint i=0;i<threads;i++){
        //原生worker线程参数
        NativeWorkerArgs* nativeWorkerArgs = new NativeWorkerArgs();
        nativeWorkerArgs->id = i;
        nativeWorkerArgs->iterations = iterations;

        //线程名柄
        pthread_t thread;
        //创建一个新线程
        int result = pthread_create(&handles[i],NULL,native_worker_thread,(void*)nativeWorkerArgs);
        if(0 != result){
            MY_LOG_DEBUG("exception result=%d", result);
            //获取异常类
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            //抛出异常
            env->ThrowNew(exceptionClazz,"Unableto create thread");
            goto exit;
        }
    }
    //等待线程终止
    for(jint i=0;i<threads;i++){
        void* result = NULL;
        //连接每个线程句柄
        if(0!=pthread_join(handles[i],&result)){
            //获取异常类
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            //抛出异常
            env->ThrowNew(exceptionClazz, "Unable to join thread");
        }else{
            //准备message
            char message[26];
            sprintf(message, "Worker %d returned %p", i, result);
            //来自C字符串的message
            jstring messageString = env->NewStringUTF(message);
            //调用原生消息方法
            env->CallVoidMethod(obj,gOnNativeMessage,messageString);
            if(NULL != env->ExceptionOccurred()){
                goto exit;
            }
        }
    }
    exit:
        return;
}


static JNINativeMethod gMethods[] = {
    {
        "nativeInit",
        "()V",
        (void*)native_init
    },
    {
        "nativeFree",
        "()V",
        (void*)native_free
    },
    {
        "nativeWorker",
        "(II)V",
        (void*)native_worker
    },
    {
        "posixThreads",
        "(II)V",
        (void*)posix_threads
    },
    {
        "holdPosixThreads",
        "(II)V",
        (void*)hold_posix_threads
    }
};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved){
    //缓存Java虚拟机接口指针
    gVm = jvm;

    JNIEnv *env = NULL;
    jint result = JNI_FALSE;

    //获取env指针
    if(jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK){
        return result;
    }

    if(NULL == env){
        return result;
    }

    //获取类引用，写类的全路径（包+类名），FindClass等JNI函数将在后面详解
    jclass clazz = env->FindClass("com/year17/analyse_ndk_threads/MainActivity");
    if(clazz == NULL){
        return result;
    }

    //注册方法
    if(env->RegisterNatives(clazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0]))<0){
        return result;
    }

    //成功
    result = JNI_VERSION_1_6;
    return result;
}
