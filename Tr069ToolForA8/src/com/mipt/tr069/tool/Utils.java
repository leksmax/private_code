package com.mipt.tr069.tool;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.app.DevInfoManager;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.DhcpInfo;
import android.net.NetworkInfo;
import android.net.NetworkUtils;
import android.net.Uri;
import android.net.ethernet.EthernetDevInfo;
import android.net.ethernet.EthernetManager;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Environment;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

public class Utils {
	public static String TAG = "Tr069ToolForA8_Utils";
	public static String REGISTER = "register broadcast";
	public static final String USER_AGENT = "PhoneUpdateOTA";
	public static final String TAG_SUFFIX = ".part";
	public static final String GROUP_UPDATE = "ottupdate";
	public static final String PATH = "path";
	public static final String MD5 = "md5";
	public static final String VERSION = "version";
	public static final String NEW = "new";
	public static final String URI = "uri";
	public static final String AUTHORITY = "com.mipt.ott.yueshiting.update";// com.mipt.ott.update
	/**
	 * The content:// style URL for the table
	 */
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY
			+ "/conf");
	/**
	 * The config group name
	 * <P>
	 * Type: TEXT
	 * </P>
	 */
	public static final String GROUP_NAME = "confgroup";

	/**
	 * The config item name of the settings value.
	 * <P>
	 * Type: TEXT
	 * </P>
	 */
	public static final String NAME = "name";

	/**
	 * The contents of the setting value
	 * <P>
	 * Type: TEXT
	 * </P>
	 */
	public static final String VALUE = "value";
	public static final Uri UPDATE_URI = Uri.parse("content://"
			+ Utils.AUTHORITY + "/update");

	private static final String[] mValueProjection = { "_id", VALUE };
	private static final String PROPERTY_SEQ_ID = "persist.sys.hwconfig.seq_id";
	private static final String PROPERTY_STB_ID = "persist.sys.hwconfig.stb_id";
	private static final String PROPERTY_SOFTWAR_ID = "persist.sys.hwconfig.soft_ver";
	private static final String PROPERTY_HW_VER = "persist.sys.hwconfig.hw_ver";
	private static final String A4_INFO_FILE_DEVICE_ID = "/private/.id.txt";
	private static String HOST_SKYWORTH = "ro.stb.user.url";
	public static final String UPDATEZIP = "update.zip";

	public static final int DEVICE_MIPT_IKAN_A6 = 2;
	public static final int DEVICE_MIPT_IKAN_A4 = 3;
	public static final int DEVICE_MIPT_BETV_U6 = 4;
	public static final int DEVICE_SKYWORTH_NEXT_PANDORA = 5;
	public static final int DEVICE_MIPT_IKAN_TEST = 6;
	public static final int DEVICE_MIPT_BETV_TEST = 7;
	public static final int DEVICE_SKYWORTH_HSM1 = 8;
	public static final int DEVICE_MIPT_IKAN_A8 = 9;
	public static final int DEVICE_BST_SNT_B10 = 10;
	public static final int DEVICE_SKYWORTH_HAO = 11;
	public static final int DEVICE_MIPT_IKAN_A5 = 12;
	public static final int DEVICE_SKYWORTH_NORMAL = 13;
	public static final int DEVICE_MIPT_IKAN_R6 = 14;
	public static final int DEBUG_FLAG = 1;
	public static final byte STATUS_PAUSED = 2;
	public static final int NEW_ITEM_ID = -1;
	public static final int ERROR_UNKNOWN = 0;
	public static final int ERROR_NETWORK = 1;
	public static final int ERROR_NO_SPACE = 2;
	public static final int ERROR_SDCARD_REMOVED = 3;

	public static final String DEBUG_VENDOR = "MiPT";
	public static final String DEBUG_MODEL = "A6";
	public static final String DEBUG_VER = "v1";
	public static final boolean DBG = true;

	public static final byte STATUS_FINISHED = 3;

	private static Utils mInstance = null;
//	private static Context mContext = null;
	private static String sDeviceId = null;
	
	//Added variable by James in 2014-1-14 start.
	private static String SHAREPREFERENCED_KEY = "JSMobile_OTT";
//	private static boolean isFirstReadUserName = true;
//	private static boolean isFirstReadPassword = true;
//	private static String USER_NAME = "userName";
//	private static String USER_PASSWORD = "password";
	//Added variable by James in 2014-1-14 end.
	
	//Added variable by tianhuaxin in 2014-1-16 start.
	private static final String PROPERTY_SET = "persist.sys.tr.serviceinfo";
	private static final String SILENT_UPGRADE = "persist.sys.tr.SilentUpgrade";
	private static final String FORCED_UPGRADE = "persist.sys.tr.ForcedUpgrade";
	private static final String NTP_SERVER = "persist.sys.tr.ntpserver";
	private static final String PROPERTY_VALUE_0 = "0";
	private static final String PROPERTY_VALUE_1 = "1";
	//private static final String PROPERTY_VALUE_2 = "2";
	//Added variable by tianhuaxin in 2014-1-16 end.

	/*public Utils() {
		mContext = ContextUtil.getInstance();
	}*/

	public static final String getOem() {
		String oem = getOttDeviceInfo("mipt.ott.oem");
		Logger.getLogger().i("getOem()--> " + oem);
		return oem;
	}

	private static final String getOttDeviceInfo(String str) {
		String id = null;

		try {
			Class<?> c = Class.forName("android.os.SystemProperties");
			Method m = c.getMethod("get", new Class[] { String.class });
			id = (String) m.invoke(c, new Object[] { str });
		} catch (ClassNotFoundException cnfe) {
			Log.e(Logger.TAG, "Error", cnfe);
		} catch (NoSuchMethodException nsme) {
			Log.e(Logger.TAG, "Error", nsme);
		} catch (SecurityException se) {
			Log.e(Logger.TAG, "Error", se);
		} catch (IllegalAccessException iae) {
			Log.e(Logger.TAG, "Error", iae);
		} catch (IllegalArgumentException iarge) {
			Log.e(Logger.TAG, "Error", iarge);
		} catch (InvocationTargetException ite) {
			Log.e(Logger.TAG, "Error", ite);
		} catch (ClassCastException cce) {
			Log.e(Logger.TAG, "Error", cce);
		} catch (Throwable th) {
			Log.e(Logger.TAG, "Error: ", th);
		}

		return id;
	}

	public static Utils getInstance() {
		if (mInstance == null) {
			mInstance = new Utils();
		}

		return mInstance;
	}

	/*
	 * @param context
	 * 
	 * @return defautIP
	 */
	// public static String getIpAddress(Context context){
	// String defautIP = "";
	// if (TextUtils.isEmpty(getNetType(context))){
	// return defautIP;
	// }
	//
	// try {
	// for (Enumeration<NetworkInterface> en = NetworkInterface
	// .getNetworkInterfaces(); en.hasMoreElements();) {
	// NetworkInterface intf = en.nextElement();
	// if (intf.getName().toLowerCase().equals("eth0")
	// || intf.getName().toLowerCase().equals("wlan0")
	// || intf.getName().toLowerCase().equals("ppp0")) {
	// for (Enumeration<InetAddress> enumIpAddr = intf
	// .getInetAddresses(); enumIpAddr.hasMoreElements();) {
	// InetAddress inetAddress = enumIpAddr.nextElement();
	// if (!inetAddress.isLoopbackAddress()) {
	// String ipaddress = inetAddress.getHostAddress()
	// .toString();
	// if (!ipaddress.contains("::")) {// ipV6
	//
	// return ipaddress;
	// }
	// }
	// }
	// } else {
	// continue;
	// }
	// }
	// } catch (SocketException e) {
	// //e.printStackTrace();
	// Log.e(TAG, "ex : " + e.toString());
	// }
	//
	// //LinkProperties ethlp =
	// cm.getLinkProperties(ConnectivityManager.TYPE_ETHERNET);
	// return defautIP;
	// }

	/**
	 * 
	 * @param netType
	 * @return value:up,down
	 */
	public static String getOperState(String netType) {// ppp0,eth0,wlan0
		String filePath = "/sys/class/net/" + netType + "/operstate";
		String temp = ReadFileContent.getFileContent(filePath);
		return temp.trim();
	}

	/*
	 * @param context
	 * 
	 * @return netType
	 */

	// public static String getNetType(Context context){
	// String netType = "";
	// if ("up".equalsIgnoreCase(getOperState("eth0"))){
	// netType = "eth0";
	// Log.i(TAG, "the eth0 type is: " + netType);
	// } else if ("up".equalsIgnoreCase(getOperState("wlan0"))){
	// netType = "wlan0";
	// Log.i(TAG, "the net wlan0 is: " + netType);
	// }
	//
	// if ("up".equalsIgnoreCase(getOperState("ppp0"))){
	// netType = "ppp0";
	// Log.i(TAG, "the net ppp0 is: " + netType);
	// }
	// Log.i(TAG, "the net type is: " + netType);
	// return netType;
	// }

	// public static String getIPState(Context context){
	// String str="";
	// if(Utils.getNetType(context).equalsIgnoreCase("eth0")){
	//
	// EthernetManager mEthManager=
	// (EthernetManager)context.getSystemService("ethernet");
	//
	// if (mEthManager != null && mEthManager.getEthernetMode().equals(
	// EthernetManager.ETHERNET_CONNECT_MODE_DHCP)) {
	// str="dhcp";
	// Log.i(TAG, "readnettype sunjian read dhcp");
	// } else if(mEthManager != null &&
	// !mEthManager.getEthernetMode().equals("NULL")) {
	// str="lan";
	// Log.i(TAG, "readnettype sunjian read static");
	// } else {
	// str="pppoe";
	// Log.i(TAG, "readnettype sunjian read pppoe");
	// }
	// }else if(Utils.getNetType(context).equalsIgnoreCase("wlan0")){
	// WifiManager man = (WifiManager)
	// context.getSystemService(Context.WIFI_SERVICE);
	// if(man.getDhcpInfo().serverAddress == 0 ){
	// str = "lan";
	// }else
	// str = "dhcp";
	// }
	//
	// return str;
	// }

	/*
	 * @param context
	 * 
	 * @return mac
	 */
	// public static String getMac(Context context){
	// String mac = "";
	// String netType = getNetType(context);
	// /*if ("wlan0".equalsIgnoreCase(netType)){
	// WifiManager wifiMng =
	// (WifiManager)context.getSystemService(Context.WIFI_SERVICE);
	// WifiInfo wifiInfor = wifiMng.getConnectionInfo();
	// mac = wifiInfor.getMacAddress();
	// } else if("ppp0".equalsIgnoreCase(netType) ||
	// "eth0".equalsIgnoreCase(netType)){
	// mac =
	// ReadFileContent.getFileContent("/sys/class/net/eth0/address").toUpperCase().substring(0,
	// 17);
	// }*/
	//
	// mac =
	// ReadFileContent.getFileContent("/sys/class/net/eth0/address").toUpperCase().substring(0,
	// 17);
	//
	// Log.i(TAG, "the mac is: " + mac);
	// return mac;
	// }

	// public static String getDeviceModle(){
	// return android.os.Build.MODEL;
	// }

	// public static String getSN(){
	// String softVer = getSysProperties(
	// PROPERTY_STB_ID);
	// return softVer + "1";
	// }

	// public static String getSoftVersion(){
	// return getVersion(PROPERTY_SOFTWAR_ID);
	// }

	// public static String getHardVersion(){
	// int hardVer = Integer.parseInt(getSysProperties(
	// PROPERTY_HW_VER));
	// return "" + hardVer;
	// }

	public static String getServiceAddress(Context context) {
		return GetAuthUrl.getUrl(context);
	}

	public static String getMobilePhoneNumber(Context context) {
		if(null == context) {
			Log.e(TAG, "context is null");
			return "";
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		
		String account = devInfoManager.getValue(DevInfoManager.PHONE);
		Log.d(TAG, "outprinted by James, account: " + account);
		
		if (TextUtils.isEmpty(account)){
			Log.e(TAG, "account is null or account.len less than zero");
			return "";
		}
		
		return account;	
	}
	
	public static void setMobilePhoneNumber(Context context, String value) {
		if(null == context) {
			Log.e(TAG, "context is null");
			return;
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return;
		}
		
		devInfoManager.update(DevInfoManager.PHONE, value, DevInfoManager.Default_Attribute);
		
	}

	public static String getServicePassword(Context context) {
		if(null == context) {
			Log.e(TAG, "context is null");
			return "";
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		
		String aPassword = devInfoManager.getValue(DevInfoManager.SPASSWORD);
		Log.d(TAG, "outprinted by James, aPassword: " + aPassword);
		
		if (TextUtils.isEmpty(aPassword)){
			Log.e(TAG, "aPassword is null or aPassword.len less than zero");
			return "";
		}		
		return aPassword;
	}
	
	public static void setServicePassword(Context context, String value) {
		if(null == context) {
			Log.e(TAG, "context is null");
			return;
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return;
		}
		
		devInfoManager.update(DevInfoManager.SPASSWORD, value, DevInfoManager.Default_Attribute);
	}
	
	public static String getAccount(Context context){
		if(null == context) {
			Log.e(TAG, "context is null");
			return "";
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		
		String phoneNumber = devInfoManager.getValue(DevInfoManager.ACCOUNT);
		Log.d(TAG, "outprinted by James, phoneNumber: " + phoneNumber);
		
		if (TextUtils.isEmpty(phoneNumber)){
			Log.e(TAG, "phoneNumber is null or phoneNumber.len less than zero");
			return "";
		}
		
		return phoneNumber;
	}
	
	public static void setAccount(Context context, String value){
		if(null == context) {
			Log.e(TAG, "context is null");
			return ;
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		
		devInfoManager.update(DevInfoManager.ACCOUNT, value, DevInfoManager.Default_Attribute);
	}
	
	public static String getAccountPassword(Context context){
		if(null == context) {
			Log.e(TAG, "context is null");
			return "";
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		
		String phoneServicePw = devInfoManager.getValue(DevInfoManager.APASSWORD);
		Log.d(TAG, "outprinted by James, phoneServicePw: " + phoneServicePw);
		
		if (TextUtils.isEmpty(phoneServicePw)){
			Log.e(TAG, "phoneServicePw is null or phoneServicePw.len less than zero");
			return "";
		}
		
		return phoneServicePw;
	}
	
	public static void setAccountPassword(Context context, String value){
		if(null == context) {
			Log.e(TAG, "context is null");
			return ;
		}
		
		DevInfoManager devInfoManager = (DevInfoManager)context.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == devInfoManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		
		devInfoManager.update(DevInfoManager.APASSWORD, value, DevInfoManager.Default_Attribute);
	}
	

	private static String getVersion(String temp) {
		String verson = "";
		int softVer = Integer.parseInt(getSysProperties(temp));
		verson = Integer.toHexString(softVer);
		Log.i(TAG, temp + ": " + verson);
		return ChangeToTen("" + verson);
	}

	/**
	 * @return
	 */
	public static final String getSysProperties(String name) {
		String id = "";

		try {
			Class c = Class.forName("android.os.SystemProperties");
			Method m = c.getMethod("get", new Class[] { String.class });
			id = (String) m.invoke(c, new Object[] { name });
		} catch (ClassNotFoundException cnfe) {
			Log.e(TAG, "Error", cnfe);
		} catch (NoSuchMethodException nsme) {
			Log.e(TAG, "Error", nsme);
		} catch (SecurityException se) {
			Log.e(TAG, "Error", se);
		} catch (IllegalAccessException iae) {
			Log.e(TAG, "Error", iae);
		} catch (IllegalArgumentException iarge) {
			Log.e(TAG, "Error", iarge);
		} catch (InvocationTargetException ite) {
			Log.e(TAG, "Error", ite);
		} catch (ClassCastException cce) {
			Log.e(TAG, "Error", cce);
		} catch (Throwable th) {
			Log.e(TAG, "Error: ", th);
		}

		Log.i(TAG, id + ": " + id);
		return id;
	}

	private static String ChangeToTen(String sixteen) {
		Log.d("DeviceInfoSettings", "ChangeToTen string is: " + sixteen);
		if (sixteen.length() == 5) {

			byte[] tmp = sixteen.getBytes();
			// Log.e("DeviceInfoSettings","ChangeToTen siteen.length is 5");
			StringBuffer buildNumber = new StringBuffer();
			for (int i = 0; i < 3; i++) {
				// Log.e("DeviceInfoSettings","byte[] is "+ tmp[i]);
				if (i == 0) {
					int one = 0;
					if (tmp[i] >= '0' && tmp[i] <= '9') {
						one = (tmp[i] - '0');
					} else if (tmp[i] >= 'a' && tmp[i] <= 'f') {
						one = (tmp[i] - 'a' + 10);
					}
					buildNumber.append(one);
				} else {
					int j = (i - 1) * 2 + 1;
					int left = 0;
					int right = 0;
					int sum = 0;
					if (tmp[j] >= '0' && tmp[j] <= '9') {
						left = (tmp[j] - '0') * 16;
					} else if (tmp[j] >= 'a' && tmp[j] <= 'f') {
						left = (tmp[j] - 'a' + 10) * 16;
					}

					if (tmp[j + 1] >= '0' && tmp[j + 1] <= '9') {
						right = (tmp[j + 1] - '0');
					} else if (tmp[j + 1] >= 'a' && tmp[j + 1] <= 'f') {
						right = (tmp[j + 1] - 'a' + 10);
					}

					sum = left + right;
					// Log.e("DeviceInfoSettings","sum is"+sum);
					buildNumber.append(".0").append(sum);
				}
			}
			Log.e("DeviceInfoSettings", "build number is: " + buildNumber);
			return buildNumber.toString();
		}
		return null;
	}

	/**
	 * Gets the device's ID.
	 * 
	 * @param ctx
	 * @return Device id
	 */
	public static final String getDeviceId(Context ctx, int deviceType) {
		if (TextUtils.isEmpty(sDeviceId)) {

			switch (deviceType) {
			case DEVICE_MIPT_IKAN_A6:
			case DEVICE_MIPT_BETV_U6:
			case DEVICE_SKYWORTH_NEXT_PANDORA:
			case DEVICE_SKYWORTH_NORMAL:
			case DEVICE_SKYWORTH_HSM1:
			case DEVICE_SKYWORTH_HAO:
			case DEVICE_MIPT_IKAN_TEST:
			case DEVICE_MIPT_BETV_TEST:
				// skyworth device id
				sDeviceId = getSysProperties("persist.sys.hwconfig.stb_id");
				break;

			case DEVICE_MIPT_IKAN_A4:
			case DEVICE_BST_SNT_B10:
				sDeviceId = ReadFileContent
						.getFileContent(A4_INFO_FILE_DEVICE_ID);
				break;

			case DEVICE_MIPT_IKAN_A8:
				sDeviceId = ReadFileContent
						.getFileContent("/sys/class/mipt_hwconfig/deviceid");
				break;

			case DEVICE_MIPT_IKAN_A5:
			case DEVICE_MIPT_IKAN_R6:
				sDeviceId = ReadFileContent.getFileContent("/hwcfg/.id.txt");
				break;

			default:
				break;
			}
			/*
			 * if (ValueConst.DEBUG) { Log.i(TAG, "DID: " + sDeviceId); }
			 */
		}

		return sDeviceId;
	}

	/**
	 * @param context
	 * @return
	 */
	public static final int deviceType(Context context) {
		SharedPreferences sp = PreferenceManager
				.getDefaultSharedPreferences(context);
		int deviceType = sp.getInt("device_type", 0);

		if (deviceType < 2) { // need load device type from system

			// new way to identify a device
			String platformName = getSysProperties("mipt.ott.platform.name");
			if (!TextUtils.isEmpty(platformName)) {
				if ("A3".equals(platformName) || "A6".equals(platformName)) {
					deviceType = DEVICE_MIPT_IKAN_A6;
				} else if ("A4".equals(platformName)) {
					deviceType = DEVICE_MIPT_IKAN_A4;
				} else if ("A8".equals(platformName)) {
					deviceType = DEVICE_MIPT_IKAN_A8;
				} else if ("A5".equals(platformName)) {
					deviceType = DEVICE_MIPT_IKAN_A5;
				} else if ("R6".equals(platformName)) {
					deviceType = DEVICE_MIPT_IKAN_R6;
				}
			}

			if (deviceType < 2) {
				String url = getSysProperties(HOST_SKYWORTH);
				if (!TextUtils.isEmpty(url)) {
					deviceType = DEVICE_SKYWORTH_NORMAL;
				}
			}

			// old way to identify a device
			if (deviceType < 2) {
				if ("MiPT".equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
					deviceType = checkMiptDevice();

				} else if ("SkyworthDigitalRT"
						.equalsIgnoreCase(android.os.Build.MANUFACTURER)
						|| "SkyworthDigital"
								.equalsIgnoreCase(android.os.Build.MANUFACTURER)
						|| "HAO".equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
					deviceType = checkSkyworthDevice();

				} else if ("BestTech"
						.equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
					deviceType = checkBstDevice();
				}
			}

			// save the useful device information.
			if (deviceType >= 2) {
				SharedPreferences.Editor editor = sp.edit();
				editor.putInt("device_type", deviceType);
				editor.commit();
				Log.d(TAG, "Save device type: " + deviceType);
			}
		}

		// Log.d(TAG, "::TD > " + deviceType);
		return deviceType;
	}

	/**
	 * @return
	 */
	private static final int checkMiptDevice() {
		int deviceType = 0;

		if ("i.Kan".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("A6".equalsIgnoreCase(android.os.Build.MODEL)
					|| "A3".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_A6; // A6, A3
			} else if ("A4".equalsIgnoreCase(android.os.Build.MODEL)
					|| "A400".equalsIgnoreCase(android.os.Build.MODEL)
					|| "A401".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_A4; // A4
			} else if ("A8".equalsIgnoreCase(android.os.Build.MODEL)
					|| "A800".equalsIgnoreCase(android.os.Build.MODEL)
					|| "A800D".equalsIgnoreCase(android.os.Build.MODEL)) { // A8
				deviceType = DEVICE_MIPT_IKAN_A8;
			} else if ("test".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_TEST;
			}
		} else if ("BeTV".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("BeTV-U6".equalsIgnoreCase(android.os.Build.MODEL)
					|| "BeTV-U8".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_BETV_U6; // BeTV-U6
			} else if ("test".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_BETV_TEST;
			}
		}

		return deviceType;
	}

	private static final int checkSkyworthDevice() {
		int deviceType = 0;

		if ("NEXT".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("pandorativibu".equalsIgnoreCase(android.os.Build.MODEL)
					|| "pandora".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_SKYWORTH_NEXT_PANDORA;
			}
		} else if ("Skyworth".equalsIgnoreCase(android.os.Build.BRAND)) {
			if (android.os.Build.MODEL != null
					&& (android.os.Build.MODEL.startsWith("HSM") || android.os.Build.MODEL
							.startsWith("hsm"))) {
				deviceType = DEVICE_SKYWORTH_HSM1;
			}
		} else if ("HAO".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("HA2800".equalsIgnoreCase(android.os.Build.MODEL)
					|| "HAO2".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_SKYWORTH_HAO;
			}
		}

		return deviceType;
	}

	private static final int checkBstDevice() {
		int deviceType = 0;

		if ("SNT".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("SNT-T01".equalsIgnoreCase(android.os.Build.MODEL)
					|| "Shinco F90".equalsIgnoreCase(android.os.Build.MODEL)
					|| "SNT-B11".equalsIgnoreCase(android.os.Build.MODEL)
					|| "AD201".equalsIgnoreCase(android.os.Build.MODEL)
					|| "AD202".equalsIgnoreCase(android.os.Build.MODEL)
					|| "SNT-B09B".equalsIgnoreCase(android.os.Build.MODEL)
					|| "SNT-B09".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_BST_SNT_B10;
			}
		}

		return deviceType;
	}

	public static boolean hasSDCard() {
		String state = Environment.getExternalStorageState();
		if (null != state && Environment.MEDIA_MOUNTED.equals(state)
				&& !Environment.isExternalStorageEmulated())
			return true;
		else
			return false;
	}

	public static String getSaveFilePath() {
		if (hasSDCard()) {
			return Environment.getExternalStorageDirectory().getAbsolutePath()
					+ "/";// filePath:
							// sdcard/video/files/
		} else {
			return Environment.getDataDirectory().getAbsolutePath()
					+ "/data/com.skyworthdigital.tr069/files/";
		}
	}

	public static String getSoftDir() {
		String path = "/sdcard/";// getSaveFilePath();
		File folder = new File(path);
		if (false == folder.exists()) {
			folder.mkdirs();
		}
		Log.i(TAG, "path:" + path);
		return path;
	}

	/**
	 * Convenience function for updating a single settings value. This will
	 * either create a new entry in the table if the given name does not exist,
	 * or modify the value of the existing row with that name.
	 * 
	 * @param cr
	 *            The ContentResolver to access.
	 * @param name
	 *            The name of the setting to modify.
	 * @param value
	 *            The new value for the setting.
	 */
	public static final void putString(ContentResolver cr, String group,
			String name, String value) {
		Uri uri = CONTENT_URI;
		Cursor c = null;
		try {
			c = cr.query(uri, mValueProjection, GROUP_NAME + "=\"" + group
					+ "\" and " + NAME + "=\"" + name + "\"", null, null);

			ContentValues values = new ContentValues();
			values.put(GROUP_NAME, group);
			values.put(NAME, name);
			values.put(VALUE, value);

			if (c.getCount() > 0) {
				cr.update(uri, values, GROUP_NAME + "=\"" + group + "\" and "
						+ NAME + "=\"" + name + "\"", null);
			} else {
				cr.insert(uri, values);
			}
			c.deactivate();
		} catch (Exception e) {
			Log.e(TAG, "The Exception is: " + e.toString());
		} finally {
			if (null != c) {
				try {
					c.close();
				} catch (Exception ex) {
				}
				;
			}
		}
	}

	public static long getDownloadSize(String fileName) {
		long size = 0;
		String filePath = getSoftDir() + fileName + TAG_SUFFIX;
		File file = new File(filePath);
		if (file.exists()) {
			size = file.length();
		}

		return size;
	}

	public static void cleanOld() {
		File file = new File(getSoftDir() + UPDATEZIP);
		File partfile = new File(getSoftDir() + UPDATEZIP + Utils.TAG_SUFFIX);
		if (file != null && file.exists())
			file.delete();
		if (partfile != null && partfile.exists())
			partfile.delete();
		// if (file.isDirectory()) {
		// File[] list = file.listFiles();
		// for (int i = 0; i < list.length; i++) {
		// list[i].delete();
		// }
		// }
		Logger.getLogger().i("del file " + file.getPath() + " and " + partfile);
	}

	/*--------------------------Added function For A8 by James start_part------------------*/
	public static String getSoftVersion() {
		String ver = "";
		try {
			int softVer = Integer.parseInt(SystemProperties.get(PROPERTY_SOFTWAR_ID));
			ver = Integer.toString(softVer);
			if (null != ver && ver.length() > 4) {
		//edit for increment update return like V.700.01 20141114		
//				ver = ver.substring(0, ver.length() - 2) 
//						+ "." + ver.substring(ver.length() - 2);
				ver = ver.substring(0, ver.length() - 4) + "."
						+ ver.substring(ver.length() - 4, ver.length() - 2)
						+ "." + ver.substring(ver.length() - 2);
				return ver;
			}
		} catch (Exception e1) {
			Log.e(TAG, "ex : " + e1.toString());
		}
		return "";
	}

	public static String getHardVersion() {
		return SystemProperties.get(PROPERTY_HW_VER);
	}

	public static String getSN() {
		String sn = readStringFromFile("/sys/class/mipt_hwconfig/serialid", null);
		Log.d(TAG, "outprinted by James, sn : " + sn);
		
		if(TextUtils.isEmpty(sn)) {
			Log.e(TAG, "sn is null or sn.len less than zero");
			return "";
		}
		return sn;	
	}

	/*
	 * @param context
	 * 
	 * @return mac
	 */
	public static String getMac(Context context) {
		String mac = "";
		String netType = getNetType(context);
		/*if(TextUtils.isEmpty(netType)) {
			Log.d(TAG, "netType is null or netType.len less than zero");
			return mac;
		}
		
		if("eth0".equalsIgnoreCase(netType)) {
			mac = ReadFileContent.getFileContent("/sys/class/net/eth0/address").toUpperCase().substring(0, 17);
			Log.i(TAG, "eth0 mac is: " + mac);
		} else if ("wlan0".equalsIgnoreCase(netType)) {
			WifiManager wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
			WifiInfo wifiInfo = wifiManager.getConnectionInfo();
			mac = wifiInfo.getMacAddress();
			mac = ReadFileContent.getFileContent("/sys/class/net/wlan0/address").toUpperCase().substring(0, 17);
			Log.i(TAG, "wlan0 mac is: " + mac);
		} else if ("ppp0".equalsIgnoreCase(netType)) {
			mac = getSysProperties("dhcp.ppp0.ipaddress");
			Log.i(TAG, "ppp0 mac is: " + mac);
		}*/
		mac = ReadFileContent.getFileContent("/sys/class/net/eth0/address").toUpperCase().substring(0, 17);
		Log.i(TAG, "the mac is: " + mac);
		
		return mac;
	}

	public static String getDeviceId() {
		String deviceId = readStringFromFile("/sys/class/mipt_hwconfig/deviceid", null);
		Log.d(TAG, "outprinted by James, deviceId : " + deviceId);
		
		if(TextUtils.isEmpty(deviceId)) {
			Log.e(TAG, "deviceId is null or deviceId.len less than zero");
			return "";
		}
		return deviceId;
	}
	
	private static String readStringFromFile(String path, String def) {
		BufferedReader reader = null;
		try {
			File fl = new File(path);

			if (!fl.exists() || !fl.canRead())
					return def;
			StringBuffer fileData = new StringBuffer(100);
			reader = new BufferedReader(new FileReader(path));
			char[] buf = new char[100];
			int numRead = 0;
			while ((numRead = reader.read(buf)) != -1) {
				String readData = String.valueOf(buf, 0, numRead);
				fileData.append(readData);
			}
			
			reader.close();
			return fileData.toString();
		} catch (Throwable e) {
			Log.e(TAG, "Exception , ex : " + e.toString());
		} finally {
			if (null != reader)
				try {
					reader.close();
					reader = null;
				} catch (Throwable t) {
					
				}
		}
		return def;
	}

	public static boolean getPPPOEState(String cmd) {
		String[] cmdStrings = new String[] { "sh", "-c", cmd };
		try {
			Process process = Runtime.getRuntime().exec(cmdStrings);
			BufferedReader stdout = new BufferedReader(new InputStreamReader(
					process.getInputStream()), 7777);
			BufferedReader stderr = new BufferedReader(new InputStreamReader(
					process.getErrorStream()), 7777);

			String line = null;

			while ((null != (line = stdout.readLine()))
					|| (null != (line = stderr.readLine()))) {
				Log.e("getNetCfgInfo", "line: " + line);
				if (line.toLowerCase().contains("ppp0")
						&& line.toLowerCase().contains("up")) {
					return true;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return false;
	}

	/**
	 * 
	 * @param context
	 * @return
	 */
	public static String getNetType(Context context) {
		String netType = "";
		if ("up".equalsIgnoreCase(getOperState("eth0"))) {
			netType = "eth0";
			// ppp0:unknow ; p2p0:dormant
			if (getPPPOEState("netcfg")) {
				netType = "ppp0";
			}
			/*
			 * if (!TextUtils.isEmpty(getOperState("ppp0")) ||
			 * "dormant".equalsIgnoreCase("p2p0")) { netType = "ppp0"; }
			 */
		} else if ("up".equalsIgnoreCase(getOperState("wlan0"))) {
			netType = "wlan0";
		}
		Log.i(TAG, "the net type is: " + netType);
		return netType;
	}

	/**
	 * 
	 * @param context
	 * @return
	 */
	public static String getIPState(Context context) {
		String str = "";
		String netType = getNetType(context);
		Log.i(TAG, "netType : " + netType);
		if (netType.equalsIgnoreCase("eth0")) {
			// EthernetManager mEthManager=
			// (EthernetManager)context.getSystemService("ETHERNET"/*"ethernet"*/);
			// if (mEthManager != null &&
			// mEthManager.getEthernetMode().equals(EthernetManager.ETHERNET_CONNECT_MODE_DHCP))
			// {
			// str="dhcp";
			// Log.i(TAG, "readnettype sunjian read dhcp");
			// } else if(mEthManager != null &&
			// !mEthManager.getEthernetMode().equals("NULL")) {
			// str="lan";
			// Log.i(TAG, "readnettype sunjian read static");
			// } else {
			// str="ppp0";
			// Log.i(TAG, "readnettype sunjian read pppoe");
			// }
			EthernetManager mEthManager = (EthernetManager) context
					.getSystemService(Context.ETH_SERVICE);
			EthernetDevInfo mEthInfo = mEthManager.getSavedEthConfig();
			if (null != mEthInfo
					&& mEthInfo.getConnectMode().equals(
							EthernetDevInfo.ETH_CONN_MODE_DHCP)) {
				str = "dhcp";
				Log.i(TAG, "readnettype sunjian read dhcp");
			} else if (mEthManager != null
					&& !mEthInfo.getConnectMode().equals("NULL")) {
				str = "lan";
				Log.i(TAG, "readnettype sunjian read static");
			}
		} else if (netType.equalsIgnoreCase("wlan0")) {
			WifiManager wifiManager = (WifiManager) context
					.getSystemService(Context.WIFI_SERVICE);
			Log.i(TAG, "wifiManager.getDhcpInfo().serverAddress: "
					+ wifiManager.getDhcpInfo().serverAddress);
			if (wifiManager.getDhcpInfo().serverAddress == 0) {
				str = "lan";
			} else {
				str = "dhcp";
			}
		} else if (netType.equalsIgnoreCase("ppp0")) {
			str = "pppoe";
			Log.i(TAG, "readnettype sunjian read pppoe");
		}
		return str;
	}

	/*
	 * @param context
	 * 
	 * @return defautIP
	 */
	public static String getIpAddress(Context context) {
		String defautIP = "";
		String netType = getNetType(context);
		Log.i(TAG, "netType : " + netType);
		if (TextUtils.isEmpty(netType)) {
			return defautIP;
		}

		ConnectivityManager connectivityManager = (ConnectivityManager) context
				.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
		if (netType.equals("eth0")) {
			try {
				if (networkInfo != null) {
					if (networkInfo.getTypeName().toString()
							.equalsIgnoreCase("ETHERNET")) {
						if (networkInfo.isConnected() && networkInfo.isAvailable()) {
							EthernetManager mEthManager = (EthernetManager) context
									.getSystemService(Context.ETH_SERVICE);
							EthernetDevInfo mEthInfo = mEthManager
									.getSavedEthConfig();
							if (null == mEthInfo) {
								Log.e(TAG, "mEthInfo is null");
								return "";
							} else if (mEthInfo.getConnectMode().equals(
									EthernetDevInfo.ETH_CONN_MODE_DHCP)) {
								DhcpInfo dhcpInfo = mEthManager.getDhcpInfo();
								// return getAddress(dhcpInfo.ipAddress);
								try {
									if (null == dhcpInfo) {
										Log.e(TAG, "dhcpInfo is null");
										return "";
									}

									defautIP = getAddress(dhcpInfo.ipAddress);
									if (defautIP.equals("0.0.0.0")
											|| null == defautIP) {
										return "";
									} else {
										return defautIP;
									}
								} catch (Exception e) {
									e.printStackTrace();
									Log.e(TAG,
											"getIpAddress Exception : "
													+ e.toString());
									return "";
								}
							} else {
								try {
									defautIP = mEthInfo.getIpAddress();
									if (defautIP.equals("0.0.0.0")
											|| null == defautIP) {
										return "";
									} else {
										return defautIP;
									}
								} catch (Exception e) {
									e.printStackTrace();
									Log.e(TAG,
											"getIpAddress Exception : "
													+ e.toString());
									return "";
								}
							}
						}
					}
				}
			} catch (Exception e) {
				Log.d(TAG, "eth0 exception , ex : " + e.toString());
				return defautIP;
			}
		} else if (netType.equals("wlan0")) {
			try {
				if(networkInfo != null) {
					if (networkInfo.isConnected() && networkInfo.isAvailable()) {
						WifiManager mWifiManager = (WifiManager) context
								.getSystemService(Context.WIFI_SERVICE);
						final WifiInfo wifiInfo = mWifiManager.getConnectionInfo();
						int ipAddr = wifiInfo.getIpAddress();
						StringBuffer ipBuf = new StringBuffer();
						ipBuf.append(ipAddr & 0xff).append('.')
								.append((ipAddr >>>= 8) & 0xff).append('.')
								.append((ipAddr >>>= 8) & 0xff).append('.')
								.append((ipAddr >>>= 8) & 0xff);
						return ipBuf.toString();
					}
				}
			} catch (Exception e) {
				Log.d(TAG, "wlan0 exception , ex : " + e.toString());
				return defautIP;
			}
		} else if (netType.equals("ppp0")) {
			return getPPP0IpAddress();
		}

		// try {
		// for (Enumeration<NetworkInterface> en = NetworkInterface
		// .getNetworkInterfaces(); en.hasMoreElements();) {
		// NetworkInterface intf = en.nextElement();
		// Log.e(TAG, "intf.Name: " + intf.getName());
		// if (intf.getName().toLowerCase().equals("eth0")
		// || intf.getName().toLowerCase().equals("wlan0")
		// || intf.getName().toLowerCase().equals("ppp0")) {
		// for (Enumeration<InetAddress> enumIpAddr = intf
		// .getInetAddresses(); enumIpAddr.hasMoreElements();) {
		// InetAddress inetAddress = enumIpAddr.nextElement();
		// if (!inetAddress.isLoopbackAddress()) {
		// String ipaddress = inetAddress.getHostAddress()
		// .toString();
		// if (!ipaddress.contains("::")) {// ipV6
		// return ipaddress;
		// }
		// }
		// }
		// } else {
		// continue;
		// }
		// }
		// } catch (SocketException e) {
		// Log.e(TAG, "ex: " + e.toString());
		// }
		return defautIP;
	}

	public static String getPPP0IpAddress() {
		return getSysProperties("net.ppp0.local-ip");
	}

	public static String getDeviceModle() {
		return android.os.Build.MODEL;
	}

	private static String getAddress(int addr) {
		return NetworkUtils.intToInetAddress(addr).getHostAddress();
	}

	public static String getPPPoeUserName(Context pramaContext) {
		if(null == pramaContext) {
			Log.e(TAG, "pramaContext is null");
			return "";
		}
		
		/*SharedPreferences mSharedPreferences = null;
		try {
			Context otherAppsContext = pramaContext.createPackageContext("com.amlogic.PPPoE", Context.CONTEXT_IGNORE_SECURITY);
			mSharedPreferences = otherAppsContext.getSharedPreferences("inputdata", Context.MODE_WORLD_READABLE);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
			Log.e(TAG, "NameNotFoundException , e : " + e.toString());
			return "";
		}
		
		if(null == mSharedPreferences) {
			Log.e(TAG, "mSharedPreferences is null");
			return "";
		}
		
		String pppoeUserName = mSharedPreferences.getString("name", null);*/

		String pppoeUserName = Settings.Secure.getString(pramaContext.getContentResolver(), "pppoe_username");
		String netType = getIPState(pramaContext);//getNetType(pramaContext);
		if(TextUtils.isEmpty(netType)) {
			Log.e(TAG, "netType is null");
			return "";
		}
		Log.i(TAG, "outprint by James , pppoeUserName : " + pppoeUserName + " , netType : " + netType);
		
		if(netType.equalsIgnoreCase("pppoe")) {
			if (TextUtils.isEmpty(pppoeUserName)) {
				Log.e(TAG, "pppoeUserName is null");
				return "";
			} else {
				return pppoeUserName;
			}
		} else {
			return "";
		}
		

	}

	public static String getPPPoePassword(Context pramaContext) {
		if(null == pramaContext) {
			Log.e(TAG, "pramaContext is null");
			return "";
		}
		
		/*SharedPreferences mSharedPreferences = null;
		try {
			Context otherAppsContext = pramaContext.createPackageContext("com.amlogic.PPPoE", Context.CONTEXT_IGNORE_SECURITY);
			mSharedPreferences = otherAppsContext.getSharedPreferences("inputdata", Context.MODE_WORLD_READABLE);
		} catch (NameNotFoundException e) {
			e.printStackTrace();
			Log.e(TAG, "NameNotFoundException , e : " + e.toString());
			return "";
		}
		
		if(null == mSharedPreferences) {
			Log.e(TAG, "mSharedPreferences is null");
			return "";
		}
		
		String pppoePassword = mSharedPreferences.getString("passwd", null);*/
		String pppoePassword = Settings.Secure.getString(pramaContext.getContentResolver(), "pppoe_password");
		String netType = getIPState(pramaContext);//getNetType(pramaContext);
		if(TextUtils.isEmpty(netType)) {
			Log.e(TAG, "netType is null");
			return "";
		}
		Log.i(TAG, "outprint by James , pppoePassword : " + pppoePassword + " , netType : " + netType);
		
		if(netType.equalsIgnoreCase("pppoe")) {
			if (TextUtils.isEmpty(pppoePassword)) {
				Log.e(TAG, "pppoePassword is null");
				return "";
			} else {
				return pppoePassword;
			}
		} else {
			return "";
		}
	}

	/*
	 * public static String getWifiLinkIp(Activity paramActivity) {
	 * ConnectivityManager connectivityManager = (ConnectivityManager)
	 * paramActivity .getSystemService(Context.CONNECTIVITY_SERVICE);
	 * NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo(); if
	 * (networkInfo != null) { if (networkInfo.getTypeName().toString()
	 * .equalsIgnoreCase("ETHERNET")) { if (networkInfo.isConnected() &&
	 * networkInfo.isAvailable()) { EthernetManager mEthManager =
	 * (EthernetManager) paramActivity .getSystemService(Context.ETH_SERVICE);
	 * EthernetDevInfo mEthInfo = mEthManager.getSavedEthConfig(); if (null ==
	 * mEthInfo) { return ""; } else if (mEthInfo.getConnectMode().equals(
	 * EthernetDevInfo.ETH_CONN_MODE_DHCP)) { DhcpInfo dhcpInfo =
	 * mEthManager.getDhcpInfo(); return getAddress(dhcpInfo.ipAddress); } else
	 * { return mEthInfo.getIpAddress(); } }
	 * 
	 * } else if (networkInfo.getType() == ConnectivityManager.TYPE_WIFI) { if
	 * (networkInfo.isConnected() && networkInfo.isAvailable()) { WifiManager
	 * mWifiManager = (WifiManager) paramActivity
	 * .getSystemService(Context.WIFI_SERVICE); final WifiInfo wifiInfo =
	 * mWifiManager.getConnectionInfo(); int ipAddr = wifiInfo.getIpAddress();
	 * StringBuffer ipBuf = new StringBuffer(); ipBuf.append(ipAddr &
	 * 0xff).append('.') .append((ipAddr >>>= 8) & 0xff).append('.')
	 * .append((ipAddr >>>= 8) & 0xff).append('.') .append((ipAddr >>>= 8) &
	 * 0xff); return ipBuf.toString(); } } } return ""; }
	 */

	/*
	 * public static String getEthernetIp(Activity paramActivity) {
	 * ConnectivityManager connectivityManager = (ConnectivityManager)
	 * paramActivity .getSystemService(Context.CONNECTIVITY_SERVICE);
	 * NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo(); if
	 * (networkInfo != null) { Log.i(TAG, "----->getTypeName: " +
	 * networkInfo.getTypeName() + " , isAvailable: " +
	 * networkInfo.isAvailable() + " , isConnected: " +
	 * networkInfo.isConnected()); if (networkInfo.getTypeName().toString()
	 * .equalsIgnoreCase("ETHERNET")) { if (networkInfo.isConnected() &&
	 * networkInfo.isAvailable()) { EthernetManager mEthManager =
	 * (EthernetManager) paramActivity .getSystemService(Context.ETH_SERVICE);
	 * EthernetDevInfo mEthInfo = mEthManager.getSavedEthConfig(); if (null ==
	 * mEthInfo) { Log.i(TAG, "mEthInfo is null"); return ""; } else if
	 * (mEthInfo.getConnectMode().equals( EthernetDevInfo.ETH_CONN_MODE_DHCP)) {
	 * DhcpInfo dhcpInfo = mEthManager.getDhcpInfo(); Log.i(TAG,
	 * "dhcpInfo.Address :" + getAddress(dhcpInfo.ipAddress)); return
	 * getAddress(dhcpInfo.ipAddress); } else { Log.i(TAG, "mEthInfo.Address :"
	 * + mEthInfo.getIpAddress()); return mEthInfo.getIpAddress(); } } } }
	 * return ""; }
	 */
	
	public static void saveSharedPreference(Context mContext, String key,
			String value) {
		if (null == mContext || TextUtils.isEmpty(key) || TextUtils.isEmpty(value)) {
			Log.e(TAG, "bad invaild param for save sharedpreference");
			return;
		}
		SharedPreferences shared = mContext.getSharedPreferences(SHAREPREFERENCED_KEY, 0);
		Editor editor = shared.edit();
		editor.putString(key, value);
		editor.commit();
	}
	
	public static String getSharedPreference(Context mContext, String key) {
		String value = "";
		if (null == mContext || TextUtils.isEmpty(key)) {
			Log.e(TAG, "bad invaild param for get sharedpreference");
			return value;
		}
		SharedPreferences shared = mContext.getSharedPreferences(SHAREPREFERENCED_KEY, 0);
		value = shared.getString(key, "");
		return value;
	}
	
	public static final void startAdb(){
		DataOutputStream os = null;
		try {
		   //Process p = Runtime.getRuntime().exec("/system/xbin/su");
			Process p = Runtime.getRuntime().exec("start adbd");
			os = new DataOutputStream(p.getOutputStream());
			Log.e("start adbd ","start adbd");
		   //os.writeBytes("start adbd");
			os.flush();
		} catch (IOException e) {
			e.printStackTrace();
			Log.e(TAG, "IOException , e : " + e.toString());
		} finally{
			if (null != os){
				try {
					os.close();
				} catch (IOException e) {
					e.printStackTrace();
					Log.e(TAG, "IOException , e : " + e.toString());
				}
				os = null;
			}
		}
	}
	
	public static final void stopAdb(){
		DataOutputStream os = null;
		try {
			//Process p = Runtime.getRuntime().exec("/system/xbin/su");
			Process p = Runtime.getRuntime().exec("stop adbd");
			os = new DataOutputStream(p.getOutputStream());
			Log.e("stop adbd ","stop adbd");
			//os.writeBytes("stop adbd");
			os.flush();
		} catch (IOException e) {
			e.printStackTrace();
			Log.e(TAG, "IOException , e : " + e.toString());
		} finally{
			if (null != os){
				try {
					os.close();
				} catch (IOException e) {
					e.printStackTrace();
					Log.e(TAG, "IOException , e : " + e.toString());
				}
				os = null;
			}
		}
	}

	public static final String setOttDeviceId(String setData, String value) {
		String id = "";
		try {
			Class<?> c = Class.forName(("android.os.SystemProperties"));
			Method m = c.getMethod("set", new Class[] { String.class ,String.class});
			id = (String) m.invoke(c, new Object[] { setData, value });
		} catch (ClassNotFoundException cnfe) {
			Log.e(TAG, "Error : " + cnfe.toString());
		} catch (NoSuchMethodException nsme) {
			Log.e(TAG, "Error : " + nsme.toString());
		} catch (SecurityException se) {
			Log.e(TAG, "Error : " + se.toString());
		} catch (IllegalAccessException iae) {
			Log.e(TAG, "Error : " + iae.toString());
		} catch (IllegalArgumentException iarge) {
			Log.e(TAG, "Error : " + iarge.toString());
		} catch (InvocationTargetException ite) {
			Log.e(TAG, "Error : " + ite.toString());
		} catch (ClassCastException cce) {
			Log.e(TAG, "Error : " + cce.toString());
		} catch (Throwable th) {
			Log.e(TAG, "Error : " + th.toString());
		}
		return id;
	}

	public static final String getOttDeviceId(String getData) {
		String id = "";
		try {
			Class<?> c = Class.forName(("android.os.SystemProperties"));
			Method m = c.getMethod("get", new Class[] { String.class });
			id = (String) m.invoke(c, new Object[] { getData });
		} catch (ClassNotFoundException cnfe) {
			Log.e(TAG, "Error : " + cnfe.toString());
		} catch (NoSuchMethodException nsme) {
			Log.e(TAG, "Error : " + nsme.toString());
		} catch (SecurityException se) {
			Log.e(TAG, "Error : " + se.toString());
		} catch (IllegalAccessException iae) {
			Log.e(TAG, "Error : " + iae.toString());
		} catch (IllegalArgumentException iarge) {
			Log.e(TAG, "Error : " + iarge.toString());
		} catch (InvocationTargetException ite) {
			Log.e(TAG, "Error : " + ite.toString());
		} catch (ClassCastException cce) {
			Log.e(TAG, "Error : " + cce.toString());
		} catch (Throwable th) {
			Log.e(TAG, "Error : " + th.toString());
		}
		return id;
	}
	
	/**
	 * 
	 * @param paramStr
	 * @return
	 */
	public static final boolean setUserIntallApplication(String paramStr) {
		if(TextUtils.isEmpty(paramStr)) {
			Log.e(TAG, "outprinted by James, paramStr is null");
			return false;
		}
		
		//setOttDeviceId(PROPERTY_SET, paramStr);
		String m_str = setOttDeviceId(PROPERTY_SET, paramStr);
		Log.e(TAG, "set setUserInstallAppliaction , m_str: " + m_str);
		/*if(TextUtils.isEmpty(m_str)) {
			Log.e(TAG, "outprinted by James, m_str is null");
			return false;
		}*/
		
		if(PROPERTY_VALUE_0.equalsIgnoreCase(paramStr) 
				|| PROPERTY_VALUE_1.equalsIgnoreCase(paramStr)) {
			stopAdb();
		} else {			
			startAdb();
		}
		return true;
	}
	
	/**
	 * 
	 * @return
	 */
	public static final int getUserIntallApplication() {
		String m_str = getOttDeviceId(PROPERTY_SET);
		/*if(TextUtils.isEmpty(m_str)) {
			Log.e(TAG, "outprinted by James, m_str is null");
			return "";
		}*/		
		try {
			int m_value = Integer.parseInt(m_str);
			Log.d(TAG, "outprinted by James, m_value: " + m_value);
			
			return m_value;
		} catch (NumberFormatException e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
		}
		return 0;
	}
	
	/**
	 * 
	 * @param paramContext
	 * @param paramStr
	 */
	public static final void setUrl(Context paramContext, String paramStr) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return;
		}
		
		if(TextUtils.isEmpty(paramStr) && !paramStr.startsWith("http")) {
			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
			return;
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		Settings.System.putString(contentResolver, Settings.System.TR069_PRI, paramStr);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		mDevManager.update(DevInfoManager.CMS_URL, paramStr, DevInfoManager.Default_Attribute);
	}
	
	/**
	 * 
	 * @param paramContext
	 * @return
	 */
	public static final String getUrl(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		String platformUrl = Settings.System.getString(contentResolver, Settings.System.TR069_PRI);
		Log.d(TAG, "outprinted by James, platformUrl: " + platformUrl);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		String cmsUrl = mDevManager.getValue(DevInfoManager.CMS_URL);
		
		if(TextUtils.isEmpty(cmsUrl)) {
			Log.e(TAG, "cmsUrl is null or cmsUrl.len less than zero");
			return "";
		} else {
			Log.d(TAG, "outprinted by James, cmsUrl: " + cmsUrl);
			return cmsUrl;
		}
	}
	
	/**
	 * 
	 * @param paramContext
	 * @param paramStr
	 */
	public static final void setUrlBackup(Context paramContext, String paramStr) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return;
		}
		
		if(TextUtils.isEmpty(paramStr) && !paramStr.startsWith("http")) {
			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
			return;
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		Settings.System.putString(contentResolver, Settings.System.TR069_SEC, paramStr);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		mDevManager.update(DevInfoManager.CMS_URL_BACKUP, paramStr, DevInfoManager.Default_Attribute);
	}
	
	/**
	 * 
	 * @param paramContext
	 * @return
	 */
	public static final String getUrlBackup(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		String platformUrlBackup = Settings.System.getString(contentResolver, Settings.System.TR069_SEC);
		Log.d(TAG, "outprinted by James, platformUrlBackup: " + platformUrlBackup);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		String cmsUrlBackup = mDevManager.getValue(DevInfoManager.CMS_URL_BACKUP);
		
		if(TextUtils.isEmpty(cmsUrlBackup)) {
			Log.e(TAG, "cmsUrlBackup is null or cmsUrlBackup.len less than zero");
			return "";
		} else {
			Log.d(TAG, "outprinted by James, cmsUrlBackup: " + cmsUrlBackup);
			return cmsUrlBackup;
		}
	}
	
	public static final void setPlatformUrl(Context paramContext, String paramStr) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return;
		}
		
		if(TextUtils.isEmpty(paramStr) && !paramStr.startsWith("http")) {
			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
			return;
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		Settings.System.putString(contentResolver, Settings.System.PRIMARY_IP, paramStr);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		mDevManager.update(DevInfoManager.PLATFORM_URL, paramStr, DevInfoManager.Default_Attribute);
	}
	
	public static final String getPlatformUrl(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		String url = Settings.System.getString(contentResolver, Settings.System.PRIMARY_IP);
		Log.d(TAG, "outprinted by James, url: " + url);*/		
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		String platformUrl = mDevManager.getValue(DevInfoManager.PLATFORM_URL);
		
		if(TextUtils.isEmpty(platformUrl)) {
			Log.e(TAG, "platformUrl is null or platformUrl.len less than zero");
			return "";
		} else {
			Log.d(TAG, "outprinted by James, platformUrl: " + platformUrl);
			return platformUrl;
		}
	}
	
	public static final void setPlatformUrlBackup(Context paramContext, String paramStr) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return;
		}
		
		if(TextUtils.isEmpty(paramStr) && !paramStr.startsWith("http")) {
			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
			return;
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		Settings.System.putString(contentResolver, Settings.System.SECONDARY_IP, paramStr);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		mDevManager.update(DevInfoManager.PLATFORM_URL_BACKUP, paramStr, DevInfoManager.Default_Attribute);
	}
	
	public static final String getPlatformUrlBackup(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		String urlBackup = Settings.System.getString(contentResolver, Settings.System.SECONDARY_IP);
		Log.d(TAG, "outprinted by James, urlBackup: " + urlBackup);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		String platformUrlBackup = mDevManager.getValue(DevInfoManager.PLATFORM_URL_BACKUP);
		
		if(TextUtils.isEmpty(platformUrlBackup)) {
			Log.e(TAG, "platformUrlBackup is null or platformUrlBackup.len less than zero");
			return "";
		} else {
			Log.d(TAG, "outprinted by James, platformUrlBackup: " + platformUrlBackup);
			return platformUrlBackup;
		}
	}

	public static final void setHdcUrl(Context paramContext, String paramStr) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return;
		}
		
		if(TextUtils.isEmpty(paramStr) && !paramStr.startsWith("http")) {
			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
			return;
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		Settings.System.putString(contentResolver, Settings.System.HDC_ADDR, paramStr);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return ;
		}
		mDevManager.update(DevInfoManager.HDC_URL, paramStr, DevInfoManager.Default_Attribute);
	}
	
	public static final String getHdcUrl(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		/*ContentResolver contentResolver = paramContext.getContentResolver();
		String hdcUrl = Settings.System.getString(contentResolver, Settings.System.HDC_ADDR);
		Log.d(TAG, "outprinted by James, hdcUrl: " + hdcUrl);*/
		DevInfoManager mDevManager = (DevInfoManager)paramContext.getSystemService(DevInfoManager.DATA_SERVER);
		if(null == mDevManager) {
			Log.e(TAG, "devInfoManager is null");
			return "";
		}
		String hdcUrl = mDevManager.getValue(DevInfoManager.HDC_URL);
		
		if(TextUtils.isEmpty(hdcUrl)) {
			Log.e(TAG, "hdcUrl is null or hdcUrl.len less than zero");
			return "";
		} else {
			Log.d(TAG, "outprinted by James, hdcUrl: " + hdcUrl);
			return hdcUrl;
		}
	}
	
//	public static final void setUserId(Context paramContext, String paramStr) {
//		if(null == paramContext) {
//			Log.e(TAG, "paramContext is null");
//			return;
//		}
//		
//		if(TextUtils.isEmpty(paramStr)) {
//			Log.e(TAG, "paramStr is null or paramStr.len less than zero");
//			return;
//		}
//	}
//	
//	public static final String getUserId(Context paramContext) {
//		if(null == paramContext) {
//			Log.e(TAG, "paramContext is null");
//			return "";
//		}
//		
//		return "";
//	}
	
	public static final void setSilentUpgrade(int paramFlag) {		
		try {
			String m_flag = Integer.toString(paramFlag);
			String silentUpgradeValue = setOttDeviceId(SILENT_UPGRADE, m_flag);
			Log.d(TAG, "set silentUpgradeValue: " + silentUpgradeValue);
		} catch (Exception e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
		}
	}
	
	public static final int getSilentUpgrade() {
		try {
			String m_flag = getOttDeviceId(SILENT_UPGRADE);
			if(TextUtils.isEmpty(m_flag)) {
				Log.e(TAG, "m_flag is null or m_flag.len less than zero");
				return 0;
			}
			
			int silentUpgradeFlag = Integer.parseInt(m_flag);
			Log.d(TAG, "outprinted by James, silentUpgradeFlag: " + silentUpgradeFlag);
			
			return silentUpgradeFlag;
		} catch (NumberFormatException e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
			return 0;
		}
	}
	
	public static final void setForcedUpgrade(int paramFlag) {		
		try {
			String m_flag = Integer.toString(paramFlag);
			String forcedUpgradeValue = setOttDeviceId(FORCED_UPGRADE, m_flag);
			Log.d(TAG, "set forcedUpgradeValue: " + forcedUpgradeValue);
		} catch (Exception e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
		}
	}
	
	public static final int getForcedUpgrade() {
		try {
			String m_flag = getOttDeviceId(FORCED_UPGRADE);
			if(TextUtils.isEmpty(m_flag)) {
				Log.e(TAG, "m_flag is null or m_flag.len less than zero");
				return 1;
			}
			
			int forcedUpgradeFlag = Integer.parseInt(m_flag);
			Log.d(TAG, "outprinted by James, forcedUpgradeFlag: " + forcedUpgradeFlag);
			
			return forcedUpgradeFlag;
		} catch (NumberFormatException e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
			return 1;
		}
	}
	
	public static String getAcsUserName(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		String url = "";
		String str = "";
		url = Utils.getUrl(paramContext);
		
		if(!TextUtils.isEmpty(url)) {
			Log.d(TAG, "outprinted by James , user_name url: " + url);			
			if(url.trim().equals("http://112.4.1.10:37021/acs")) {	//"http://10.254.0.59:37020/acs"			
				//str = "acs";
				str = "testcpe";
				Log.d(TAG, "outprinted by James , 11 user_name:  " + str);
			} else {
				str = "testcpe";
				Log.d(TAG, "outprinted by James , 22 user_name:  " + str);
			}
		} else {
			str = "testcpe";
			Log.d(TAG, "outprinted by James , 33 user_name:  " + str);
		}
		
		Log.d(TAG, "outprinted by James , final user_name str: " + str);
		return str;
	}
	
	public static String getAcsPassword(Context paramContext) {
		if(null == paramContext) {
			Log.e(TAG, "paramContext is null");
			return "";
		}
		
		String url = "";
		String str = "";
		url = Utils.getUrl(paramContext);
		
		if(!TextUtils.isEmpty(url)) {
			Log.d(TAG, "outprinted by James , password url: " + url);			
			if(url.trim().equals("http://112.4.1.10:37021/acs")) {	//"http://10.254.0.59:37020/acs"			
				//str = "acs";
				str = "ac5entry";
				Log.d(TAG, "outprinted by James , 11 password:  " + str);
			} else {
				str = "ac5entry";
				Log.d(TAG, "outprinted by James , 22 password:  " + str);
			}
		} else {
			str = "ac5entry";
			Log.d(TAG, "outprinted by James , 33 password:  " + str);
		}
		
		Log.d(TAG, "outprinted by James , final password str: " + str);
		return str;
	}
	
	public static String getNtpServer(Context paramContext) {		
		String ntpServer = getOttDeviceInfo(NTP_SERVER);
		Log.d(TAG, "outprinted by James , ntpServer: " + ntpServer);
		if(!TextUtils.isEmpty(ntpServer)) {			
			return ntpServer;
		} else {
			Log.d(TAG, "outprinted by James , ntpServer is empty.");
			if(null == paramContext) {
				Log.e(TAG, "paramContext is null");
				return "";
			}
			
			Resources res = paramContext.getResources();
			String defaultNtpServer = res.getString(com.android.internal.R.string.config_ntpServer);
			Log.d(TAG, "outprinted by James , defaultNtpServer: " + defaultNtpServer);
			if(!TextUtils.isEmpty(defaultNtpServer)) {
				return defaultNtpServer;	
			} else {
				return "";
			}
		}
	}
	
	public static void setNtpServer(String value) {
		Log.d(TAG, "outprinted by James , user set ntpServer value: " + value);
		try {
			String result = setOttDeviceId(NTP_SERVER,value);
			Log.d(TAG, "outprinted by James , ntpServer is setted value: " + result);
		} catch (Exception e) {
			e.printStackTrace();
			Log.e(TAG, "ex: " + e.toString());
		}
	}
	
	public static String getMask(Context paramContext) {
		String mask = "";
		String netType = getNetType(paramContext);
		if(TextUtils.isEmpty(netType)) {
			Log.d(TAG, "netType is null or netType.len less than zero");
			return mask;
		}
		
		if("eth0".equalsIgnoreCase(netType)) {
			/*WifiManager wifiManager = (WifiManager) paramContext.getSystemService(Context.WIFI_SERVICE);
			DhcpInfo dhcpInfo = wifiManager.getDhcpInfo();
			mask = Formatter.formatIpAddress(dhcpInfo.netmask);*/
			mask = getSysProperties("dhcp.eth0.mask");
			Log.d(TAG, "eth0 mask is: " + mask);
		} else if("wlan0".equalsIgnoreCase(netType)) {
			mask = getSysProperties("dhcp.wlan0.mask");
			Log.d(TAG, "wlan0 mask is: " + mask);
		} else if("ppp0".equalsIgnoreCase(netType)) {
			mask = getSysProperties("dhcp.ppp0.mask");
			Log.d(TAG, "ppp0 mask is: " + mask);
		}
		
		return mask;
	}
	
	public static String getDnsServer(Context paramContext) {
		String dnsServer = "";
		String netType = getNetType(paramContext);
		if(TextUtils.isEmpty(netType)) {
			Log.d(TAG, "netType is null or netType.len less than zero");
			return dnsServer;
		}
		
		if("eth0".equalsIgnoreCase(netType)) {
			dnsServer = getSysProperties("dhcp.eth0.dns1");
			Log.d(TAG, "eth0 dnsServer is: " + dnsServer);
		} else if("wlan0".equalsIgnoreCase(netType)) {
			dnsServer = getSysProperties("dhcp.wlan0.dns1");
			Log.d(TAG, "wlan0 dnsServer is: " + dnsServer);
		} else if("ppp0".equalsIgnoreCase(netType)) {
			dnsServer = getSysProperties("dhcp.ppp0.dns1");
			Log.d(TAG, "ppp0 dnsServer is: " + dnsServer);
		}
		
		return dnsServer;
	}
	
	public static String getGateWay(Context paramContext) {
		String gateway = "";
		String netType = getNetType(paramContext);
		if(TextUtils.isEmpty(netType)) {
			Log.d(TAG, "netType is null or netType.len less than zero");
			return gateway;
		}
		
		if("eth0".equalsIgnoreCase(netType)) {
			/*WifiManager wifiManager = (WifiManager) paramContext.getSystemService(Context.WIFI_SERVICE);
			DhcpInfo dhcpInfo = wifiManager.getDhcpInfo();
			gateway = Formatter.formatIpAddress(dhcpInfo.gateway);*/
			gateway = getSysProperties("dhcp.eth0.gateway");
			Log.d(TAG, "eth0 gateway is: " + gateway);
		} else if("wlan0".equalsIgnoreCase(netType)) {
			gateway = getSysProperties("dhcp.wlan0.gateway");
			Log.d(TAG, "wlan0 gateway is: " + gateway);		
		} else if("ppp0".equalsIgnoreCase(netType)) {
			gateway = getSysProperties("dhcp.ppp0.gateway");
			Log.d(TAG, "ppp0 gateway is: " + gateway);
		}
		
		return gateway;
	}

	/*--------------------------Added function by James end_part------------------*/
}
