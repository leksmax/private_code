package com.skyworthdigital.tr069;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

public class SkyTR069Receiver extends BroadcastReceiver {
	private static final String Tag = "SkyTR069Receiver";
	
	private static boolean isBootRecieved = false;
	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		Log.d(Tag, "action = " + action);

		if(action.equals(Intent.ACTION_BOOT_COMPLETED)){
			if(isBootRecieved)	{
				Log.i(Tag, "tr069 is already booted! return!!!");
				return;
			}
			Log.i(Tag, "---ready to start tr069service!ACTION_BOOT_COMPLETED");
			
			
			//Toast.makeText(context, "SkyTR069Receiver-->ACTION_BOOT_COMPLETED", Toast.LENGTH_SHORT).show();
			Intent intentService = new Intent(context, SkyTR069Service.class);
			Bundle bundle  = new Bundle();
			bundle.putInt("action", SkyTR069Service.ACTION_START);
			intentService.putExtras(bundle);
			context.startService(intentService);
			isBootRecieved = true;
			
		} else if(action.equalsIgnoreCase("com.mipt.gotosleep")) {
			Log.i(Tag, "ready to start tr069service!");
			Intent intentService = new Intent(context, SkyTR069Service.class);
			Bundle bundle  = new Bundle();
			bundle.putInt("action", SkyTR069Service.ACTION_STOP);
			intentService.putExtras(bundle);
			//modify by lijingchao
			//context.startServiceAsUser(intentService,UserHandle.CURRENT);
			context.startService(intentService);
		} else if (action.equals("com.mipt.boot.complete")){
			if(isBootRecieved)	{
				Log.d(Tag, "tr069 is already booted! return!!!");
				return;
			}
			Log.d(Tag, "---ready to start tr069service!com.mipt.boot.complet");
			//Toast.makeText(context, "SkyTR069Receiver-->com.mipt.boot.complete", Toast.LENGTH_SHORT).show();
			Intent intentService = new Intent(context, SkyTR069Service.class);
			Bundle bundle  = new Bundle();
			bundle.putInt("action", SkyTR069Service.ACTION_START);
			intentService.putExtras(bundle);
			context.startService(intentService);
			isBootRecieved = true;
		}
		
		
		
	}
}
