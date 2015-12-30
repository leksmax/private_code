package com.skyworthdigital.tr069;


import java.io.File;
import java.io.InputStream;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.PreferenceActivity;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

public class TR069Setting extends PreferenceActivity implements OnPreferenceChangeListener{
	private EditTextPreference mEditHeartBeat;
	private EditTextPreference mEditACS;
	private EditTextPreference mEditUsername;
	private EditTextPreference mEditPassword;
	private EditTextPreference mEditCPEUser;
	private EditTextPreference mEditCPEPass;
	
	private Button mTestButton;
	
	private TR069Params mParam;
	private static final String TAG = "TR069Setting";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        
		write_res_to_disk("config/", "tr069_model.xml");
		
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
        		WindowManager.LayoutParams.FLAG_FULLSCREEN);
        
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
		addPreferencesFromResource(R.xml.setting);
		setContentView(R.layout.preferencescreenlayout);
		
        getListView().setSelector(getResources().getDrawable(R.drawable.preference_item_bg));
        getListView().setCacheColorHint(Color.TRANSPARENT);
        getListView().setDivider(null);
		findControls();
		initControls();
		
		//modify by lijingchao
		
		
		/*Log.d("zkl","onCreate TR069_PARAM_TYPE_SEL NewDownloadService");
		Intent intent = new Intent(this, NewDownloadService.class);
	    startService(intent);*/

		Intent intent = new Intent("com.mipt.tr069.start.downloadservice");
		Log.d("TR069Setting", "mContext.sendBroadcast -->com.mipt.tr069.start.downloadservice");
		sendBroadcast(intent);
		
		mParam.setConfigPath(getFilesDir().getParent() + "/files/");
		
	}
	
	@Override
	protected void onDestroy() {	
		super.onDestroy();
	}

	private void findControls(){
		mParam = TR069Params.getInstance();
		
		mEditHeartBeat = (EditTextPreference) findPreference("setting_heartbeat");
		mEditACS = (EditTextPreference) findPreference("setting_acs");
		mEditUsername = (EditTextPreference) findPreference("setting_username");
		mEditPassword = (EditTextPreference) findPreference("setting_password");
		mEditCPEUser = (EditTextPreference) findPreference("setting_cpeuser");
		mEditCPEPass = (EditTextPreference)findPreference("setting_cpepass");
		mTestButton = (Button)findViewById(R.id.test);
		mTestButton.setOnClickListener(new Button.OnClickListener(){
			@Override
			public void onClick(View view) {
				mParam.doNativeTest();
			}
		});
	}
	
	private void initControls(){
		setPerferenceValue(mEditHeartBeat, mParam.getParam(TR069Params.PARAM_NAME_TR069_HEARTBEAT));
		setPerferenceValue(mEditACS, mParam.getParam(TR069Params.PARAM_NAME_TR069_ACS));
		setPerferenceValue(mEditUsername, mParam.getParam(TR069Params.PARAM_NAME_TR069_USERNAME));
		setPerferenceValue(mEditPassword, mParam.getParam(TR069Params.PARAM_NAME_TR069_PASSWORD));
		setPerferenceValue(mEditCPEUser, mParam.getParam(TR069Params.PARAM_NAME_TR069_CPEUSER));
		setPerferenceValue(mEditCPEPass, mParam.getParam(TR069Params.PARAM_NAME_TR069_CPEPASS));
		
		mEditHeartBeat.setOnPreferenceChangeListener(this);
		mEditACS.setOnPreferenceChangeListener(this);
		mEditUsername.setOnPreferenceChangeListener(this);
		mEditPassword.setOnPreferenceChangeListener(this);
		mEditCPEUser.setOnPreferenceChangeListener(this);
		mEditCPEPass.setOnPreferenceChangeListener(this);
	}
	
	private void setPerferenceValue(EditTextPreference perference, String value){
		if(perference != mEditPassword && perference != mEditCPEPass){
			perference.setSummary(value);
		}
		
		perference.setText(value);
	}

	public boolean onPreferenceChange(Preference preference, Object newValue) {
		String value = newValue.toString();
		if (preference.equals(mEditHeartBeat)) {
			mEditHeartBeat.setSummary(value);
			mEditHeartBeat.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_HEARTBEAT, value);
		} else if (preference.equals(mEditACS)) {
			mEditACS.setSummary(value);
			mEditACS.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_ACS, value);
		} else if (preference.equals(mEditUsername)) {
			mEditUsername.setSummary(value);
			mEditUsername.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_USERNAME, value);
		} else if (preference.equals(mEditPassword)) {
			//mEditPassword.setSummary(value);
			mEditPassword.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_PASSWORD, value);
		} else if (preference.equals(mEditCPEUser)){
			mEditCPEUser.setSummary(value);
			mEditCPEUser.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_CPEUSER, value);
		} else if (preference.equals(mEditCPEPass)){
			mEditCPEPass.setSummary(value);
			mEditCPEPass.setText(value);
			mParam.setParam(TR069Params.PARAM_NAME_TR069_CPEPASS, value);
		}
		return true;
	}
	
	public void write_res_to_disk(String src_path, String src_file)
	{
		String srcfile = src_path + src_file;
		String destfile = getFilesDir().getParent() + "/files/" + src_file;
		
		try
		{
			SkFileIO fileIO = new SkFileIO(this);
			if(!fileIO.isConfigExists(destfile)){
				Log.d("sktr069", "write_res_to_disk:srcfile:" + srcfile + "destfile:" + destfile);
				fileIO.writeResToDisk(srcfile, src_file);
			}
			else
			{
				InputStream fis = this.getResources().getAssets().open(srcfile);  
				int src_file_size = fis.available();
				Log.d(TAG, "srcfile filesize = " +  src_file_size);
				fis.close(); 
				
				
				File f  = new File(destfile);
				long dst_file_size = f.length();
				Log.d(TAG, "dstfile filesize = " +  dst_file_size);
				
				if(src_file_size != dst_file_size)
				{
					Log.d(TAG, "file is different !\n write_res_to_disk:srcfile: " + srcfile + " destfile:" + destfile);
					
					fileIO.writeResToDisk(srcfile, src_file);
				}
				else
				{
					
					Log.d(TAG, "file tr069_model.xml is sample!\n ");
					
				}
				
			}
		}
		catch (Exception e) 
		{
			e.printStackTrace();
		}
	}

}