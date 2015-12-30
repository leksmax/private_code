package com.skyworthdigital.tr069;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;

/**
 * @author Jack
 *
 */
public class CMAccount {
	private static final String TAG = "CMAccount";
	
	public static final String CONTENT_PATH = "cmAccount";
	
	public static final String AUTHORITY = "com.mipt.gdcm.account.provider.Usercenter";
	
	/**
     * The content:// style URL for this table
     */
    public static final Uri CONTENT_URI = Uri.parse("content://" 
    		+ AUTHORITY + "/" + CONTENT_PATH);

    /**
     * The MIME type of {@link #CONTENT_URI} providing a directory of notes.
     */
    public static final String CONTENT_TYPE = 
    	"vnd.android.cursor.dir/vnd.mipt.gdcm.account";

    /**
     * The MIME type of a {@link #CONTENT_URI} sub-directory of a single note.
     */
    public static final String CONTENT_ITEM_TYPE = 
    	"vnd.android.cursor.item/vnd.mipt.gdcm.account";
    
	public static final String ACCOUNT_ID = "account_id";	// a email
	public static final String PASSWORD = "password";
	public static final String USERCP_ID = "usercp_id";
	public static final String INFO1 = "info1";
	public static final String INFO2 = "info2";
	public static final String INFO3 = "info3";
	public static final String INFO4 = "info4";
	


	/**
	 * 
	 */
	public CMAccount() {
	}
	
	/**
	 * "content://com.mipt.gdcm.account.provider.Usercenter/cmAccount"
	 * contentResolver.query("content://com.mipt.gdcm.account.provider.Usercenter/cmAccount", null, "name=?", new String[] {��ccount_id��, null);
	 * 
	 * @param name
	 * @param contentResolver
	 * @return
	 */
	public static String getValue(String name, ContentResolver contentResolver) {
		String value = null;
		
		Cursor cs = null;
		try {
			cs = contentResolver.query(CMAccount.CONTENT_URI, null, 
					"name=?", new String[] {name}, null);
			if (cs != null && cs.moveToFirst()) {
				value = cs.getString(cs.getColumnIndexOrThrow("value"));
//				if (ValueConst.DEBUG) Log.d(TAG, name + ":" + value);
			}
		} catch (Exception e) {
			Log.e(TAG, "Error", e);
		} finally { 
			if(cs != null)
				cs.close();
		}
		
		return value;
	}

	/**
	 * Saves value into database;
	 * @param name
	 * @param value
	 */
	public static void saveValue(String name, String value, ContentResolver contentResolver) {
		Cursor cs = null;
		try {
			cs = contentResolver.query(CMAccount.CONTENT_URI, null, 
					"name=?", new String[] {name}, null);
			
			if (cs != null && cs.getCount() > 0) {
				// update
				ContentValues values = new ContentValues();
				values.put("value", value);
				int count = contentResolver.update(CMAccount.CONTENT_URI, values, "name=?", new String[] {name});
//				if (ValueConst.DEBUG) Log.d(TAG, "update value: " + count + ", " + name + ":" + value);
			} else {
				// insert
				ContentValues values = new ContentValues();
				values.put("name", name);
				values.put("value", value);
				contentResolver.insert(CMAccount.CONTENT_URI, values);
//				if (ValueConst.DEBUG) Log.d(TAG, "insert value: name=" + name + "; value=" + value);
			}
		} catch (Exception e) {
			Log.e(TAG, "Error", e);
		} finally {
			if(cs != null)
				cs.close();
		}
	}
}
