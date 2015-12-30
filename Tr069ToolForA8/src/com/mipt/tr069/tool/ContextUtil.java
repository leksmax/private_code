package com.mipt.tr069.tool;

import android.app.Application;


public class ContextUtil extends Application {
	private static ContextUtil mInstance = null;
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		mInstance = this;
		
		Utils.getInstance();
	}
	
	public static ContextUtil getInstance(){
		return mInstance;
	}
}
