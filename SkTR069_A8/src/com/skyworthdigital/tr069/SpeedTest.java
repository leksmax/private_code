package com.skyworthdigital.tr069;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class SpeedTest {  
	private String TAG =  "SpeedTest";
  
    private static final int LOADING = 0x111;  
    private static final int STOP = 0x112;  
    private ProgressBar mBar;  
    private int mProgressState;  
    private TextView mSpeed;  
    private Button mMeasureSpeed;  
    private float mSpeedContent;  
    private String mAddr = "http://cdn.market.hiapk.com/data/upload/2012/12_09/22/cn.lgx.phoneexpert_221804.apk";  
    private String mAddr2 = "http://gdown.baidu.com/data/wisegame/6f9153d4a8d1f7d8/QQ.apk";  
    private String mAddr3 = "http://gdown.baidu.com/data/wisegame/baidusearch_Android_10189_1399k.apk";  
    private int testCount = 0;  
    private Context mContext = null;
    private Handler mHandler = null;
    
    public SpeedTest(Context context, Handler handler) {
    	if (context == null || handler == null)
    		return;
    	mContext = context;
    	mHandler = handler;
	}
  
    public void measureSpeed(String httpUrl) {  
    	Log.d("TAG", "" + httpUrl);
        int fileLen = 0;  
        long startTime = 0;  
        long endTime = 0;  
        final String fileName = "tmp.apk";  
        HttpURLConnection conn = null;  
        InputStream is = null;  
        FileOutputStream fos = null;  
        File tmpFile = new File(mContext.getCacheDir() +  "/" + "temp");  
        Log.d("TAG", "" + tmpFile);
        if (!tmpFile.exists()) {  
            tmpFile.mkdir();  
        }  
        final File file = new File(mContext.getFilesDir() + "/" + fileName);  
        Log.d("TAG", "" + file);
        try {  
            URL url = new URL(httpUrl);  
            try {  
                conn = (HttpURLConnection) url.openConnection();  
                conn.setConnectTimeout(30000); 
                conn.setReadTimeout(30000); 
                fileLen = conn.getContentLength();  
                Log.d("TAG" , "fileLen is " + fileLen );
                if (fileLen <= 0) {  
                	Log.d("TAG", "fileLen <= 0");
                    mSpeedContent = 0;  
                    return;  
                }  
                startTime = System.currentTimeMillis();  
                is = conn.getInputStream();  
                fos = new FileOutputStream(file);  
                byte[] buf = new byte[256];  
                conn.connect();  
                Log.d("TAG", "connected");
                if (conn.getResponseCode() >= 400) {  
                    return;  
                } else {  
                    while (true) {  
                        if (is != null) {  
                            int numRead = is.read(buf);  
                            if (numRead <= 0) {  
                                break;  
                            } else {  
                                fos.write(buf, 0, numRead);  
                            }  
                            mProgressState += (int) (((numRead + 0.0) / (fileLen + 0.0)) * 1000000);  
                            // LogUtil.d("numRead=" + numRead + "  fileLen="  
                            // + fileLen);  
                        } else {  
                            break;  
                        }  
                    }  
                }  
                endTime = System.currentTimeMillis();  
            } catch (IOException e) {  
                e.printStackTrace();  
            } finally {  
            	Message msg = new Message();
                msg.what = TR069Params.BAND_WIDTH_TEST_ERROR;
                msg.obj = "102052";
                mHandler.sendMessage(msg);
                if (conn != null) {  
                    conn.disconnect();  
                }  
                try {  
                    if (fos != null) {  
                        fos.close();  
                    }  
                    if (is != null) {  
                        is.close();  
                    }  
                } catch (IOException e1) {  
                    e1.printStackTrace();  
                }  
  
            }  
        } catch (MalformedURLException e) {  
            e.printStackTrace();  
        }  
        mSpeedContent = fileLen / (endTime - startTime); 
        Message msg = new Message();
        msg.what = TR069Params.BAND_WIDTH_AVERAGE_SPEED;
        msg.arg1 = (int) mSpeedContent;
        mHandler.sendMessage(msg);
    }  
} 