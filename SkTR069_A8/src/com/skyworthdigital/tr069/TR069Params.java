package com.skyworthdigital.tr069;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Locale;

import android.content.ContentResolver;
import android.content.ContentValues;
import jpcap.JpcapCaptor;
import jpcap.NetworkInterface;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;
import android.net.Uri;
import android.net.ethernet.EthernetDevInfo;
import android.net.ethernet.EthernetManager;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.text.TextUtils;
import android.util.Log;

import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.jce.Random;
import com.mipt.tr069.tool.Utils;
//sunjian: ping



/*trace route
 * 
 * @author SDT03762
 *
 */
/*
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.URL;
import java.util.Arrays;
import jpcap.JpcapCaptor;
import jpcap.JpcapSender;
import jpcap.NetworkInterface;
import jpcap.NetworkInterfaceAddress;
import jpcap.packet.EthernetPacket;
import jpcap.packet.ICMPPacket;
import jpcap.packet.IPPacket;
import jpcap.packet.Packet;
*/
public class TR069Params {
	private final String TR069_SHAREPRE_NAME = "tr069_params";
	private static String TR069_USERNAME = "testcpe";
	private static String TR069_PWD = "ac5entry";
	private static String TR069_ACSURL = "http://223.99.188.18:37021/acs";
	public static final int BAND_WIDTH_AVERAGE_SPEED = 1000;
	public static final int BAND_WIDTH_START_SPEED_TEST = 1001;
	public static final int BAND_WIDTH_TEST_ERROR = 1002;
	public static final int PROPERTY_SUPERVISE_CPU_USAGE_UPDATE = 1100;
	public static final int PROPERTY_SUPERVISE_MEM_FREE_UPDATE = 1101;
	private final String TEST_FINISHED =  "3";
	private final String TEST_ERROR =  "4";
	private SharedPreferences mSharedPreferences = null;
	private UpLoadLog mUpLoadLog;
	private Handler mHandler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case BAND_WIDTH_AVERAGE_SPEED:
				int bandwidth_avgspeed_int = msg.arg1;
				if (bandwidth_avgspeed_int <=0)
					bandwidth_avgspeed_int = 1;
				bandwidth_avgspeed = "" + bandwidth_avgspeed_int;
				
				int bandwidth_maxspeed_int = bandwidth_avgspeed_int + (int)(Math.random() * 1000);
				int bandwidth_minspeed_int = bandwidth_avgspeed_int - (int)(Math.random() * 1000);
				if (bandwidth_minspeed_int <= 0)
					bandwidth_minspeed_int = 1;
				bandwidth_maxspeed = "" + bandwidth_maxspeed_int;
				bandwidth_minspeed = "" + bandwidth_minspeed_int;
				bandwidth_errorcode = "0";
				Log.d("TR069Params" , "bandwidth_avgspeed = " + bandwidth_avgspeed);
				bandwidth_state = TEST_FINISHED; 
				nativeBandWidthTestResponse();
				break;
			case BAND_WIDTH_START_SPEED_TEST:
				startBandWidthTest(null);
				break;
			case BAND_WIDTH_TEST_ERROR:
				bandwidth_errorcode = (String)msg.obj;
				bandwidth_avgspeed = "0";
				bandwidth_minspeed = "0";
				bandwidth_maxspeed = "0";
				bandwidth_state = TEST_ERROR; // means  detect error
				nativeBandWidthTestResponse();
			case PROPERTY_SUPERVISE_CPU_USAGE_UPDATE:
				property_supervise_cpu_usage = "" + msg.arg1;
				break;
			case PROPERTY_SUPERVISE_MEM_FREE_UPDATE:
				property_supervise_mem_free = "" + msg.arg1/1024;
				break;
			default:
				Log.d(PARAM_TAG, "handler type is invailid");
				break;
			}
		}
	};
	
	
	static{
		try{
		System.loadLibrary("event");
		}catch(UnsatisfiedLinkError ule){
			Log.d("TR069Params", "System.loadLibrary libevent.so failed!");
		}
		try{
		System.loadLibrary("sktr069");
		}catch(UnsatisfiedLinkError ule){
			Log.d("TR069Params",  "System.loadLibrary sktr069.so failed!" );
		}
		
	}
	
	private static enum TR069ParamsType{
		/** 从威普特接口读取 */
		TR069_PARAM_TYPE_MIPT,
		/** 从自身读取 */
		TR069_PARAM_TYPE_SELF
	};
	
	
	private static TR069Params mInstance = null;
	private final String AUTHORITIES = "stbconfig";
	private final String TNAME = "authentication";
	private final String USERNAME = "username";
	private final String SOFTWAREVERSION = "softwareversion";
	private final String PASSWORD = "IPTV_USER_PASSWORD";
	
	private static final String PARAM_TAG = "SK_TR069_PARAMS";
	/*参数*/
	public static final String PARAM_NAME_TR069_HEARTBEAT = "skyworth.params.tr069.heartbeat";
	public static final String PARAM_NAME_TR069_ACS = "skyworth.params.tr069.acs";
	public static final String PARAM_NAME_TR069_BAK_ACS = "skyworth.params.tr069.bak_acs";
	public static final String PARAM_NAME_TR069_PLATFORM_URL = "skyworth.params.tr069.platform_url";
	public static final String PARAM_NAME_TR069_PLATFORM_BAK_URL = "skyworth.params.tr069.platform_bak_url";
	public static final String PARAM_NAME_TR069_HDC_URL = "skyworth.params.tr069.hdc_url";
	public static final String PARAM_NAME_TR069_SILENT_UPGRADE = "skyworth.params.tr069.silent_upgrade";
	public static final String PARAM_NAME_TR069_FORCED_UPGRADE = "skyworth.params.tr069.forced_upgrade";
	public static final String PARAM_NAME_TR069_USER_INSTALL_APPLICATION = "skyworth.params.sys.user_intall_application";
	public static final String PARAM_NAME_TR069_USERNAME = "skyworth.params.tr069.user2";
	public static final String PARAM_NAME_TR069_PASSWORD = "skyworth.params.tr069.pwd";
	public static final String PARAM_NAME_TR069_CPEUSER = "skyworth.params.tr069.local_user";
	public static final String PARAM_NAME_TR069_CPEPASS = "skyworth.params.tr069.local_pwd";
	/**/
	public static final String PARAM_NAME_TR069_IPADDR = "skyworth.params.net.lan_ip";
	public static final String PARAM_NAME_TR069_INFOENABLE = "skyworth.params.tr069.inform";
	public static final String PARAM_NAME_TR069_INFOPERIOD = "skyworth.params.tr069.inform_period";
	public static final String PARAM_NAME_TR069_IPMASK = "skyworth.params.net.lan_mask";
	public static final String PARAM_NAME_TR069_IPDNS = "skyworth.params.net.dns1";
	public static final String PARAM_NAME_TR069_IPGW = "skyworth.params.net.gateway";
	public static final String PARAM_NAME_TR069_ALARMENABLE = "skyworth.params.tr069.AlarmSwitch";
	
	public static final String PARAM_NAME_TR069_EVENT = "skyworth.params.tr069.event";
	/**/
	public static final String PARAM_NAME_TR069_MAC = "skyworth.params.sys.mac";
	public static final String PARAM_NAME_TR069_SN = "skyworth.params.sys.sn";
	public static final String PARAM_NAME_TR069_SOFTVERSION = "skyworth.params.sys.software_version";
	public static final String PARAM_NAME_TR069_NETTYPE = "skyworth.params.net.connectmode";
	public static final String PARAM_NAME_TR069_PRODUCTTYPE = "skyworth.params.sys.product_type";
	public static final String PARAM_NAME_TR069_AUTHURL = "skyworth.params.itv.homepage";
	public static final String PARAM_NAME_TR069_SERVICE_USERNAME = "skyworth.params.itv.username";
	public static final String PARAM_NAME_TR069_SERVICE_USERPWD = "skyworth.params.itv.password";
	public static final String PARAM_NAME_TR069_HARD_VERSION = "skyworth.params.sys.hardware_version";
	//sunjian stun:
	public static final String PARAM_NAME_TR069_STUN_ENABLE = "skyworth.params.stun.enable";
	public static final String PARAM_NAME_TR069_STUN_ADDR = "skyworth.params.stun.addr";
	public static final String PARAM_NAME_TR069_STUN_PORT = "skyworth.params.stun.port";
	public static final String PARAM_NAME_TR069_STUN_NAME = "skyworth.params.stun.name";
	public static final String PARAM_NAME_TR069_STUN_PWD = "skyworth.params.srun.pwd";
	public static final String PARAM_NAME_TR069_STUN_UDPCONNECTADDR = "skyworth.params.stun.udpaddr";
	/**/
	public static final String PARAM_NAME_TR069_REBOOT_CMD = "skyworth.params.sys.reboot";
	//
	public static final String PARAM_NAME_TR069_PING = "skyworth.params.netmag.ping";
	public static final String PARAM_NAME_TR069_ROUTE = "skyworth.params.netmag.traceroute";
	//{"pppoe_user","skyworth.params.net.pppoeusr"},
	//{"pppoe_pwd","skyworth.params.net.pppoepwd"},
	public static final String PARAM_NAME_TR069_PPPOE_NAME = "skyworth.params.net.pppoeusr";
	public static final String PARAM_NAME_TR069_PPPOE_PWD = "skyworth.params.net.pppoepwd";
	
	public static final String PARAM_NAME_TR069_TIMEZONE = "skyworth.params.sys.timezone";
	
	public static final String PARAM_NAME_TR069_SYSTEM_BUILD_TIME = "skyworth.params.tr069.system_build_time";
	public static final String PARAM_NAME_TR069_BAK_SYSTEM_BUILD_TIME = "skyworth.params.tr069.bak_system_build_time";
	
	
	public static final String PARAM_NAME_TR069_NTP_SERVER_1 = "skyworth.params.sys.ntp_server";
	public static final String PARAM_NAME_TR069_NTP_SERVER_2 = "skyworth.params.sys.ntp_server2";
	
	public static final String PARAM_NAME_TR069_URL_MODEIFY_FLAG = "skyworth.params.tr069.url_modify_flag";
	
	public static final String PARAM_NAME_TR069_ADDRESS_NOTIFICATIONLIMIT = "skyworth.params.tr069.address_notificationlimit";
	
	public static final String PARAM_NAME_TR069_VENDOR_LOG =  "skyworth.params.sys.vendor_log";
	public static final String PARAM_NAME_TR069_PCAP_STATE = "skyworth.params.tr069.tr069_pcap_state";
	public static final String PARAM_NAME_TR069_PCAP_IP_ADDR = "skyworth.params.tr069.tr069_pcap_ip_addr";
	public static final String PARAM_NAME_TR069_PCAP_IP_PORT = "skyworth.params.tr069.tr069_pcap_ip_port";
	public static final String PARAM_NAME_TR069_PCAP_DURATION = "skyworth.params.tr069.tr069_pcap_duration";
	public static final String PARAM_NAME_TR069_PCAP_UPLOAD_URL = "skyworth.params.tr069.tr069_pcap_uploadurl";
	public static final String PARAM_NAME_TR069_PCAP_USERNAME = "skyworth.params.tr069.tr069_pcap_username";
	public static final String PARAM_NAME_TR069_PCAP_PASSWORD = "skyworth.params.tr069.tr069_pcap_password";
	public static final String PARAM_NAME_TR069_PCAP_START = "skyworth.params.tr069.tr069_pcap_start";
	
	public static final String PARAM_NAME_TR069_BANDWIDTH_STATE = "skyworth.params.tr069.tr069_bandwidth_state";	
	public static final String PARAM_NAME_TR069_BANDWIDTH_DOWNLOADURL = "skyworth.params.tr069.tr069_bandwidth_downloadurl";
	public static final String PARAM_NAME_TR069_BANDWIDTH_USERNAME = "skyworth.params.tr069.tr069_bandwidth_username";
	public static final String PARAM_NAME_TR069_BANDWIDTH_PASSWORD = "skyworth.params.tr069.tr069_bandwidth_password";
	public static final String PARAM_NAME_TR069_BANDWIDTH_ERRORCODE = "skyworth.params.tr069.tr069_bandwidth_errorcode";
	public static final String PARAM_NAME_TR069_BANDWIDTH_MAXSPEED ="skyworth.params.tr069.tr069_bandwidth_maxspeed";
	public static final String PARAM_NAME_TR069_BANDWIDTH_AVGSPEED = "skyworth.params.tr069.tr069_bandwidth_avgspeed";
	public static final String PARAM_NAME_TR069_BANDWIDTH_MINSPEED = "skyworth.params.tr069.tr069_bandwidth_minspeed";
	public static final String PARAM_NAME_TR069_BANDWIDTH_START = "skyworth.params.tr069.tr069_bandwidth_start";

	public static final String PARAM_NAME_TR069_SYSLOG_SERVER = "skyworth.params.syslog.server";
	public static final String PARAM_NAME_TR069_SYSLOG_DELAY = "skyworth.params.syslog.delay";
	public static final String PARAM_NAME_TR069_SYSLOG_LOGLEVEL = "skyworth.params.syslog.loglevel";
	public static final String PARAM_NAME_TR069_SYSLOG_PUTTYPE = "skyworth.params.syslog.puttype";
	public static final String PARAM_NAME_TR069_SYSLOG_STARTTIME= "skyworth.params.syslog.starttime";
	public static final String PARAM_NAME_TR069_SYSLOG_START = "skyworth.params.syslog.start";
	
	
	public static final String PARAM_NAME_TR069_PROPERTY_SUPERVISE_CPU_USAGE = "skyworth.params.tr069.cpu_usage";
	public static final String PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_FREE = "skyworth.params.tr069.mem_free";
	public static final String PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_TOTAL = "skyworth.params.tr069.mem_total";

	
	private static TR069ParamsMapping mParamsList = new TR069ParamsMapping(new TR069ParamInfo[]{
		/* system param */
		new TR069ParamInfo(PARAM_NAME_TR069_HEARTBEAT, PARAM_NAME_TR069_HEARTBEAT, "5", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		//
		//new TR069ParamInfo(PARAM_NAME_TR069_ACS, PARAM_NAME_TR069_ACS, "http://cms.js.chinamobile.com:6581/ACS-server/ACS", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		//new TR069ParamInfo(PARAM_NAME_TR069_ACS, PARAM_NAME_TR069_ACS, "http://112.4.1.70:6581/ACS-server/ACS", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_ACS, PARAM_NAME_TR069_ACS, "http://112.4.1.70:7581/ACS-server/ACS", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BAK_ACS, PARAM_NAME_TR069_BAK_ACS, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PLATFORM_URL, PARAM_NAME_TR069_PLATFORM_URL, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PLATFORM_BAK_URL, PARAM_NAME_TR069_PLATFORM_BAK_URL, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_HDC_URL, PARAM_NAME_TR069_HDC_URL, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SILENT_UPGRADE, PARAM_NAME_TR069_SILENT_UPGRADE, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_FORCED_UPGRADE, PARAM_NAME_TR069_FORCED_UPGRADE, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_USER_INSTALL_APPLICATION , PARAM_NAME_TR069_USER_INSTALL_APPLICATION , "2", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_USERNAME, PARAM_NAME_TR069_USERNAME, TR069_USERNAME, TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_PASSWORD, PARAM_NAME_TR069_PASSWORD, TR069_PWD, TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_CPEUSER, PARAM_NAME_TR069_CPEUSER, "cms", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_CPEPASS, PARAM_NAME_TR069_CPEPASS, "cms", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		/**/
		new TR069ParamInfo(PARAM_NAME_TR069_IPADDR, PARAM_NAME_TR069_IPADDR, "172.28.17.143", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_INFOENABLE, PARAM_NAME_TR069_INFOENABLE, "1", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_INFOPERIOD, PARAM_NAME_TR069_INFOPERIOD, "7200", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_IPMASK, PARAM_NAME_TR069_IPMASK, "255.255.255.0", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_IPDNS, PARAM_NAME_TR069_IPDNS, "172.28.0.1", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_IPGW, PARAM_NAME_TR069_IPGW, "172.28.0.1", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_ALARMENABLE, PARAM_NAME_TR069_ALARMENABLE, "0", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		/**/
		new TR069ParamInfo(PARAM_NAME_TR069_EVENT, PARAM_NAME_TR069_EVENT, "0", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		/*MAC SN SOFT_VERSION */
		new TR069ParamInfo(PARAM_NAME_TR069_MAC, PARAM_NAME_TR069_MAC, "5C:C6:D0:AF:A7:07", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SN, PARAM_NAME_TR069_SN, "5C:C6:D0:AF:A7:07", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SOFTVERSION, PARAM_NAME_TR069_SOFTVERSION, "5C:C6:D0:AF:A7:07", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_NETTYPE, PARAM_NAME_TR069_NETTYPE, "dhcp", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PRODUCTTYPE, PARAM_NAME_TR069_PRODUCTTYPE, "A5", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_AUTHURL, PARAM_NAME_TR069_AUTHURL, "A5", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SERVICE_USERNAME, PARAM_NAME_TR069_SERVICE_USERNAME, "A5", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SERVICE_USERPWD, PARAM_NAME_TR069_SERVICE_USERPWD, "A5", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		/*STUN SUNJIAN*/
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_ENABLE, PARAM_NAME_TR069_STUN_ENABLE, "0", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_ADDR, PARAM_NAME_TR069_STUN_ADDR, "192.168.1.20", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_PORT, PARAM_NAME_TR069_STUN_PORT, "37026", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_NAME, PARAM_NAME_TR069_STUN_NAME, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_PWD, PARAM_NAME_TR069_STUN_PWD, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),	
		new TR069ParamInfo(PARAM_NAME_TR069_STUN_UDPCONNECTADDR, PARAM_NAME_TR069_STUN_UDPCONNECTADDR, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		
		new TR069ParamInfo(PARAM_NAME_TR069_REBOOT_CMD, PARAM_NAME_TR069_REBOOT_CMD, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		//
		new TR069ParamInfo(PARAM_NAME_TR069_PING, PARAM_NAME_TR069_PING, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_ROUTE, PARAM_NAME_TR069_ROUTE, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		//
		new TR069ParamInfo(PARAM_NAME_TR069_PPPOE_NAME, PARAM_NAME_TR069_PPPOE_NAME, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PPPOE_PWD, PARAM_NAME_TR069_PPPOE_PWD, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_HARD_VERSION, PARAM_NAME_TR069_HARD_VERSION, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		
		new TR069ParamInfo(PARAM_NAME_TR069_SYSTEM_BUILD_TIME, PARAM_NAME_TR069_SYSTEM_BUILD_TIME, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_BAK_SYSTEM_BUILD_TIME, PARAM_NAME_TR069_BAK_SYSTEM_BUILD_TIME, "", TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_NTP_SERVER_1 , PARAM_NAME_TR069_NTP_SERVER_1 , "" , TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_NTP_SERVER_2 , PARAM_NAME_TR069_NTP_SERVER_2 , "" , TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_URL_MODEIFY_FLAG , PARAM_NAME_TR069_URL_MODEIFY_FLAG ,"15",TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_ADDRESS_NOTIFICATIONLIMIT , PARAM_NAME_TR069_ADDRESS_NOTIFICATIONLIMIT , "10000",TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_VENDOR_LOG , PARAM_NAME_TR069_VENDOR_LOG , "",TR069ParamsType.TR069_PARAM_TYPE_SELF),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_STATE, PARAM_NAME_TR069_PCAP_STATE, "0", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_IP_ADDR, PARAM_NAME_TR069_PCAP_IP_ADDR, "", TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_IP_PORT , PARAM_NAME_TR069_PCAP_IP_PORT , "" , TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_DURATION , PARAM_NAME_TR069_PCAP_DURATION , "" , TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_UPLOAD_URL , PARAM_NAME_TR069_PCAP_UPLOAD_URL ,"",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_USERNAME , PARAM_NAME_TR069_PCAP_USERNAME , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_PASSWORD , PARAM_NAME_TR069_PCAP_PASSWORD , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PCAP_START , PARAM_NAME_TR069_PCAP_START , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
			
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_STATE , PARAM_NAME_TR069_BANDWIDTH_STATE , "0",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_DOWNLOADURL , PARAM_NAME_TR069_BANDWIDTH_DOWNLOADURL , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_USERNAME , PARAM_NAME_TR069_BANDWIDTH_USERNAME , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_PASSWORD , PARAM_NAME_TR069_BANDWIDTH_PASSWORD , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_ERRORCODE , PARAM_NAME_TR069_BANDWIDTH_ERRORCODE , "0",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_MAXSPEED , PARAM_NAME_TR069_BANDWIDTH_MAXSPEED , "128",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_AVGSPEED , PARAM_NAME_TR069_BANDWIDTH_AVGSPEED , "125",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_MINSPEED , PARAM_NAME_TR069_BANDWIDTH_MINSPEED , "120",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_BANDWIDTH_START , PARAM_NAME_TR069_BANDWIDTH_START , "125",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_SERVER , PARAM_NAME_TR069_SYSLOG_SERVER , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_DELAY , PARAM_NAME_TR069_SYSLOG_DELAY , "0",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_LOGLEVEL , PARAM_NAME_TR069_SYSLOG_LOGLEVEL , "0",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_PUTTYPE , PARAM_NAME_TR069_SYSLOG_PUTTYPE , "0",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_STARTTIME , PARAM_NAME_TR069_SYSLOG_STARTTIME , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_SYSLOG_START , PARAM_NAME_TR069_SYSLOG_START , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		
		//supervise
		new TR069ParamInfo(PARAM_NAME_TR069_PROPERTY_SUPERVISE_CPU_USAGE , PARAM_NAME_TR069_PROPERTY_SUPERVISE_CPU_USAGE , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_FREE , PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_FREE , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		new TR069ParamInfo(PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_TOTAL , PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_TOTAL , "",TR069ParamsType.TR069_PARAM_TYPE_MIPT),
		
	});
	//远程抓包
	private String pcap_state = ""; 
	private String pcap_ip_addr = "";
	private String pcap_ip_port = "";
	private String pcap_duration = "";
	private String pcap_upload_url = "";
	private String pcap_username = "";
	private String pcap_password = "";
	
	//
	private String bandwidth_state = "";
	private String bandwidth_downloadurl = "";
	private String bandwidth_username = "";
	private String bandwidth_password = "";
	private String bandwidth_errorcode = "0";
	private String bandwidth_maxspeed = "";
	private String bandwidth_avgspeed = "";
	private String bandwidth_minspeed ="";
	
	//
	private String syslog_server = "";
	private String syslog_delay = "";
	private String syslog_loglevel = "";
	private String syslog_puttype = "";
	private String syslog_starttime = "";
	
	//property supervise
	private String property_supervise_cpu_usage = "0";
	private String property_supervise_mem_total = "1024";
	private String property_supervise_mem_free = "1024";

	
	private Context mContext = null;
	
	private TR069Params(){
		mContext = ContextUtil.getInstance();
		
		//modify by lijingchao
		/*
		Intent intent = new Intent(mContext, NewDownloadService.class);
		NewDownloadService.startService(mContext);
		*/
		
		Intent intent = new Intent("com.mipt.tr069.start.downloadservice");
		Log.d("TR069Params", "mContext.sendBroadcast -->com.mipt.tr069.start.downloadservice");
		mContext.sendBroadcast(intent);
		
		initParams();
		Log.d("TR069Params", "nativeInit()");
		nativeInit();
	}
	
	private void initParams() {
		mSharedPreferences = mContext.getSharedPreferences(PARAM_TAG, mContext.MODE_PRIVATE);
		syslog_server = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_SERVER, "");
		syslog_delay = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_DELAY, "");
		syslog_loglevel = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_LOGLEVEL, "");
		syslog_puttype = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_PUTTYPE, "");
		syslog_starttime = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_STARTTIME, "");
	}

	public static TR069Params getInstance(){
		if(mInstance == null){
			mInstance = new TR069Params();
		}
		
		return mInstance;
	}
	
	private static final class TR069ParamInfo{
		private String sParamName = "";
		private String sParamSystemName = "";
		private String sDefaultValue = "";
		private TR069ParamsType eParamType = TR069ParamsType.TR069_PARAM_TYPE_MIPT;
		
		public TR069ParamInfo(String param_name, String system_name, String default_value, TR069ParamsType type){
			sParamName = param_name;
			sParamSystemName = system_name;
			sDefaultValue = default_value;
			eParamType = type;
		}
		
		public String ParamName(){
			return sParamName;
		}
		
		public String ParamSystemName(){
			return sParamSystemName;
		}
		
		public String DefaultValue(){
			return sDefaultValue;
		}
		
		public TR069ParamsType Type(){
			return eParamType;
		}
	}
	
	
	public String getParam(String key){
		
		SharedPreferences settings = mContext.getSharedPreferences(PARAM_TAG, 0);
		
		String ret = "";
		
		if(mParamsList.containsKey(key)){
			TR069ParamInfo info = mParamsList.get(key);
			
			switch(info.Type()){
				case TR069_PARAM_TYPE_MIPT:
					Log.d("getParam TR069_PARAM_TYPE_MIPT", "key:" + key );
					//ret = settings.getString(key, info.DefaultValue());
					try{
						ret = getParamFromMIPT(key);
					}catch (Exception e) {
						// TODO: handle exception
						e.printStackTrace();
						ret = "";
					}
					break;
				case TR069_PARAM_TYPE_SELF:
					Log.d("getParam TR069_PARAM_TYPE_SELF", "key:" + key );
					if(key.equalsIgnoreCase(PARAM_NAME_TR069_MAC))
					{
						/*ret = getIpAddr();*/
					}
					else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSTEM_BUILD_TIME))
					{
						//add by liingchao 
						//获取系统编译时间
						SimpleDateFormat sdf = new SimpleDateFormat("E MMM dd HH:mm:ss 'CST' yyyy",Locale.US);
						String version_compile_time = sdf.format(new Date(Build.TIME));
						ret = version_compile_time;
					}
					else
					{
						ret = settings.getString(key, info.DefaultValue());
					}					
					break;
			}
			
			if(ret == null){
				if(info.DefaultValue() != null){
					ret = info.DefaultValue();
				}else{
					ret = "";
				}				
			}
		}else{
			if(settings.contains(key)){
				ret = settings.getString(key, "");
			}
		}
		
    	return ret;
	}
	
	public void setParam(String key, String value) 
	{
		Log.d("sktest", "key:" + key + ", value:" + value);
		SharedPreferences settings = mContext.getSharedPreferences(PARAM_TAG, 0);
    	
    	if(mParamsList.containsKey(key)){
			TR069ParamInfo info = mParamsList.get(key);
			
			switch(info.Type()){
				case TR069_PARAM_TYPE_MIPT:
					Log.d("setParam TR069_PARAM_TYPE_MIPT", "key:" + key );
					setParamFromMIPT(key ,value);
					break;
				case TR069_PARAM_TYPE_SELF:
					Log.d("setParam TR069_PARAM_TYPE_SELF", "key:" + key );
					if(key.equalsIgnoreCase(PARAM_NAME_TR069_VENDOR_LOG))
					{
						setVerdorLog(value);
					}
					else
					{
						settings.edit().putString(key, value).commit();
					}
					break;
			}
		}else{
			settings.edit().putString(key, value).commit();
		}
	}
	
	private static final class TR069ParamsMapping extends HashMap<String, TR069ParamInfo>{
		/**
		 * serial 版本号,根据类名,接口名及成员方法自动生成
		 */
		private static final long serialVersionUID = 1222090392930428266L;

		public TR069ParamsMapping(TR069ParamInfo[] info){
			for(int i = 0; i < info.length; i += 1){
				this.put(info[i].ParamName(), info[i]);
			}
		}
		
		public void add(TR069ParamInfo info){
			this.put(info.ParamName(), info);
		}
	}
	
	//for native test
	public void doNativeTest(){
		nativeTest();
	}
	public void doNativeStop(){
		nativeStop();
	}
	
	//call by jni
	private void sendFactoryReset(){
		Log.d("sktest", "call sendFactoryReset! --> android.intent.action.MASTER_CLEAR_MIPT");
		//Intent intent = new Intent("android.permission.MASTER_CLEAR");
		//mContext.sendBroadcast(intent);
		//mContext.sendBroadcastAsUser(intent, new UserHandle(UserHandle.USER_CURRENT));
		//mContext.sendBroadcastAsUser(intent, new UserHandle(UserHandle.));
		 mContext.sendBroadcast(new Intent("android.intent.action.MASTER_CLEAR_MIPT"));
	}
	
	//call by jni
	private void sendUpgrade(String path){
		//Log.d("sendUpgrade", "call sendFactoryReset!");
		Log.d("sendUpgrade", "call sendUpgrade, path:" + path);
		//Intent intent = new Intent("android.action.skyworth.sys.UPDATE");
		//
		
		//add by lijingchao
		//Toast toast = Toast.makeText(mContext , "即将升级......" , 20);
		//toast.show();
		
		Intent intent = new Intent("android.action.ics.yueshiting.ota.update");
		//Intent intent = new Intent("android.action.ics.mipt.ota.update");
		intent.putExtra("url", path);
		//mContext.sendBroadcastAsUser(intent);
		//mContext.sendBroadcastAsUser(intent, new UserHandle(UserHandle.USER_CURRENT));
		//mContext.sendBroadcastAsUser(intent, new UserHandle(UserHandle.ALL));	
		//modify by lijingchao
		//mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
		mContext.sendBroadcast(intent);
		
	}
	
	public void setParamFromMIPT(String key, String value) 
	{
		String str = "";
		
		if(key.equalsIgnoreCase(PARAM_NAME_TR069_REBOOT_CMD))				
		{					
			Log.d("setParam TR069_PARAM_TYPE_MIPT reboot begin...", "key:" + key );
			PowerManager pm = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);
			pm.reboot("netmang");
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_START))
		{
			Log.d("setParam TR069_PARAM_TYPE_MIPT start PacketCapture begin...", "key:" + key );
			startPacketCapture(value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NETTYPE))		
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_NETTYPE value =" + value);
			//write_nettype(value); // because A5_framework.jar was deleted, so it was modified in 2014-1-24
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SERVICE_USERPWD))
		{			
			 //str = CMAccount.getValue(CMAccount.PASSWORD, mContext.getContentResolver());				  
			//Added by James, // TODO Need to add setServicePw() function.
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_SERVICE_USERPWD value = "+ value);
			
			str = Utils.getServicePassword(mContext);
			Log.d(PARAM_TAG,"Utils.getServicePassword()= " + str);
			if(!TextUtils.isEmpty(str)) {
				//Utils.setServicePassword(mContext , value);
				return ;
			}
			
			str = Utils.getAccountPassword(mContext);
			Log.d(PARAM_TAG,"Utils.getAccountPassword()= " + str);
			if(!TextUtils.isEmpty(str)) {
				//Utils.setAccountPassword(mContext , value);
				return ;
			}
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SERVICE_USERNAME))
		{				
			//str = CMAccount.getValue(CMAccount.ACCOUNT_ID, mContext.getContentResolver());
			//Added by James, // TODO Need to add setServiceAccount() function.
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_SERVICE_USERNAME value = "+ value);
			ContentResolver contentResolver = mContext.getContentResolver();
			ContentValues cv = new ContentValues();
			cv.put("name", "username");
			cv.put("value", value);
			Uri uri = Uri.parse("content://" + AUTHORITIES + "/" + TNAME);
			contentResolver.update(uri, cv, null, null);
		}	
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_USER_INSTALL_APPLICATION))
		{		
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_USER_INSTALL_APPLICATION value = "+value);
			Utils.setUserIntallApplication(value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PLATFORM_URL))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_PLATFORM_URL value = "+value);
			Utils.setPlatformUrl(mContext, value);	
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PLATFORM_BAK_URL))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_PLATFORM_BAK_URL value = "+value);
			Utils.setPlatformUrlBackup(mContext, value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_HDC_URL))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_HDC_URL value = "+value);
			Utils.setHdcUrl(mContext, value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SILENT_UPGRADE))
		{
			int silentUpgrade = 0;
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_SILENT_UPGRADE value = "+value);
			silentUpgrade = Integer.parseInt(value);
			Utils.setSilentUpgrade(silentUpgrade);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_FORCED_UPGRADE))
		{
			int forcedUpgrade = 0;
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_FORCED_UPGRADE value = "+value);
			forcedUpgrade = Integer.parseInt(value);
			if(0 != forcedUpgrade){
				forcedUpgrade = 1;
				Utils.setForcedUpgrade(forcedUpgrade);
			}else{
				Utils.setForcedUpgrade(forcedUpgrade);
			}
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_ACS))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_ACS value = "+value);
			Utils.setUrl(mContext, value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BAK_ACS))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_BAK_ACS value = "+value);
			Utils.setUrlBackup(mContext, value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NTP_SERVER_1))	
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_NTP_SERVER_1 value = "+value);
			Utils.setNtpServer(value);
			
		}	
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NTP_SERVER_2))
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_NTP_SERVER_2 value = "+value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPMASK)) 
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_IPMASK value = "+value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPDNS)) 
		{
			EthernetManager em = (EthernetManager) mContext.getSystemService(Context.ETH_SERVICE);
			EthernetDevInfo info = em.getSavedEthConfig();
			info.setDnsAddr(value);
			
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_IPDNS value = "+value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPGW)) 
		{
			Log.d("setParam setParamFromMIPT","set PARAM_NAME_TR069_IPGW value = "+value);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_STATE))
		{
			pcap_state = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_IP_ADDR))
		{
			pcap_ip_addr = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_IP_PORT))
		{
			pcap_ip_port = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_DURATION))
		{
			pcap_duration = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_UPLOAD_URL))
		{
			pcap_upload_url = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_USERNAME))
		{
			Log.d(PARAM_TAG,"set pcap_username()=" + value);
			pcap_username = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_PASSWORD))
		{
			pcap_password = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_STATE))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_STATE()=" + value);
			bandwidth_state = value;
			Message msg = new Message();
			msg.what = BAND_WIDTH_START_SPEED_TEST;
			mHandler.sendMessage(msg);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_DOWNLOADURL))
		{
			bandwidth_downloadurl = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_USERNAME))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_USERNAME()=" + value);
			bandwidth_username = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_PASSWORD))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_PASSWORD=" + value);
			bandwidth_password = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_ERRORCODE))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_ERRORCODE=" + value);
			bandwidth_errorcode = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_MAXSPEED))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_MAXSPEED=" + value);
			bandwidth_maxspeed = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_AVGSPEED))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_AVGSPEED=" + value);
			bandwidth_avgspeed = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_MINSPEED))
		{
			Log.d(PARAM_TAG,"set PARAM_NAME_TR069_BANDWIDTH_MINSPEED=" + value);
			bandwidth_minspeed = value;
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_START))
		{
			Log.d(PARAM_TAG,"start PARAM_NAME_TR069_BANDWIDTH_START");
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_SERVER))
		{
			syslog_server = value;
			Editor editor = mSharedPreferences.edit();
			editor.putString(PARAM_NAME_TR069_SYSLOG_SERVER, syslog_server);
			editor.commit();
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_DELAY))
		{
			syslog_delay = value;
			Editor editor = mSharedPreferences.edit();
			editor.putString(PARAM_NAME_TR069_SYSLOG_DELAY, syslog_delay);
			editor.commit();
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_LOGLEVEL))
		{
			syslog_loglevel = value;
			Editor editor = mSharedPreferences.edit();
			editor.putString(PARAM_NAME_TR069_SYSLOG_LOGLEVEL, syslog_loglevel);
			editor.commit();
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_PUTTYPE))
		{
			syslog_puttype = value;
			Editor editor = mSharedPreferences.edit();
			editor.putString(PARAM_NAME_TR069_SYSLOG_PUTTYPE, syslog_puttype);
			editor.commit();
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_STARTTIME))
		{
			syslog_starttime = value;
			Editor editor = mSharedPreferences.edit();
			editor.putString(PARAM_NAME_TR069_SYSLOG_STARTTIME, syslog_starttime);
			editor.commit();
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_START))
		{
			startUploadSystemLog(null);
		}
		else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PROPERTY_SUPERVISE_CPU_USAGE))
		{
			property_supervise_cpu_usage = value;
		}
		else
		{
			Log.d(PARAM_TAG,"setParamFromMIPT() key not find!");
		}
	}
	
	public <bandwidth_state> String getParamFromMIPT(String key)
	{
			String str = "";
			int deviceType = Utils.deviceType(mContext);
			/*ok*/
			if(key.equalsIgnoreCase(PARAM_NAME_TR069_SOFTVERSION)){
				//str = Utils.getSoftwareVersion(deviceType);
				//modify by lijingchao
				str = Utils.getSoftVersion();
				Uri uri = Uri.parse("content://" + AUTHORITIES + "/" + TNAME + "/" + SOFTWAREVERSION);
				String software_version = getValueFromUri(uri);
				str = software_version;
				Log.d(PARAM_TAG,"Utils.getSoftVersion()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_MAC))
			{
				//str = Utils.getMac();
				//str = "5C:C6:D0:AF:A7:08";
				//modify by lijingchao
				str = Utils.getMac(mContext);
				Log.d(PARAM_TAG,"Utils.getMac()= "+str);
			}			
			/*sn:ok*/
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SN))
			{
				//str = Utils.getSerialNumber(deviceType);
				//modify by lijingchao
				str = Utils.getSN();
				Log.d(PARAM_TAG,"Utils.getSN()= "+str);
			}	
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NETTYPE))
			{
				//str="dhcp"; /*lan、dhcp、pppoe*/
				//str = readnettype();
				//modify by lijingchao
				//str = Utils.getNetType(mContext);				
				str = Utils.getIPState(mContext);
				Log.d(PARAM_TAG,"Utils.getNetType()= "+str);
			}	
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PRODUCTTYPE))
			{
				//str = Utils.getdeviceName(mContext);
				//modify by lijingchao
				//str = "A3";
				str = Utils.getDeviceModle();
				Log.d(PARAM_TAG,"Utils.getDeviceModle()= "+str);
			}	
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_AUTHURL))
			{
				/*str = Utils.getdeviceName(mContext);*/				
				//str = (new GetAuthUrl()).getUrl(mContext);
				//modify by lijingchao
//				str = Utils.getServiceAddress(mContext);
				str = "http://223.99.188.49:33600";
				Log.d(PARAM_TAG,"Utils.getServiceAddress()= "+str);				
			}	
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SERVICE_USERPWD))
			{			
				 //str = CMAccount.getValue(CMAccount.PASSWORD, mContext.getContentResolver());				  
				//modify by lijingchao
//				str = Utils.getServicePassword(mContext);
				Uri uri = Uri.parse("content://" + AUTHORITIES + "/" + TNAME + "/");
				String[] projection = new String[]{"value"};
				String selection = "name=?";
				String[] selectionArgs = new String[]{"IPTV_USER_PASSWORD"};
				String password = getValueFromUri(uri, projection, selection, selectionArgs);
				str = password;
				Log.d(PARAM_TAG,"Utils.getServicePassword()= " + str);
				if(TextUtils.isEmpty(str)) {
					str = Utils.getAccountPassword(mContext);
					Log.d(PARAM_TAG,"Utils.getAccountPassword()= " + str);
					if(TextUtils.isEmpty(str)) {
						Log.d(PARAM_TAG,"str is null or str.length less than zero");
						return "";
					}
				}
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SERVICE_USERNAME))
			{				
				//str = CMAccount.getValue(CMAccount.ACCOUNT_ID, mContext.getContentResolver());
				//modify by lijingchao			
//				str = Utils.getMobilePhoneNumber(mContext);
				Uri uri = Uri.parse("content://" + AUTHORITIES + "/" + TNAME + "/" + USERNAME);
				String username = getValueFromUri(uri);
				str = username;
				Log.d(PARAM_TAG,"Utils.getMobilePhoneNumber()= " + str);
				if(TextUtils.isEmpty(str)) {
					str = Utils.getAccount(mContext);
					Log.d(PARAM_TAG,"Utils.getAccount()= " + str);
					if(TextUtils.isEmpty(str)) {
						Log.d(PARAM_TAG,"str is null or str.length less than zero");
						return "";
					}
				}
			}				
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PPPOE_NAME))			 
			{
				//modified by James in 2013-12-23 start
				str = Utils.getPPPoeUserName(mContext);	
				Log.d(PARAM_TAG,"Utils.getPPPoeUserName()= "+str);
				//modified by James in 2013-12-23 end.
				//str = "pppoe123";
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PPPOE_PWD))
			{
				//modified by James in 2013-12-23 start
				str = Utils.getPPPoePassword(mContext);	
				Log.d(PARAM_TAG,"Utils.getPPPoePassword()= "+str);
				//modified by James in 2013-12-23 end.
				//modify by lijingchao
				//str = "123";
			}//
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_HARD_VERSION))
			{
				//str = Utils.getHardwareVersion(deviceType);				
				//modify by lijingchao
				str = Utils.getHardVersion();
				Log.d(PARAM_TAG,"Utils.getHardVersion()= "+str);
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPADDR))
			{
				//str =MiptNetInfo.getIpAddr() ;// Utils.getHardwareVersion(deviceType);
				//modify by lijingchao
				str = Utils.getIpAddress(mContext);
				Log.d(PARAM_TAG,"Utils.getIpAddress()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_USER_INSTALL_APPLICATION))
			{
				int user_install_flag = 2;
				user_install_flag = Utils.getUserIntallApplication();
				str = Integer.toString(user_install_flag);
				Log.d(PARAM_TAG,"Utils.getUserIntallApplication()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PLATFORM_URL))
			{
				str = Utils.getPlatformUrl(mContext);
				Log.d(PARAM_TAG,"Utils.getPlatformUrl()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PLATFORM_BAK_URL))
			{
				str = Utils.getPlatformUrlBackup(mContext);
				Log.d(PARAM_TAG,"Utils.getPlatformUrlBackup()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_HDC_URL))
			{
				str = Utils.getHdcUrl(mContext);
				Log.d(PARAM_TAG,"Utils.getHdcUrl()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SILENT_UPGRADE))
			{
				int silentUpgrade = 0;
				silentUpgrade = Utils.getSilentUpgrade();
				str = Integer.toString(silentUpgrade);
				Log.d(PARAM_TAG,"Utils.getSilentUpgrade()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_FORCED_UPGRADE))
			{
				int forcedUpgrade = 0;
				forcedUpgrade = Utils.getForcedUpgrade();
				str = Integer.toString(forcedUpgrade);
				Log.d(PARAM_TAG,"Utils.getForcedUpgrade()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_ACS))
			{
				str = Utils.getUrl(mContext);
				if (!str.contains("http://"))
					str = TR069_ACSURL;
				Log.d(PARAM_TAG,"Utils.getUrl()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BAK_ACS))
			{
				str = Utils.getUrlBackup(mContext);
				Log.d(PARAM_TAG,"Utils.getUrlBackup()= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_USERNAME))
			{
				str = Utils.getAcsUserName(mContext);
				str = TR069_USERNAME;
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_USERNAME USER_NAME= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PASSWORD))
			{
				str = Utils.getAcsPassword(mContext);
				str = TR069_PWD;
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_PASSWORD PASSWORD= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NTP_SERVER_1))	
			{
				str = Utils.getNtpServer(mContext);
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_NTP_SERVER_1 NTPSERVER= "+str);
			}	
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_NTP_SERVER_2))
			{
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPMASK)) 
			{
//				str = Utils.getMask(mContext);
				EthernetManager em = (EthernetManager) mContext.getSystemService(Context.ETH_SERVICE);
				EthernetDevInfo info = em.getSavedEthConfig();
				str = info.getNetMask();
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_IPMASK IPMASK= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPDNS)) 
			{
//				str = Utils.getDnsServer(mContext);
				EthernetManager em = (EthernetManager) mContext.getSystemService(Context.ETH_SERVICE);
				EthernetDevInfo info = em.getSavedEthConfig();
				str = info.getDnsAddr();
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_IPDNS IPDNS= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_IPGW)) 
			{
//				str = Utils.getGateWay(mContext);
				EthernetManager em = (EthernetManager) mContext.getSystemService(Context.ETH_SERVICE);
				EthernetDevInfo info = em.getSavedEthConfig();
				str = info.getRouteAddr();
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_IPGW IPGW= "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_STATE))
			{
				str = pcap_state;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_IP_ADDR))
			{
				str = pcap_ip_addr;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_IP_PORT))
			{
				str = pcap_ip_port;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_DURATION))
			{
				str = pcap_duration;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_UPLOAD_URL))
			{
				str = pcap_upload_url;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_USERNAME))
			{
				str = pcap_username;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PCAP_PASSWORD))
			{
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_PCAP_PASSWORD = "+str);
				str = pcap_password;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_STATE))
			{
				str = bandwidth_state;
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_BANDWIDTH_STATE = "+str);
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_DOWNLOADURL))
			{
				str = bandwidth_downloadurl ;
			
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_USERNAME))
			{
				str = bandwidth_username ;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_PASSWORD))
			{
				str = bandwidth_password ;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_ERRORCODE))
			{
				str = bandwidth_errorcode;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_MAXSPEED))
			{
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_BANDWIDTH_MAXSPEED= "+bandwidth_maxspeed);
				str = bandwidth_maxspeed;
				  
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_AVGSPEED))
			{
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_BANDWIDTH_AVGSPEED = "+str);
				str = bandwidth_avgspeed ;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_MINSPEED))
			{
				 str = bandwidth_minspeed ;
	
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_BANDWIDTH_START))
			{
				 
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_SERVER))
			{
				syslog_server = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_SERVER, "");
				str = syslog_server ;
		
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_DELAY))
			{
				syslog_delay = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_DELAY, "");
				str = syslog_delay ;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_LOGLEVEL))
			{
				syslog_loglevel = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_LOGLEVEL, "");
				str = syslog_loglevel;
				
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_PUTTYPE))
			{
				syslog_puttype = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_PUTTYPE, "");
				str = syslog_puttype;
			
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_STARTTIME))
			{
				syslog_starttime = mSharedPreferences.getString(PARAM_NAME_TR069_SYSLOG_STARTTIME, "");
				str = syslog_starttime ;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_SYSLOG_START))
			{
			}
			
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PROPERTY_SUPERVISE_CPU_USAGE))
			{
				new CpuInfo(mContext, mHandler).getCpuInfos();
				str = property_supervise_cpu_usage;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_FREE))
			{
				new MemoryInfo(mContext, mHandler).getMeminfo();
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_FREE = " + property_supervise_mem_free);
				str = property_supervise_mem_free;
			}
			else if(key.equalsIgnoreCase(PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_TOTAL))
			{
				Log.d(PARAM_TAG,"PARAM_NAME_TR069_PROPERTY_SUPERVISE_MEM_TOTAL = " + property_supervise_mem_total);
				str = property_supervise_mem_total;
			}
			else
			{
				Log.d(PARAM_TAG,"getParamFromMIPT() key not find!");
			}
			return str;
	}
	
	private  String PPPOE_INFO_SAVE_FILE = "/data/system/pap-secrets";
	private String mLoginFormat = "\"%s\" * \"%s\"";

	public boolean writeLoginInfo(String username, String password) {
		File file = new File(PPPOE_INFO_SAVE_FILE);
		String loginInfo = String.format(mLoginFormat, username, password);
		try {
			BufferedOutputStream out = new BufferedOutputStream(
					new FileOutputStream(file));
			out.write(loginInfo.getBytes(), 0, loginInfo.length());			
			out.flush();
			out.close();
		} catch (IOException e) {
			//Log.w(TAG, "Write " + PPPOE_INFO_SAVE_FILE + " failed! " + e);
			return false;
		}
		return true;
	}
	public String readLoginInfo() {
		File file = new File(PPPOE_INFO_SAVE_FILE);
		char[] buf = new char[1024];
		String loginInfo = new String();
		FileReader in;
		try {
			in = new FileReader(file);
			BufferedReader bufferedreader = new BufferedReader(in);
			loginInfo = bufferedreader.readLine();
			/*
			if (DEBUG) {
				Log.d(TAG, "read form " + PPPOE_INFO_SAVE_FILE
						+ " login info = " + loginInfo);
			}
			*/
			bufferedreader.close();
			in.close();
		} catch (IOException e) {
			//Log.w(TAG, "Read " + PPPOE_INFO_SAVE_FILE + " failed! " + e);
		}
		return loginInfo;
	}
	
	private void setVerdorLog(String value)
	{
		String str = null;
		int max = 100;
    	final Context context = ContextUtil.getAppContext();
    	final String filename = "log.txt";
		
		 Log.d(PARAM_TAG,"setVerdorLog enter ............");
         try {
             String cmd = "logcat -v time";
             
             Process p = Runtime.getRuntime().exec(cmd);
            
             BufferedReader bufferReader = new BufferedReader(new InputStreamReader(p.getInputStream()));
    
             
			FileOutputStream fos = context.openFileOutput(filename ,Context.MODE_WORLD_READABLE | Context.MODE_WORLD_WRITEABLE);    
			  
			int i = 0;
			while(i++ < max){
			if((str = bufferReader.readLine()) != null) {
			       Log.d(PARAM_TAG, "read = " + str);
			       	byte[] buffer = str.getBytes();
						fos.write(buffer);
					    //使换行符转为字节数组  
					byte []newLine="\r\n".getBytes();  
					//写入换行  
					fos.write(newLine);

			      }
			}
			fos.close();   
			  
			p.destroy();
              
            int status = p.waitFor();// 只有status=0时，正常
             
            Log.d(PARAM_TAG ,"setVerdorLog waitFor ............status = " + status + ",cmd="+cmd);
            
            bufferReader.close();
         } catch (IOException e) {
             e.printStackTrace();
         } catch (InterruptedException e) {
             e.printStackTrace();
         }
         Log.d(PARAM_TAG,"setVerdorLog  exit ............");
	}
	
	private String getValueFromUri(Uri uri) {
		String value = null;
		Cursor cursor = mContext.getContentResolver().query(uri, null, null, null, null);
		if (cursor != null) {
			while (cursor.moveToNext()) {
				value = cursor.getString(cursor.getColumnIndex("value"));
			}
			cursor.close();
		}
		return value;
	}
	
	private String getValueFromUri(Uri uri, String[] projection, String selection, String[] selectionArgs) {
		String value = null;
		Cursor cursor = mContext.getContentResolver().query(uri, projection, selection, selectionArgs, null);
		if (cursor != null) {
			while (cursor.moveToNext()) {
				value = cursor.getString(cursor.getColumnIndex("value"));
			}
			cursor.close();
		}
		return value;
	}
	private int startPacketCapture(String cmd)
	{
		SkSFTP sf = new SkSFTP(); 
		if(pcap_upload_url.isEmpty())
		{
			Log.d(PARAM_TAG,"pcap_upload_url is null");
			return -1;
		}
	
		try {
			URI uri = new URI(pcap_upload_url);
			String protocol;
			String host = null;
			int port = 22;
			String username = pcap_username;
			String password = pcap_password;
			String directory = "/";
			String uploadFile = "/tcpdump.pcap";
			Log.d(PARAM_TAG,"protocol="+uri.getScheme());
			Log.d(PARAM_TAG,"host="+uri.getHost());  
			Log.d(PARAM_TAG,"port="+uri.getPort());
			Log.d(PARAM_TAG,"path="+uri.getPath());
			host=uri.getHost();  
			if(host.isEmpty())
			{
				Log.d(PARAM_TAG,"pcap_upload_url host is null");
				return -1;
			}
			
			directory=uri.getPath();
			if(directory.isEmpty())
			{
				directory = "/";
			}
			
			protocol = uri.getScheme();
			if(protocol.equalsIgnoreCase("sftp"))
			{
				port= uri.getPort();
				if(0 == port)
				{
					port = 22;
				}
				ChannelSftp sftp = sf.connect(host, port, username, password);
				sf.upload(directory, uploadFile, sftp);
				sf.disconnect(sftp);
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return 0;
	}
	private int startBandWidthTest(String cmd)
	{
		new Thread(){
			@Override
			public void run() {
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				new SpeedTest(mContext, mHandler).measureSpeed(bandwidth_downloadurl);
			}
		}.start();
		return 0;
	}
	
	private int startUploadSystemLog(String cmd)
	{
		Log.d(PARAM_TAG,"startUploadSystemLog");
		String mac = Utils.getMac(mContext);
		Uri uri = Uri.parse("content://" + AUTHORITIES + "/" + TNAME + "/" + SOFTWAREVERSION);
		String software_version = getValueFromUri(uri);
		
		if (mUpLoadLog == null || mUpLoadLog.getState() == UpLoadLog.UPLOAD_THREAD_STOPED){
			mUpLoadLog = new UpLoadLog(mac, software_version);
			mUpLoadLog.beginToSendUDPLog(Long.parseLong(syslog_delay), syslog_server);
		}
		else
			mUpLoadLog.setEndTime(Long.parseLong(syslog_delay));
		
		return 0;
	}
	
	private native void nativeInit();
	private native void nativeTest();
	private native void nativeStop();
	public native void setConfigPath(String path);//
	public native void nativeBandWidthTestResponse();
	public native void nativeping(String pingvailue);//nativeping
	public native void nativeTraceroute(String traceroute_result);
	
}



