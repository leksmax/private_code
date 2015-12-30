package com.skyworthdigital.tr069;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class CpuInfo {
	private final String TAG = "CpuInfo";
	private Context mContext = null;
	private Handler mHandler = null;
	
	public CpuInfo(Context context, Handler handler) {
		mContext = context;
		mHandler = handler;
	}

	public void getCpuInfos() {

		new Thread() {
			@Override
			public void run() {
				String[] cpuInfos1 = null;
				String load1 = getProcStatFirstLine();
				cpuInfos1 = load1.split(" ");
				String cpuName = cpuInfos1[0];
				String user1 = cpuInfos1[1];
				String nice1 = cpuInfos1[2];
				String system1 = cpuInfos1[3];
				String idle1 = cpuInfos1[4];
				String iowait1 = cpuInfos1[5];
				String irq1 = cpuInfos1[6];
				String softirq1 = cpuInfos1[7];
				
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					Log.d(TAG, "sleep failed");
					e.printStackTrace();
				}

				String[] cpuInfos2 = null;
				String load2 = getProcStatFirstLine();
				cpuInfos2 = load2.split(" ");
				String user2 = cpuInfos2[1];
				String nice2 = cpuInfos2[2];
				String system2 = cpuInfos2[3];
				String idle2 = cpuInfos2[4];
				String iowait2 = cpuInfos2[5];
				String irq2 = cpuInfos2[6];
				String softirq2 = cpuInfos2[7];
				
				long user_pass = Long.parseLong(user2) - Long.parseLong(user1);
				long system_pass = Long.parseLong(system2)
						- Long.parseLong(system1);
				long irq_pass = Long.parseLong(irq2) - Long.parseLong(irq1);
				long idle_pass = Long.parseLong(idle2) - Long.parseLong(idle1);

				long cpuUsage = (user_pass + system_pass + irq_pass) * 100
						/ (user_pass + irq_pass + system_pass + idle_pass);
				Log.d(TAG, "cpuUsage is " + cpuUsage);
				Message msg = new Message();
				msg.what = TR069Params.PROPERTY_SUPERVISE_CPU_USAGE_UPDATE;
				msg.arg1 = (int)cpuUsage;
				mHandler.sendMessage(msg);
			}
		}.start();
	}

	private String getProcStatFirstLine() {
		String str = null;
		try {
			FileInputStream in = new FileInputStream("/proc/stat");
			InputStreamReader inReader = new InputStreamReader(in);
			BufferedReader reader = new BufferedReader(inReader);
			str = reader.readLine();
			reader.close();
		} catch (IOException ex) {
			Log.e(TAG, "IOException" + ex.toString());
			return null;
		}

		str = str.replaceAll("\\s{1,}", " ");
		return str;
	}
}
