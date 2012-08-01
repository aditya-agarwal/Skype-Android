package com.skype.android;


import java.util.Collection;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.ContentValues;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.skype.android.ThinClientProtocol.Contact;
import com.skype.android.contentprovider.SkypeContactsContentProvider;
import com.skype.android.databasehelper.SkypeContactsDatabaseHelper;
import com.skype.android.helper.SkypeApiIntents;

public class LoginActivity extends Activity {
	
	private static final String TAG = "LoginActivity";
	private Intent mServiceIntent;
	private ThinClientProtocol mProtocol;
	private Collection<Contact> mContacts;
	private String mUserName = null;
	private String mPassword = null;
	private ProgressDialog mProgressDialog;
	private Toast mToast;
	private CharSequence mToastText = "Loggin In";
	private int mToastDuration = Toast.LENGTH_SHORT;
	private EditText mTxtUserName;
	private EditText mTxtPassword;
//	private Button mBtnLogin;

	final Messenger mMessenger = new Messenger(new IncomingHandler());

	class IncomingHandler extends Handler {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case SkypeApiIntents.LOGIN_COMPLETE:
				Log.i(TAG,"**************** MESSAGE FROM SERVICE : LOGIN_COMPELTE ******************");
				mToast.setText("LOGGIN COMPLETE");
				mToast.show();
				stopService(mServiceIntent);
				loadContactsInDB();
				Intent contactsIntent = new Intent(getApplicationContext(), ContactsActivity.class);
				startActivity(contactsIntent);
				mProgressDialog.dismiss();
				finish();
				break;
				
			case SkypeApiIntents.LOGGED_OUT:
				Log.i(TAG,"**************** MESSAGE FROM SERVICE : LOGGED OUT ******************" + mServiceIntent);
				stopService(mServiceIntent);
				mProgressDialog.dismiss();
				mToast.setText("ERROR : LOGGING OUT PLEASE TRY AGAIN");
				mToast.show();

			default:
				super.handleMessage(msg);
			}
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		Log.i(TAG,"--------------------Login ONCREATE--------------------");
		
		mProtocol = ThinClientProtocol.getInstance(new FetchContactsService());
		
		if(FetchContactsService.isUserLoggedIn){
			Log.i(TAG,"--------------------Login ONCREATE LOGGED IN--------------------");
			Intent contactsIntent = new Intent(getApplicationContext(), ContactsActivity.class);
			startActivity(contactsIntent);
			finish();
		}
		mToast = Toast.makeText(getApplicationContext(), mToastText, mToastDuration);

		mTxtUserName=(EditText)this.findViewById(R.id.username);
        mTxtPassword=(EditText)this.findViewById(R.id.password);
//        mBtnLogin=(Button)this.findViewById(R.id.btnLogin);
	}
	
	@Override
	protected void onStart(){
		super.onStart();
		Log.i(TAG,"--------------------Login ONSTART--------------------");
	}

	 public void onLoginClick(View v) {  
		 if( v.getId() == R.id.btnLogin ) {
			 if(mTxtUserName.getText().toString().length() > 0 && mTxtPassword.getText().toString().length() > 0 ){
				 mProgressDialog = ProgressDialog.show(LoginActivity.this, "In progress", "Loading");
				 mUserName = mTxtUserName.getText().toString();
				 mPassword = mTxtPassword.getText().toString();
				 if(mProtocol.isConnected() == false && mProtocol.isLoggedIn() == false){

						mProtocol.setUsername(mUserName);
						mProtocol.setPassword(mPassword);
						mProtocol.connect("dir1.sd.skype.net:9010");
					}
				 	mToast.show();
					mServiceIntent = new Intent(getApplicationContext(), FetchContactsService.class);
					mServiceIntent.putExtra(FetchContactsService.ACTIVITY_MESSENGER, mMessenger);
					startService(mServiceIntent);
			 }
		 }
	 }
	@Override
	protected void onResume() {
		super.onResume();
		Log.i(TAG,"--------------------Login ONRESUME--------------------");

	}

	@Override
	protected void onDestroy(){
		super.onDestroy();
		Log.i(TAG,"--------------------Login ONDESTROY--------------------");
		if(mServiceIntent != null){
			stopService(mServiceIntent);
		}
	}
	
	private void loadContactsInDB() {
		mContacts = mProtocol.getContacts();

		for(Contact c : mContacts){
			Log.i(TAG,"******** LOADED CONTACT ********" + c.availability);
			ContentValues values = new ContentValues();
			values.put(SkypeContactsDatabaseHelper.COLUMN_INDEX,c.index);
			values.put(SkypeContactsDatabaseHelper.COLUMN_NAME, c.name);
			values.put(SkypeContactsDatabaseHelper.COLUMN_FULLNAME, c.fullname);
			values.put(SkypeContactsDatabaseHelper.COLUMN_AVAILABILITY, c.availability);	
			getContentResolver().insert(
					SkypeContactsContentProvider.CONTENT_URI, values);
		}

	}
}