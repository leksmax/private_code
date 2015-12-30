package com.skyworthdigital.tr069;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class MemoryInfo {
	private final String TAG = "MemoryInfo";
	private Context mContext = null;
	private Handler mHandler = null;
	
	public MemoryInfo(Context context, Handler handler) {
		if (context == null || handler == null){
			Log.d(TAG, "context or handler is null");
			return;
		}
		mContext = context;
		mHandler = handler;
	}
	
	public void getMeminfo(){
		String memfreeLine = getProcMeminfoSecondLine();
		String freeCount = memfreeLine.split(" ")[1]; //µ¥Î»ÊÇkb
		Message msg = new Message();
		msg.what = TR069Params.PROPERTY_SUPERVISE_MEM_FREE_UPDATE;
		msg.arg1 = Integer.parseInt(freeCount);
		mHandler.sendMessage(msg);
	}
	
	private String getProcMeminfoSecondLine() {
		String str = null;
		try {
			FileInputStream in = new FileInputStream("/proc/meminfo");
			InputStreamReader inReader = new InputStreamReader(in);
			BufferedReader reader = new BufferedReader(inReader);
			while((str = reader.readLine()) != null){
				if (str.contains("MemFree"))
					break;
			}
			reader.close();
			Log.d(TAG, "getProcMeminfoSecondLine = " + str);
		} catch (IOException ex) {
			Log.e(TAG, "IOException" + ex.toString());
			return null;
		}

		str = str.replaceAll("\\s{1,}", " ");
		return str;
	}
}
