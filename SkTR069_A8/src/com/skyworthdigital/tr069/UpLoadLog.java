package com.skyworthdigital.tr069;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import android.os.Handler;
import android.util.Log;

public class UpLoadLog {
	private final String TAG = "UpLoadLog";
	private long endTime = 0;
	private DatagramSocket s = null;
	private String mMac = null;
	private String mSoftWareVersion = null;
	public static final int UPLOAD_THREAD_RUNNING = 1000;
	public static final int UPLOAD_THREAD_STOPED = 1001;
	private int state = UPLOAD_THREAD_STOPED;

	public UpLoadLog(String mac, String softWareVersion) {
		if (mac == null || softWareVersion == null) {
			Log.d(TAG, "mac or softWareVersion is null");
			return;
		}
		mMac = mac;
		mSoftWareVersion = softWareVersion;
	}

	public void beginToSendUDPLog(long continueTime, String udpServer) {
		final String[] serverStr;
		endTime = System.currentTimeMillis() + continueTime * 1000;
		serverStr = parseUdpServerUrl(udpServer);

		try {
			if (s == null)
				s = new DatagramSocket();
			else {
				// Toast.makeText(this, "send task is running!", 1).show();
				return;
			}
		} catch (SocketException e) {
			e.printStackTrace();
		}

		new Thread(new Runnable() {
			@Override
			public void run() {
				Process logcatProcess = null;
				BufferedReader bufferedReader = null;
				setState(UPLOAD_THREAD_RUNNING);
				String message;
				try {
					/** 获取系统logcat日志信息 */

					// 相当于在命令行运行 logcat -s TAG

					String[] logcatClear = new String[] { "logcat", "-c" };
					logcatProcess = Runtime.getRuntime().exec(logcatClear);
					String[] running = new String[] { "logcat" };
					logcatProcess = Runtime.getRuntime().exec(running);

					bufferedReader = new BufferedReader(new InputStreamReader(
							logcatProcess.getInputStream()));

					String line;
					while ((line = bufferedReader.readLine()) != null) {
						message = buildUDPMessage(line);
						sendToUDPServer(serverStr, message);
						Thread.sleep(100);
						long nowTime = System.currentTimeMillis();

						if (nowTime >= endTime) {
							s.close();
							s = null;
							setState(UPLOAD_THREAD_STOPED);
							Log.d(TAG, "syslog send finish!");
							break;
						}

					}

				} catch (Exception e) {

					e.printStackTrace();
				}
			}
		}).start();

	}

	private String[] parseUdpServerUrl(String url) {
		// url =192.168.20.115:6666
		String[] udpServerArray = null;
		udpServerArray = url.split(":");
		return udpServerArray;

	}

	private String buildUDPMessage(String text) {
		String buffer, str, sendTextStr = "<143>";
		String[] dateArray = null;
		BufferedReader bufferedReader = null;

		try {
			String[] running = new String[] { "date" }; // Thu Jan 22 10:36:04
														// CST 2015
			Process dateCmdResult = Runtime.getRuntime().exec(running);
			bufferedReader = new BufferedReader(new InputStreamReader(
					dateCmdResult.getInputStream()));
			if ((str = bufferedReader.readLine()) != null) {
				dateArray = str.split(" ");
			}

		} catch (Exception e) {

			e.printStackTrace();
		}
		sendTextStr = sendTextStr + dateArray[1] + " " + dateArray[2] + " "
				+ dateArray[3] + " ";
		// <143>Jun 10 17:00:20 00:11:09:ef:b5:af EC1308V100R001C02B021
		// 202.173.12.88 connect rtsp://202.173.4.88/mov/test.ts timeout
		sendTextStr = sendTextStr + mMac + " ";
		sendTextStr = sendTextStr + mSoftWareVersion + " ";
		// sendTextStr = sendTextStr + "10.2.2.2"+ " ";
		sendTextStr = sendTextStr + text;
		sendTextStr = sendTextStr + "\n";
		return sendTextStr;
	}

	private void sendToUDPServer(String[] serverStr, String message) {

		message = (message == null ? "STB has no log to send!" : message);
		InetAddress local = null;
		try {
			local = InetAddress.getByName(serverStr[0]);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}
		int msg_length = message.length();
		byte[] messageByte = message.getBytes();
		DatagramPacket p = new DatagramPacket(messageByte, msg_length, local,
				Integer.parseInt(serverStr[1]));
		try {
			s.send(p);

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public int getState() {
		return state;
	}

	public void setState(int state) {
		this.state = state;
	}

	public long getEndTime() {
		return endTime;
	}

	public void setEndTime(long continueTime) {
		this.endTime = System.currentTimeMillis() + continueTime * 1000;
	}
	
}
