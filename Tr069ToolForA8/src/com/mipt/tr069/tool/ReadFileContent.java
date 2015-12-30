package com.mipt.tr069.tool;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

import android.util.Log;

public class ReadFileContent {
	/**
	 * 
	 * @param filePath
	 * @return
	 */
	public static String getFileContent(String filePath) {
		try {
			StringBuffer fileData = new StringBuffer(1000);
			BufferedReader reader = new BufferedReader(new FileReader(filePath));
			char[] buf = new char[1024];
			int numRead = 0;
			while ((numRead = reader.read(buf)) != -1) {
				String readData = String.valueOf(buf, 0, numRead);
				if (null != readData && readData.length() > 0) {
					fileData.append(readData);
				} else {
					Log.e("getFileContent",
							"readData is null or readData.length less than zero !");
				}
			}
			reader.close();
			return fileData.toString();
		} catch (IOException e) {
			// e.printStackTrace();
			Log.e("NetworkStateApi", "ex : " + e.toString());
			return "";
		}
	}
}
