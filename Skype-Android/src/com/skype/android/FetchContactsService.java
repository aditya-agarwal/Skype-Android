package com.skype.android;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;

import com.skype.android.helper.SkypeApiIntents;

public class FetchContactsService extends Service implements ThinClientProtocol.Callback {

	private static final String TAG = "FetchContactsService";
	public static boolean isUserLoggedIn = false;
	boolean isLoggingIn = false;
	private ThinClientProtocol mProtocol = ThinClientProtocol.getInstance(this);
	private static Timer mTimer;


	private static Messenger mActivityMessenger;
	public static final int MSG_RESPONSE_HANDLER = 1;
	public static final String ACTIVITY_MESSENGER = "com.skype.android.activity_messenger";
	
	private Handler timerHandler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			Log.i(TAG,"-------****** KILL THE TIMER *****----"  );
			stopSelf();
			mTimer.cancel();			
		}
	};

	@Override
	public void onCreate(){
		super.onCreate();
		Log.i(TAG,"--------------------Service CREATED--------------------");
	}

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void onStart(Intent intent, int startId) {
		super.onStart(intent, startId);

		Log.i(TAG,"--------------------Service STARTED--------------------");

		//check if bundle present
		Bundle extras = intent.getExtras();
		Messenger messenger;

		messenger = (Messenger)extras.get(ACTIVITY_MESSENGER);
		if (messenger != null) {
			mActivityMessenger = messenger;
		}
		mTimer = new Timer();
		mTimer.schedule(new ProcessIncomingDataTask(), 0, 500);	   
	}

	private class ProcessIncomingDataTask extends TimerTask{

		@Override
		public void run() {
			Log.i(TAG,"--------------------Processing Incoming Data--------------------");
			mProtocol.processIncomingData();
		} 

	}

	@Override
	public void onConnected() {
		Log.i(TAG,"--------------------Connected--------------------");
//		loginHandler.sendEmptyMessage(0);
		mProtocol.login();

	}

	@Override
	public void onLoggedIn() {
		Log.i(TAG,"--------------------LoggedIN--------------------");
		isUserLoggedIn = true;
		mProtocol.getBuddyList();

	}

	@Override
	public void onLoggedOut() {

		isUserLoggedIn = false;
		timerHandler.sendEmptyMessage(0);
		Message message = Message.obtain(null, SkypeApiIntents.LOGGED_OUT);
		try {
			mActivityMessenger.send(message);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onWarningReceived(int code, String message) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onErrorReceived(int code, String message) {

		if(mProtocol.isLoggedIn()){
			mProtocol.logout();
			isUserLoggedIn = false;
		}
	}

	@Override
	public void onBuddyListLoaded() {
		Log.i(TAG,"--------------------BUDDYLIST--------------------");

		isUserLoggedIn = true;

		timerHandler.sendEmptyMessage(0);
		Message message = Message.obtain(null, SkypeApiIntents.LOGIN_COMPLETE);
		try {
			mActivityMessenger.send(message);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onPresenceUpdated(int buddyIndex, int availability) {
		Log.i(TAG,"--------------------Presence Updated--------------------" + buddyIndex);
		isUserLoggedIn = true;

		//Sending message to Contact Activity : presence is updated
		Message message = Message.obtain(null, SkypeApiIntents.PRESENCE_UPDATED, buddyIndex, availability);
		try {
			mActivityMessenger.send(message);
		} catch (RemoteException e) {
			e.printStackTrace();
		}

	}


	@Override
	public void onDestroy(){
		super.onDestroy();
		Log.i(TAG,"--------------------Service Destoryed--------------------");
	}

}
