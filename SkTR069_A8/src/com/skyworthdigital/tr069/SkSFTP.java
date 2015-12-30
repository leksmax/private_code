package com.skyworthdigital.tr069;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;
import java.util.Vector;

import android.content.Context;
import android.util.Log;

import com.jcraft.jsch.Channel;
import com.jcraft.jsch.ChannelSftp;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;
import com.jcraft.jsch.SftpException;

/**
 * @author YangHua ת����ע��������http://www.xfok.net/2009/10/124485.html
 */
public class SkSFTP {

	/**
	 * ����sftp������
	 * 
	 * @param host
	 *            ����
	 * @param port
	 *            �˿�
	 * @param username
	 *            �û���
	 * @param password
	 *            ����
	 * @return
	 */
	private static final String PARAM_TAG = "SK_TR069_SFTP";
	
	public ChannelSftp connect(String host, int port, String username,
			String password) {
		ChannelSftp sftp = null;
		try {
			JSch jsch = new JSch();
			Session sshSession = jsch.getSession(username, host, port);
			Log.d(PARAM_TAG, "Session created.");
			sshSession.setPassword(password);
			Properties sshConfig = new Properties();
			sshConfig.put("StrictHostKeyChecking", "no");
			sshSession.setConfig(sshConfig);
			sshSession.connect();
			Log.d(PARAM_TAG, "Session connected.");
			System.out.println("Opening Channel.");
			Channel channel = sshSession.openChannel("sftp");
			channel.connect();
			sftp = (ChannelSftp) channel;
			Log.d(PARAM_TAG,"Connected to " + host + ".");
		} catch (Exception e) {

		}
		return sftp;
	}

	
	public void disconnect(ChannelSftp sftp)
	{
		if(null != sftp)
		{
			 sftp.disconnect();
			 sftp.exit();
		}
	}
	/**
	 * �ϴ��ļ�
	 * 
	 * @param directory
	 *            �ϴ���Ŀ¼
	 * @param uploadFile
	 *            Ҫ�ϴ����ļ�
	 * @param sftp
	 */
	public void upload(String directory, String uploadFile, ChannelSftp sftp) {
		try {
			
			sftp.cd(directory);
			File file = new File(uploadFile);
			
			TR069Params mInstance = TR069Params.getInstance();
			String mac = mInstance.getParam(TR069Params.PARAM_NAME_TR069_MAC);
	    	mac = mac.toLowerCase();
	    	mac = mac.replaceAll(":", "");
	    	
	    	SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");//�������ڸ�ʽ
	    	String time = df.format(new Date());// new Date()Ϊ��ȡ��ǰϵͳʱ��
	    	String dst_file_name = mac+"_"+time+".pcap";
	    	if(dst_file_name.isEmpty()){
	    		dst_file_name = file.getName();
	    	}
	    	
			sftp.put(new FileInputStream(file), dst_file_name);
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * �����ļ�
	 * 
	 * @param directory
	 *            ����Ŀ¼
	 * @param downloadFile
	 *            ���ص��ļ�
	 * @param saveFile
	 *            ���ڱ��ص�·��
	 * @param sftp
	 */
	public void download(String directory, String downloadFile,
			String saveFile, ChannelSftp sftp) {
		try {
			sftp.cd(directory);
			File file = new File(saveFile);
			sftp.get(downloadFile, new FileOutputStream(file));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * ɾ���ļ�
	 * 
	 * @param directory
	 *            Ҫɾ���ļ�����Ŀ¼
	 * @param deleteFile
	 *            Ҫɾ�����ļ�
	 * @param sftp
	 */
	public void delete(String directory, String deleteFile, ChannelSftp sftp) {
		try {
			sftp.cd(directory);
			sftp.rm(deleteFile);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * �г�Ŀ¼�µ��ļ�
	 * 
	 * @param directory
	 *            Ҫ�г���Ŀ¼
	 * @param sftp
	 * @return
	 * @throws SftpException
	 */
	public Vector listFiles(String directory, ChannelSftp sftp)
			throws SftpException {
		return sftp.ls(directory);
	}

	/*
	public static void main(String[] args) {
		MySFTP sf = new MySFTP();
		String host = "192.168.65.92";
		int port = 22;
		String username = "debug";
		String password = "123456";
		String directory = "/";
		//String uploadFile = "D:\\tmp\\upload.txt";
		//String downloadFile = "upload.txt";
		//String saveFile = "D:\\tmp\\download.txt";
		//String deleteFile = "delete.txt";
		ChannelSftp sftp = sf.connect(host, port, username, password);
		//sf.upload(directory, uploadFile, sftp);
		//sf.download(directory, downloadFile, saveFile, sftp);
		//sf.delete(directory, deleteFile, sftp);
		try {
			sftp.cd(directory);
			sftp.mkdir("ss");
			System.out.println("finished");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	*/
}