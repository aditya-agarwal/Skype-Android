package com.skype.android;

import java.util.Collection;
import java.util.HashMap;

import android.util.Log;

/**
 * ThinClientProtocol JNI wrapper
 * 
 * @author Nikhil Purushe
 */
class ThinClientProtocol {
	
	private static ThinClientProtocol mTCP = null;

	private static final String TAG = "ThinClientProtocol";
	private Callback callback;
	private HashMap<Integer,Contact> contacts;
	
	/**
	 * Callback interface that receives events received from the server.
	 */
	public static interface Callback
	{
		/**
		 * Received connected event.
		 */
		public void onConnected();
		
		/**
		 * Received logged in event.
		 */
		public void onLoggedIn();
		
		/**
		 * Received logged out event.
		 */
		public void onLoggedOut();
		
		/**
		 * Received warning.
		 * @param code
		 * @param message
		 */
		public void onWarningReceived(int code, String message);
		
		/**
		 * Received error.
		 * @param code
		 * @param message
		 */
		public void onErrorReceived(int code, String message);
		
		/**
		 * Buddy list loaded.
		 */
		public void onBuddyListLoaded();
		
		/**
		 * Presence updated.
		 * @param availability 
		 */
		public void onPresenceUpdated(int buddyIndex, int availability);
	}
	
	/**
	 * Represents the availability status of a contact.
	 * Maps to the values in tcprotoV3.hpp
	 */
	public static class Availability {
		
		private Availability() {}
		
		public static final int UNKNOWN=0;
		public static final int OFFLINE=1;
		public static final int ONLINE=2;
		public static final int AWAY=3;
		public static final int NOT_AVAILABLE=4;
		public static final int DO_NOT_DISTURB=5;
		public static final int INVISIBLE=6;
		public static final int SKYPE_ME=7;
		public static final int PENDINGAUTH=8;
		public static final int BLOCKED=9;
		public static final int SKYPEOUT=10;
		public static final int BLOCKED_SKYPEOUT=11;
		public static final int OFFLINE_BUT_VM_ABLE=12;
		public static final int OFFLINE_BUT_CF_ABLE=13;
		public static final int CONNECTING=14;
		public static final int DELETED=99;
	};
	
	/**
	 * Represents a Skype contact/buddy.
	 */
	public class Contact {
		
		public int index;
		public String name;
		public String fullname;
		public int availability;
	}
	
	/**
	 * Create protocol wrapper
	 * @param callback
	 */
	private ThinClientProtocol(Callback callback) {
		
		this.callback = callback;
		contacts = new HashMap<Integer,Contact>();
		System.loadLibrary("skypelib-jni");
		initialize();
	}
	
	//Making ThinClientProtocol Singleton
	public static synchronized ThinClientProtocol getInstance(Callback callback){
		if(mTCP == null)
			mTCP = new ThinClientProtocol(callback);
		return mTCP;
	}
	
	// native methods
	
	/**
	 * Initialize native object.
	 */
	private native void initialize();
	
	/**
	 * Set's the skype username to login with.
	 * @param username
	 */
	protected native void setUsername(String username);
	
	/**
	 * Set's the skype password to login with.
	 * @param password
	 */
	protected native void setPassword(String password);
	
	/**
	 * Connect to the given host server.
	 * @param host
	 * @return
	 */
	protected native boolean connect(String host);
	
	/**
	 * Process data incoming to the client. You need to call this method on a separate thread. 
	 * The data received will be processed and the appropriate {@link Callback} methods executed.
	 * @return true if data was processed or not.
	 */
	protected native boolean processIncomingData();
	
	/**
	 * Login to the server.
	 * @return
	 */
	protected native boolean login();
	
	/**
	 * Logout from the server.
	 * @return
	 */
	protected native boolean logout();
	
	/**
	 * Disconnect the client from the server.
	 */
	protected native void disconnect();
	
	/**
	 * Send a get buddy list request to the server.
	 * @return true if the request was sent, false otherwise.
	 */
	protected native boolean getBuddyList();
	
	/**
	 * Send a presence update request to the server.
	 * @return true if the request was sent, false otherwise.
	 */
	protected native boolean getPresenceUpdate();
	
	/**
	 * @return true if the user is logged in, false otherwise.
	 */
	protected native boolean isLoggedIn();
	
	/**
	 * Flag indicating if the client is connected to the server.
	 * @return true if connected, false otherwise
	 */
	protected native boolean isConnected();
	
	/**
	 * Flag indicating if the client has been disconnected.
	 * @return true if disconnected, false otherwise.
	 */
	protected native boolean isDisconnected();
	
	/**
	 * Get's the current collection of contacts.
	 * @return
	 */
	public Collection<Contact> getContacts() {
		return contacts.values();
	}
	
	
	// the methods below are invoked by the native code. You should NOT call these methods!
	
	public void onConnected() {
		Log.i(TAG, "onConnected");
		
		if(callback != null) {
			callback.onConnected();
		}
	}
	
	public void onLoggedIn() {
		Log.i(TAG, "onLoggedIn");
		if(callback != null) {
			callback.onLoggedIn();
		}
	}
	
	public void onLoggedOut() {
		Log.i(TAG, "onLoggedOut");
		if(callback != null) {
			callback.onLoggedOut();
		}
	}
	
	public void onWarningReceived(int code, String message) {
		Log.i(TAG, "onWarningReceived " + code +  " + message");
		if(callback != null) {
			callback.onWarningReceived(code, message);
		}
	}
	
	public void onErrorReceived(int code, String message) { 
		Log.i(TAG, "onErrorReceivied " + code + " " + message);
		if(callback != null) {
			callback.onErrorReceived(code, message);
		}
	}
	
	public void onBuddyReceived(int total, int buddyIndex, int availability, String name, String fullname) {
		
		Contact c = contacts.get(buddyIndex);
		if(c == null) {
			c = new Contact();
			contacts.put(buddyIndex, c);
		}
		c.index = buddyIndex;
		c.name = name;
		c.fullname = fullname;
//		c.availability = availability;
		Log.i(TAG, "Create contact " + buddyIndex + " " + name + " " + availability);
		if(contacts.size() == total && callback != null) {
			callback.onBuddyListLoaded();
		}
	}
	
	public void onPresenceUpdated(int buddyIndex, int availability) {
		
		Contact c = contacts.get(buddyIndex);
		if(c != null) {
			c.availability = availability;
			if(callback != null) {
				callback.onPresenceUpdated(buddyIndex, availability);
			}
		}
	}

}
