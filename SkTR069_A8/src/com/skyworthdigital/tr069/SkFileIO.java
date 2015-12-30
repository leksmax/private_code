package com.skyworthdigital.tr069;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.http.util.EncodingUtils;

import android.content.Context;
import android.util.Log;

public class SkFileIO 
{     
	private static final String TAG = "FileIO";
	private Context context;       
	public SkFileIO(Context c) 
	{         
		this.context = c;     
	}       
	 
	public String readSDCardFile(String path) throws IOException 
	{         
		File file = new File(path);         
		FileInputStream fis = new FileInputStream(file);         
		String result = streamRead(fis);         
		return result;     
	}       
	   
	public String readRawFile(int fileId) throws IOException 
	{              
		InputStream is = context.getResources().openRawResource(fileId);         
		String result = streamRead(is);     
		return result;     
	}       
	private String streamRead(InputStream is) throws IOException 
	{         
		int buffersize = is.available();    
		byte buffer[] = new byte[buffersize];
		is.read(buffer);        
		is.close();
		String result = EncodingUtils.getString(buffer, "UTF-8");   
		return result;     
	}       

	
	public String readAssetsFile(String filename) throws IOException 
	{         
		        
		InputStream is = context.getResources().getAssets().open(filename);    
		String result = streamRead(is);     
		return result;     
		}       
	
	public void writeSDCardFile(String path, byte[] buffer) throws IOException 
	{         
		File file = new File(path);         
		FileOutputStream fos = new FileOutputStream(file);         
		fos.write(buffer);       
		fos.close();     
	}       
	 
	public void writeDateFile(String fileName, byte[] buffer) throws Exception 
	{         
		byte[] buf = fileName.getBytes("iso8859-1");         
		fileName = new String(buf, "utf-8");         
		FileOutputStream fos = context.openFileOutput(fileName,Context.MODE_APPEND);    
		fos.write(buffer);         
		fos.close();     
	}       
	 
	public String readDateFile(String fileName) throws Exception 
	{         
		FileInputStream fis = context.openFileInput(fileName);         
		String result = streamRead(fis);         
		return result;     
	} 
	public boolean isConfigExists(String filename)
	{
		try
		{
			File f=new File(filename);
			if(!f.exists())
			{
				return false;
			}
		}
		catch (Exception e) 
		{
			// TODO: handle exception
			return false;
		}
		return true;
	}	
	
	
	public boolean writeResToDisk(String src_file,String dest_file)  throws Exception 
	{
		try
		{
			Log.i(TAG, "writeResToDisk start");
		//	if(isConfigExists(dest_file)) 
		//		return false;
		
			 
			InputStream fis = context.getResources().getAssets().open(src_file);  
			int buffersize = fis.available();
		
			byte buffer[] = new byte[buffersize];
		    fis.read(buffer);
			fis.close();   
			
			byte[] buf = dest_file.getBytes("iso8859-1");         
			dest_file = new String(buf, "utf-8");         
			FileOutputStream fos = context.openFileOutput(dest_file,Context.MODE_WORLD_READABLE + Context.MODE_WORLD_READABLE);      
			fos.write(buffer);         
			fos.close();  
			
		}
		catch (Exception e) 
		{
			Log.i(TAG, "writeResToDisk error,file="+src_file);
			// TODO: handle exception
			return false;
		}	
		Log.i(TAG, "writeResToDisk success,file="+src_file);
		return true;
	}
	

}
