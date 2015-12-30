package com.mipt.tr069.tool;

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;


public class Account {
	private static final String TAG = "ServeiceAccount";
	public static final String CONTENT_PATH = "cmAccount";
	public static final String AUTHORITY = "com.mipt.gdcm.account.provider.Usercenter";
	/**
     * The content:// style URL for this table
     */
    public static final Uri CONTENT_URI = Uri.parse("content://" 
    		+ AUTHORITY + "/" + CONTENT_PATH);
	
    public static final String ACCOUNT_ID = "account_id";	// a email
	public static final String PASSWORD = "password";
	public static final String USERCP_ID = "usercp_id";
	
	/**
	 * "content://com.mipt.gdcm.account.provider.Usercenter/cmAccount"
	 * contentResolver.query("content://com.mipt.gdcm.account.provider.Usercenter/cmAccount", null, "name=?", new String[] {“account_id”}, null);
	 * 
	 * @param name
	 * @param contentResolver
	 * @return
	 */
	public static String getValue(String name, ContentResolver contentResolver) {
		String value = "";
		Cursor cs = null;
		try {
			cs = contentResolver.query(Account.CONTENT_URI, null, 
					"name=?", new String[] {name}, null);
			if (cs != null && cs.moveToFirst()) {
				value = cs.getString(cs.getColumnIndexOrThrow("value"));
			}
		} catch (Exception e) {
			Log.e(TAG, "Error", e);
		} finally { 
			if(cs != null)
				cs.close();
		}
		
		return value;
	}
}
