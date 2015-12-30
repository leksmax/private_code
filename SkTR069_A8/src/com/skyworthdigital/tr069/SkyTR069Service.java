package com.skyworthdigital.tr069;

import java.io.File;
import java.io.InputStream;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;

public class SkyTR069Service extends Service {
	private static final String TAG = "SkyTR069Service";
	public static final int ACTION_START = 1;
	public static final int ACTION_STOP = 2;	
	private static final int MSG_DELAY_2_SECONDS_SKTR069SERVICE = 1001;
	
	private static final int MSG_START_TR069 = 1002;
	private static final int MSG_STOP_TR069 = 1003;
	
	private static  boolean m_screen_on_or_off = true;
	private static boolean m_first_used_flag = true;
	
	private TR069Params mParam;
	private Handler mHandler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			switch (msg.what) {
			case MSG_DELAY_2_SECONDS_SKTR069SERVICE:
				Log.i(TAG, "output MSG_DELAY_2_SECONDS_SKTR069SERVICE by James");
				checkStartTR069();
				break;
				
			case MSG_START_TR069:
				Log.i(TAG, "output MSG_START_TR069 by James");
				checkStartTR069();
				break;
				
			case MSG_STOP_TR069:
				Log.i(TAG, "output MSG_STOP_TR069 by James");
				StopTR069();
				break;
				
			default:
				break;
			}
		}		
	};
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		Log.e(TAG, "SkyTR069Service onCreate()");
	//	Toast.makeText(this, "SkyTR069Service oncreate", Toast.LENGTH_SHORT).show();
		
		IntentFilter mintentfilter = new IntentFilter();
		//mintentfilter.setPriority(1000); 
		mintentfilter.addAction("android.intent.action.SCREEN_OFF");		
		mintentfilter.addAction("android.intent.action.SCREEN_ON");	
		mintentfilter.addAction("com.mipt.powerkey.pressed");
		
		this.registerReceiver(new BroadcastReceiver(){
			
			@Override
			public void onReceive(Context context, Intent intent) {
				String action = intent.getAction();
				Log.i(TAG, "action = " + action);
				
				if(action.equals("android.intent.action.SCREEN_ON")){
					
					Log.i(TAG, "receive android.intent.action.SCREEN_ON ! m_screen_on_or_off = " + m_screen_on_or_off);
					
					
					if(m_first_used_flag)
					{
						m_first_used_flag = false;	
						return;
					}
					
					/*
					if(false == m_screen_on_or_off)
					{
						m_screen_on_or_off = true;
						Log.i(Tag, "sendEmptyMessageDelayed  MSG_START_TR069! start");
						mHandler.sendEmptyMessageDelayed(MSG_START_TR069 , 0);
						
					}
					*/
					Log.i(TAG, "sendEmptyMessageDelayed  MSG_START_TR069! start");
					mHandler.sendEmptyMessageDelayed(MSG_START_TR069 , 0);
					
				}else if(action.equals("android.intent.action.SCREEN_OFF")){
					
					Log.i(TAG, "receive android.intent.action.SCREEN_OFF ! m_screen_on_or_off = " + m_screen_on_or_off);
					/*
					if(true == m_screen_on_or_off)
					{
						m_screen_on_or_off = false;
						Log.i(Tag, "sendEmptyMessageDelayed  MSG_STOP_TR069! stop");
						mHandler.sendEmptyMessageDelayed(MSG_STOP_TR069 , 0);
						
					}
					*/
					
				}else if(action.equals("com.mipt.powerkey.pressed")){
					
					Log.i(TAG, "receive com.mipt.powerkey.pressed ! m_screen_on_or_off = "+ m_screen_on_or_off);
					
					/*
					if(true == m_screen_on_or_off)
					{
						m_screen_on_or_off = false;
						Log.i(Tag, "sendEmptyMessageDelayed  MSG_STOP_TR069! stop");
						mHandler.sendEmptyMessageDelayed(MSG_STOP_TR069 , 0);
						
					}
					else 
					{	
						m_screen_on_or_off = true;
						Log.i(Tag, "sendEmptyMessageDelayed  MSG_START_TR069! start");
						mHandler.sendEmptyMessageDelayed(MSG_START_TR069 , 0);
						
					}
					*/
						
					Log.i(TAG, "sendEmptyMessageDelayed  MSG_STOP_TR069! stop");
					mHandler.sendEmptyMessageDelayed(MSG_STOP_TR069 , 0);
				}
				
				
			}
		}, mintentfilter);

			
		
	}

	@Override
	public void onDestroy() {
		Log.e(TAG, "SkyTR069Service onDestroy()");
	
		super.onDestroy();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		if (intent != null) {
            Bundle bundle = intent.getExtras();
            if (bundle != null) {
                int action = bundle.getInt("action");
                switch (action) {
                case ACTION_START:
                	//checkStartTR069();
                	//mHandler.sendEmptyMessageDelayed(MSG_DELAY_2_SECONDS_SKTR069SERVICE, 20000);
                	mHandler.sendEmptyMessageDelayed(MSG_DELAY_2_SECONDS_SKTR069SERVICE, 0);
                	break;

                case ACTION_STOP:
                	mHandler.sendEmptyMessageDelayed(MSG_STOP_TR069, 0);
                    break;
                    
                default:
                	mHandler.sendEmptyMessageDelayed(MSG_DELAY_2_SECONDS_SKTR069SERVICE, 0);
                	break;
                }
            }else{
            	mHandler.sendEmptyMessageDelayed(MSG_DELAY_2_SECONDS_SKTR069SERVICE, 0);
            }
        }else{
        	mHandler.sendEmptyMessageDelayed(MSG_DELAY_2_SECONDS_SKTR069SERVICE, 0);
        }
		return START_STICKY;
	}

	private void checkStartTR069(){
		
			Log.e(TAG, "TR069Service is start!!!");
	//		Toast.makeText(this, "TR069Service is start", Toast.LENGTH_SHORT).show();
			mParam = TR069Params.getInstance();
			
			write_res_to_disk("config/", "tr069_model.xml");
			mParam.setConfigPath(getFilesDir().getParent() + "/files/");
			mParam.doNativeTest();
			
	}
	
	private void StopTR069(){
			Log.i(TAG, "TR069Service is stop!!!");
			mParam = TR069Params.getInstance();
			
			//write_res_to_disk("config/", "tr069_model.xml");
			//mParam.setConfigPath(getFilesDir().getParent() + "/files/");
			
			mParam.doNativeStop();
	}
	
	public void write_res_to_disk(String src_path, String src_file)
	{
		String srcfile = src_path + src_file;
		String destfile = getFilesDir().getParent() + "/files/" + src_file;
		
		try
		{
			SkFileIO fileIO = new SkFileIO(this);
			if(!fileIO.isConfigExists(destfile)){
				Log.d(TAG, "write_res_to_disk:srcfile:" + srcfile + " destfile:" + destfile);
				fileIO.writeResToDisk(srcfile, src_file);
			}
			else
			{
		
				InputStream fis = this.getResources().getAssets().open(srcfile);  
				int src_file_size = fis.available();
				Log.d(TAG, "srcfile filesize = " +  src_file_size);
				fis.close(); 
				
				
				File f  = new File(destfile);
				long dst_file_size = f.length();
				Log.d(TAG, "dstfile filesize = " +  dst_file_size);
				
				if(src_file_size != dst_file_size)
				{
					Log.d(TAG, "file is different !\n write_res_to_disk:srcfile: " + srcfile + " destfile:" + destfile);
					fileIO.writeResToDisk(srcfile, src_file);
				}
				else
				{
					
					Log.d(TAG, "file tr069_model.xml is sample!\n ");
				}
				
			}
		}
		catch (Exception e) 
		{
			e.printStackTrace();
		}
	}
}
