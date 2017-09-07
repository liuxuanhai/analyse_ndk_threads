package com.year17.analyse_ndk_threads;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    /** thread edit **/
    private EditText threadEdit;
    /** iterations edit **/
    private EditText iterationEdit;
    /** start button **/
    private Button startButton;
    /** log view **/
    private TextView logView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //初始化原生代码
        nativeInit();

        threadEdit = (EditText)findViewById(R.id.thread_edit);
        iterationEdit = (EditText)findViewById(R.id.iteration_edit);
        startButton = (Button)findViewById(R.id.start_button);
        logView = (TextView)findViewById(R.id.log_view);

        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int threads = getNumber(threadEdit,0);
                int iterations = getNumber(iterationEdit,0);
                if(threads>0&&iterations>0){
                    startThread(threads,iterations);
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        //释放原生资源
        nativeFree();
        super.onDestroy();
    }

    /**
     * 由原生代码调用，用来向UI发送进度消息的回调函数
     * @param message
     */
    private void onNativeMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logView.append(message);
                logView.append("\n");
            }
        });
    }

    /**
     * 以integer格式获取编辑文本的值。
     * 如果值为空或计数不能分析，则返回默认值
     * @param editText
     * @param defaultView  默认值
     * @return
     */
    private static int getNumber(EditText editText,int defaultView){
        int value;
        try {
            value = Integer.parseInt(editText.getText().toString());
        }catch (NumberFormatException e){
            value = defaultView;
        }
        return value;
    }

    /**
     * 启动给定个数的线程进行迭代
     * @param threads
     * @param iterations
     */
    private void startThread(int threads,int iterations){
        javaThreads(threads,iterations);
    }

    /**
     * 使用基于Java的线程
     * @param threads
     * @param iterations
     */
    private void javaThreads(int threads, final int iterations){
        //为每一个worker创建一个基于Java的线程
        for(int i =0;i<threads;i++){
            final int id = i;
            Thread thread = new Thread(){
                @Override
                public void run() {
                    nativeWorker(id,iterations);
                }
            };
            thread.start();
        }
    }

    /**
     * 初始化原生代码
     */
    private native void nativeInit();

    /**
     * 释放原生资源
     */
    private native void nativeFree();

    /**
     * 原生worker
     */
    private native void nativeWorker(int id,int iterations);

    public static native String logFromJni();

    static {
        System.loadLibrary("NDK_THREAD");
    }
}
