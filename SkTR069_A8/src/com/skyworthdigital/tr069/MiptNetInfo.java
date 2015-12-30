package com.skyworthdigital.tr069;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

public class MiptNetInfo extends BroadcastReceiver {
	private static final boolean DEBUG = true;

	private static final String TAG = "MiptNetInfo";
	private static final int TYPE_WIFI = 0;
	private static final int TYPE_ETH = 1;
	
	private static boolean isWifiConn = false;
	private static boolean isEthConn = false;

	private static final String ETHERNET_NAME = "ETHERNET";
	private static String wifiIpAddr = "0.0.0.0";
	private static String ethIpAddr = "0.0.0.0";

	public static String getIpAddr() {
		if(isEthConn)
			return ethIpAddr;
		else if(isWifiConn)
			return wifiIpAddr;
		else
			return "0.0.0.0";
	}

	private static void setIpAddr(String ipAddr,int type) {
		if(type == TYPE_WIFI){
			MiptNetInfo.wifiIpAddr = ipAddr;
		}else if(type == TYPE_ETH){
			MiptNetInfo.ethIpAddr = ipAddr;
		}
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		handleAction(context, intent);
	}

	private void handleAction(Context context, Intent intent) {
		String action = intent.getAction();
		
		if (WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) {
			
			final int event = intent.getIntExtra(
					WifiManager.EXTRA_WIFI_STATE,
					WifiManager.WIFI_STATE_ENABLED);
			
			switch (event) {
			case WifiManager.WIFI_STATE_ENABLED:
				WifiManager wifiManager = (WifiManager) context
					.getSystemService(Context.WIFI_SERVICE);
				WifiInfo wifiInfo = wifiManager.getConnectionInfo();
				if (wifiInfo != null) {
					isWifiConn = true;
					setIpAddr(intToIp(wifiInfo.getIpAddress()),TYPE_WIFI);
				}
				break;
			case WifiManager.WIFI_STATE_DISABLED:
				isWifiConn = false;
				break;
			default:
				isWifiConn = false;
			}

		} /*else if (EthernetManager.NETWORK_STATE_CHANGED_ACTION.equals(action)) { // because A5_framework.jar was deleted, so it was modified in 2014-1-24
			final LinkProperties linkProperties = (LinkProperties) intent
					.getParcelableExtra(EthernetManager.EXTRA_LINK_PROPERTIES);
			final int event = intent.getIntExtra(
					EthernetManager.EXTRA_ETHERNET_STATE,
					EthernetManager.EVENT_CONFIGURATION_SUCCEEDED);
			switch (event) {
			case EthernetManager.EVENT_CONFIGURATION_SUCCEEDED:
				for (LinkAddress l : linkProperties.getLinkAddresses()) {
					isEthConn = true;
					setIpAddr(l.getAddress().getHostAddress(),TYPE_ETH);
				}
				break;
			case EthernetManager.EVENT_CONFIGURATION_FAILED:
				isEthConn = false;
				break;
			default:
				isEthConn = false;
			}
		}*/
	}

	private String intToIp(int i) {
		return (i & 0xFF) + "." + ((i >> 8) & 0xFF) + "." + ((i >> 16) & 0xFF)
				+ "." + ((i >> 24) & 0xFF);
	}

}
