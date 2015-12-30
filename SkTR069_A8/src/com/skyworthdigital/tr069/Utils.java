/*
 * Utils.java
 * @date 2012-7-9
 *
 * Copyright (c) Mipt. All rights reserved.
 */
package com.skyworthdigital.tr069;

/*import java.io.FileReader;*/
import java.io.BufferedReader;
import java.io.FileReader;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import org.xmlpull.v1.XmlPullParser;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.preference.PreferenceManager;
import android.text.TextUtils;
import android.util.Log;
/*
import com.mipt.ott.usercenter.constant.ValueConst;
import com.mipt.ott.usercenter.db.FuntionEntrys;
import com.mipt.ott.usercenter.network.TaskEngine;
import com.mipt.ott.usercenter.task.FuntionEntryTask;
*/
/**
 * @author Jack
 * @version 1.00
 * @date 2012-7-9
 * @since 1.6
 */
public class Utils {
	private static final String TAG = "Utils";
	
	private static final byte[] sEntryLock = new byte[0];
	
	public static final String A4_INFO_FILE_DEVICE_ID = "/private/.id.txt";
	public static final String A4_INFO_FILE_SN = "/private/.sn.txt";
	public static final String A4_INFO_FILE_MACETH = "/private/.maceth.txt";
	public static final String A4_INFO_FILE_MAC = "/private/.mac.txt";
	
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
	
	public static String sDeviceId = null;
	
	public static String HOST_SKYWORTH = "ro.stb.user.url";
	
	private final static String R_FILE_MAC = "/sys/class/net/eth0/address";

	
	/**
	 * @param context
	 * @return
	 */
	public static final int deviceType(Context context) {
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
		int deviceType = sp.getInt("device_type", 0);
		
		if (deviceType < 2) {				// need load device type from system
			
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
					
				} else if ("SkyworthDigitalRT".equalsIgnoreCase(android.os.Build.MANUFACTURER) ||
						"SkyworthDigital".equalsIgnoreCase(android.os.Build.MANUFACTURER) ||
						"HAO".equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
					deviceType = checkSkyworthDevice();
					
				} else if ("BestTech".equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
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
		
		//Log.d(TAG, "::TD > " + deviceType);
		return deviceType;
	}
	
	/**
	 * Gets the device's ID.
	 * 
	 * @param ctx
	 * @return Device id
	 * @date 2012-7-11
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
				sDeviceId = getFileInfo(A4_INFO_FILE_DEVICE_ID);
				break;
				
			case DEVICE_MIPT_IKAN_A8:
				sDeviceId = getFileInfo("/sys/class/mipt_hwconfig/deviceid");
				break;
				
			case DEVICE_MIPT_IKAN_A5:
			case DEVICE_MIPT_IKAN_R6:
				sDeviceId = getFileInfo("/hwcfg/.id.txt");
				break;

			default:
				break;
			}
			/*
			if (ValueConst.DEBUG) {
				Log.i(TAG, "DID: " + sDeviceId);			
			}
			*/
		}
		
		return sDeviceId;
	}
	
	public static final String getSoftwareVersion(int deviceType) {
		switch(deviceType){
		case DEVICE_MIPT_IKAN_R6:
			return getSysProperties("ro.product.version");
		default:
			return getSysProperties("persist.sys.hwconfig.soft_ver");
		}
		
	}
	
	public static final String getHardwareVersion(int deviceType) {
		switch(deviceType){
		case DEVICE_MIPT_IKAN_R6:
			return getSysProperties("ro.product.hw_version");
		default:
			return getSysProperties("persist.sys.hwconfig.hw_ver");
		}
	}
	
	/**
	 * Return the SN.
	 * 
	 * @param deviceType
	 * @return
	 */
	public static final String getSerialNumber(int deviceType) {
		switch (deviceType) {
		case DEVICE_MIPT_IKAN_A6:
		case DEVICE_MIPT_BETV_U6:
		case DEVICE_SKYWORTH_NEXT_PANDORA:
		case DEVICE_SKYWORTH_NORMAL:
		case DEVICE_SKYWORTH_HAO:
		case DEVICE_SKYWORTH_HSM1:
		case DEVICE_MIPT_IKAN_TEST:
		case DEVICE_MIPT_BETV_TEST:
			return getSysProperties("persist.sys.hwconfig.seq_id");
			
		case DEVICE_MIPT_IKAN_A4:
		case DEVICE_BST_SNT_B10:
			return getFileInfo(A4_INFO_FILE_SN);
			
		case DEVICE_MIPT_IKAN_A8:
			return getFileInfo("/sys/class/mipt_hwconfig/serialid");
			
		case DEVICE_MIPT_IKAN_A5:
		case DEVICE_MIPT_IKAN_R6:
			return getFileInfo("/hwcfg/.sn.txt");

		default:
			return null;
		}
	}
	
	/**
	 * @return
	 */
	public static final String getSysProperties(String name) {
		String id = null;

		try {
			Class c = Class.forName("android.os.SystemProperties");
			Method m = c.getMethod("get", new Class[] {String.class});
			id = (String) m.invoke(c, new Object[] {name});
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
		
		return id;
	}

	/**
	 * @param ctx
	 * @date 2012-7-10
	 */
	/*
	public static final void downloadFunctionEntry(Context ctx) {
		synchronized (sEntryLock) {
			try {
				SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(ctx);
				long lastGetEntryTime = sp.getLong(
						ValueConst.PF_LAST_GET_FUNCTIONENTRY_TIME, 0);
				long now = System.currentTimeMillis();
				
				if (lastGetEntryTime <= 0
						|| Math.abs(now - lastGetEntryTime) > ValueConst.FUNCTION_OUT_OF_DATE) {
					// save status
					SharedPreferences.Editor editor = sp.edit();
					editor.putLong(ValueConst.PF_LAST_GET_FUNCTIONENTRY_TIME, now);
					Date date = new Date(now);
					editor.putString(ValueConst.PF_LAST_GET_FUNCTIONENTRY_RECORD, date.toString());
					editor.commit();

					// start to down-load function entry
					FuntionEntryTask task = new FuntionEntryTask(ctx, null);
					TaskEngine.getInstance().addTaskTail(task);
					
					Log.w(TAG, "::Need get entry::");
				}
			} catch (Throwable th) {
				Log.e(TAG, "Error: ", th);
			}
		}

	}
    */
	/**
	 * This method should not be called in main thread.
	 * 
	 * @param ctx
	 * @param title
	 * @return
	 * @date 2012-7-10
	 */
	/*public static final String getEntry(Context ctx, String title) {
		String entry = null;
		Cursor c = null;

		try {
			Uri uri = Uri.withAppendedPath(FuntionEntrys.CONTENT_URI, title);
			c = ctx.getContentResolver().query(uri, new String[] {FuntionEntrys.ENTRY}, 
					null, null, null);
			if (c != null && c.moveToFirst()) {
				int id = c.getColumnIndexOrThrow(FuntionEntrys.ENTRY);
				entry = c.getString(id);
			}
			
		} catch (Exception e) {
			Log.e(TAG, "Get Entry error: " + title, e);
		} finally {
			if (c != null) {
				try {
					c.close();
				} catch (Exception e) {
				}
			}
		}
		
		return entry;
	}*/
	
	/**
	 * Returns the current activate account.
	 * 
	 * @param ctx
	 * @return
	 * @date 2012-7-11
	 */
	/*
	public static final MobeeAccount curAccount(Context ctx) {
		MobeeAccount account = null;
		
		// get passport
        Uri uri = Uri.parse("content://com.mipt.ott.provider.Usercenter/activateUser");
        Cursor cs = null;
        try {
        	cs = ctx.getContentResolver().query(uri, null, null, null, null);
            if (cs != null && cs.moveToFirst()) {
            	account = new MobeeAccount();
            	account.activate = true;
            	
            	int id = cs.getColumnIndexOrThrow("passport");
            	account.passport = cs.getString(id);
            	id = cs.getColumnIndexOrThrow("account_id");
            	account.accountId = cs.getString(id);
            	id = cs.getColumnIndexOrThrow("password");
            	account.password = cs.getString(id);
            }
        } catch (Exception e) {
        	Log.e(TAG, "None passport !", e);
        } finally {
        	if (cs != null) {
        		try {
        			cs.close();
        		} catch (Exception e) {}
        	}
        }
        
        return account;
	}
	*/
	/**
	 * Returns the current active account's passport.
	 * 
	 * @param ctx
	 * @return
	 * @date 2012-7-11
	 */
	public static final String getPassport(Context ctx) {
		String passport = null;
		
		// get passport
        Uri uri = Uri.parse("content://com.mipt.ott.provider.Usercenter/activateUser");
        Cursor cs = null;
        try {
        	cs = ctx.getContentResolver().query(uri, null, null, null, null);
            if (cs != null && cs.moveToFirst()) {
            	int id = cs.getColumnIndexOrThrow("passport");
            	passport = cs.getString(id);
            }
        } catch (Exception e) {
        	Log.e(TAG, "None passport !", e);
        } finally {
        	if (cs != null) {
        		try {
        			cs.close();
        		} catch (Exception e) {}
        	}
        }
        
        return passport;
	}
	
	/**
	 * Sets the passport of a special account.
	 * 
	 * @param ctx
	 * @param accountId
	 * @return
	 * @date 2012-7-11
	 */
	public static final boolean setPassport(Context ctx, String accountId,
			String newPassport) {
		Uri uri = Uri.parse("content://com.mipt.ott.provider.Usercenter/users/"
        		+ accountId);
        try {
        	ContentValues updateCv = new ContentValues();
        	updateCv.put("passport", newPassport);
        	int count = ctx.getContentResolver().update(uri, updateCv, null, null);
        	if (count > 0) {
        		return true;
        	}
        } catch (Exception e) {
        	Log.e(TAG, "Update passport error!", e);
        }
        
        return false;
	}

	/**
	 * @param parser
	 * @throws Exception
	 * @date 2012-7-9
	 */
	public static final void skipXmlSubTree(XmlPullParser parser)
			throws Exception {
		parser.require(XmlPullParser.START_TAG, null, null);
		int level = 1;
		while (level > 0) {
			int eventType = parser.next();
			if (eventType == XmlPullParser.END_TAG) {
				--level;
			} else if (eventType == XmlPullParser.START_TAG) {
				++level;
			} else if (eventType == XmlPullParser.END_DOCUMENT) {
				break;
			}
		}
	}
	
	/**
	 * Sends passport has been updated.
	 * 
	 * @param context
	 */
	public static final void sendPassprotUpdateBroadcast(Context context) {
		Intent it = new Intent("mipt.ott.uc.intent.action.PASSPORT_UPDATED");
		it.setFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
		context.sendBroadcast(it);
	}
	
	/**
	 * Current area information.
	 * 
	 * @param cr
	 * @return
	 */
	public static final String curArea(ContentResolver cr) {
		StringBuilder sb = new StringBuilder();
		sb.append(getSettingString(cr, "ott_location", "country_en_name"));
		sb.append("+");
		sb.append(getSettingString(cr, "ott_location", "province_en_name"));
		sb.append("+");
		sb.append(getSettingString(cr, "ott_location", "city_en_name"));
		
		return sb.toString();
	}
	
	/**
     * Convenience function for retrieving a single system settings value.
     *
     * @param cr The ContentResolver to access.
     * @param name the name of the conf
     * @return String The setting's current value, or null if it is not
     *         defined.
     */
    public static final String getSettingString(ContentResolver cr, String group, String name) {
    	Uri uri = Uri.parse("content://mipt.ott_setting/conf");
        String res = null;
        
        if (cr != null && !TextUtils.isEmpty(group) && !TextUtils.isEmpty(name)) {
        	Cursor c = null;
            String[] projection = { "value" };
            try {
                c = cr.query(uri, projection, 
                		"confgroup=\"" + group + "\" and name=\"" + name + "\"",
                		null, null);
                if (c != null && c.moveToFirst()) {
                    res = c.getString(c.getColumnIndexOrThrow("value"));
                }
            } catch (Exception e) {
            	Log.e(TAG, "Exception:", e);
            } finally {
            	if(null != c) {
            		try{ c.close(); } catch(Exception ex){};
            	}
            }
        }

        if (TextUtils.isEmpty(res)) {
        	res = "Unknown";
        }
        return res;
    }
    
    /**
     * @param filename
     * @return
     */
    public static final String getFileInfo(String filename) {
    	if (filename == null || filename.length() < 1) {
    		return null;
    	}
    	
    	String info = null;
    	
    	FileReader fr = null;
    	try {
    		fr = new FileReader(filename);
    		
    		StringBuilder sb = new StringBuilder();
    		int ch = 0;
    		while((ch = fr.read()) != -1) {
    			if ('\n' == ch || '\r' == ch) {
    				continue;
    			}
    			
    			sb.append((char) ch);
    		}
    		
    		info = sb.toString();
    	} catch (Exception e) {
    		Log.e(TAG, "error", e);
    	} finally {
    		if (fr != null) {
    			try {
    				fr.close();
    			} catch (Exception e) {}
    		}
    	}
    	
    	return info;
    }
    
    /**
	 * @return
	 */
	private static final int checkMiptDevice() {
		int deviceType = 0;
		
		if ("i.Kan".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("A6".equalsIgnoreCase(android.os.Build.MODEL) ||
					"A3".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_A6;			// A6, A3
			} else if("A4".equalsIgnoreCase(android.os.Build.MODEL) ||
					"A400".equalsIgnoreCase(android.os.Build.MODEL) ||
					"A401".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_A4;			// A4
			} else if ("A8".equalsIgnoreCase(android.os.Build.MODEL) ||
					"A800".equalsIgnoreCase(android.os.Build.MODEL) ||
					"A800D".equalsIgnoreCase(android.os.Build.MODEL)) {		// A8
				deviceType = DEVICE_MIPT_IKAN_A8;
			}
			else if ("test".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_IKAN_TEST;
			}
		} else if ("BeTV".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("BeTV-U6".equalsIgnoreCase(android.os.Build.MODEL) ||
					"BeTV-U8".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_BETV_U6;			// BeTV-U6
			} else if ("test".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_MIPT_BETV_TEST;
			}
		}
		
		return deviceType;
	}
	
	private static final int checkSkyworthDevice() {
		int deviceType = 0;
		
		if ("NEXT".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("pandorativibu".equalsIgnoreCase(android.os.Build.MODEL) || 
					"pandora".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_SKYWORTH_NEXT_PANDORA;
			}
		} else if ("Skyworth".equalsIgnoreCase(android.os.Build.BRAND)) {
			if(android.os.Build.MODEL != null && (android.os.Build.MODEL.startsWith("HSM") ||
					android.os.Build.MODEL.startsWith("hsm"))){
				deviceType = DEVICE_SKYWORTH_HSM1;
			}
		} else if ("HAO".equalsIgnoreCase(android.os.Build.BRAND)) {
			if("HA2800".equalsIgnoreCase(android.os.Build.MODEL) ||
					"HAO2".equalsIgnoreCase(android.os.Build.MODEL)){
				deviceType = DEVICE_SKYWORTH_HAO;
			}
		}
		
		return deviceType;
	}
	
	private static final int checkBstDevice() {
		int deviceType = 0;
		
		if ("SNT".equalsIgnoreCase(android.os.Build.BRAND)) {
			if ("SNT-T01".equalsIgnoreCase(android.os.Build.MODEL) ||
					"Shinco F90".equalsIgnoreCase(android.os.Build.MODEL) ||
					"SNT-B11".equalsIgnoreCase(android.os.Build.MODEL) ||
					"AD201".equalsIgnoreCase(android.os.Build.MODEL) ||
					"AD202".equalsIgnoreCase(android.os.Build.MODEL) ||
					"SNT-B09B".equalsIgnoreCase(android.os.Build.MODEL) ||
					"SNT-B09".equalsIgnoreCase(android.os.Build.MODEL)) {
				deviceType = DEVICE_BST_SNT_B10;
			}
		}
		
		return deviceType;
	}
	
	public static boolean getActivateStatus(Context ctx){
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(ctx);
		return sp.getBoolean("isActivate", false);
	}
	
	public synchronized static boolean setActivateStatus(Context ctx,boolean isActivate){
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(ctx);
		return sp.edit().putBoolean("isActivate", isActivate).commit();
	}
	
	public static String readFileData(String fileName) {
		BufferedReader reader = null;
		try {
			StringBuffer fileData = new StringBuffer(1000);
			reader = new BufferedReader(new FileReader(fileName));
			char[] buf = new char[1024];
			int numRead = 0;
			while ((numRead = reader.read(buf)) != -1) {
				String readData = String.valueOf(buf, 0, numRead);
				fileData.append(readData);
			}
			reader.close();
			return fileData.toString();
		} catch (Throwable e) {
			Log.e(TAG, "" + (e == null ? "" : e.getMessage()));
		}
		if (null != reader)
			try {
				reader.close();
				reader = null;
			} catch (Throwable t) {
			}
		return "";

	}

	
	public static String getMac() {
		String mac = null;
		try {
			mac = readFileData(R_FILE_MAC);
		} catch (Exception e) {
			Log.e(TAG, "" + (e == null ? "" : e.getMessage()));
		}
		return mac;
	}
	
	public static  String  getdeviceName(Context context) {
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
					
		// new way to identify a device
		String platformName = getSysProperties("mipt.ott.platform.name");		
		
		
		//Log.d(TAG, "::TD > " + deviceType);
		return platformName;
	}
	

}
