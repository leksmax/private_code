package com.skyworthdigital.tr069;

import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.util.Base64;

public class GetAuthUrl {
	private static final String TU_SERVER = "120.197.230.162:8082";
	
	public  String getUrl(Context mContext){
		String model = "MiPT" + android.os.Build.MODEL;
		String url = "http://" + TU_SERVER + "/EDS/itv/launcherLogin/" + model + "/";		// stbType
		
		// headers
		Map<String, String> hdrs = new HashMap<String, String>();
		// username
		hdrs.put("Itv-userName", "66666");
		// password, 1.md5; 2.base64
		String password;
		try {
			password = encodePassword("2013");
			hdrs.put("Itv-passWord", password);
			// device id
			hdrs.put("Itv-devid", Utils.getDeviceId(mContext, Utils.deviceType(mContext)));
			return url;
		} catch (NoSuchAlgorithmException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return "";
	}
	
	private String encodePassword(String password) throws NoSuchAlgorithmException {
//      MessageDigest md5 = MessageDigest.getInstance("MD5");
//      md5.update(password.getBytes());
//      byte[] md5Bytes = md5.digest();
//      
//      StringBuffer hexValue = new StringBuffer(); 
//      for (int i = 0; i < md5Bytes.length; i++) { 
//      	int val = ((int) md5Bytes[i]) & 0xff; 
//      	if (val < 16) 
//      		hexValue.append("0"); 
//      	hexValue.append(Integer.toHexString(val)); 
//      } 
//      
//      String newstr = Base64.encodeToString(hexValue.toString().getBytes(), 
//      		Base64.NO_PADDING | Base64.NO_WRAP);
		String hexValue = CodeUtils.md5Hex(password);
	    String newstr = Base64.encodeToString(hexValue.getBytes(), 
	    		Base64.NO_PADDING | Base64.NO_WRAP);
	    
      return newstr;
	}
}
