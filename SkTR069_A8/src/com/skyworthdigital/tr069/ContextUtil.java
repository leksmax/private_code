package com.skyworthdigital.tr069;

import android.app.Application;
import android.content.Context;

public class ContextUtil extends Application {
	private static ContextUtil mInstance = null;
	private static Context mContext = null;
	
	public static ContextUtil getInstance(){
		return mInstance;
	}

	public static Context getAppContext(){
		return mContext;
	}
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
		
		mInstance = this;
		
		TR069Params.getInstance();
		
		mContext = getApplicationContext();
	}
}
