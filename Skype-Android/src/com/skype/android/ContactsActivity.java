package com.skype.android;

import java.util.Timer;
import java.util.TimerTask;

import android.app.ListActivity;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.skype.android.contentprovider.SkypeContactsContentProvider;
import com.skype.android.databasehelper.SkypeContactsDatabaseHelper;
import com.skype.android.helper.SkypeApiIntents;

public class ContactsActivity extends ListActivity {

	private static final String TAG = "ContactsActivity";
	private Cursor cursor;
	private Intent serviceIntent;
	private Timer timer;
	private ThinClientProtocol mTCP ;

	final Messenger mMessenger = new Messenger(new IncomingHandler());

	class IncomingHandler extends Handler {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case SkypeApiIntents.PRESENCE_UPDATED:
				Log.i(TAG,"**************** MESSAGE FROM SERVICE : PRESENCE UPDATED AT INDEX ******************" + msg.arg1);
				updateDatabaseWithAvailability(msg.arg1, msg.arg2);
				break;
			case SkypeApiIntents.LOGGED_OUT:
				Log.i(TAG,"**************** MESSAGE FROM SERVICE : LOG OUT COMPLETE ******************" + serviceIntent);
				stopService(serviceIntent);
				dropTablesFromDatabase();
				finish();
			default:
				super.handleMessage(msg);
			}
		}
	}
    
		@Override
		public void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);
			setContentView(R.layout.contactslistview);
			
			cursor = getContentResolver().query(SkypeContactsContentProvider.CONTENT_URI, new String[] {SkypeContactsDatabaseHelper.COLUMN_ID,SkypeContactsDatabaseHelper.COLUMN_NAME, SkypeContactsDatabaseHelper.COLUMN_AVAILABILITY}, null, null, null);
			            startManagingCursor(cursor);
			cursor.setNotificationUri(getContentResolver(),SkypeContactsContentProvider.CONTENT_URI);
			setListAdapter(new ContactCursorAdapter(getApplicationContext(), R.layout.contactrow, cursor, new String[] {SkypeContactsDatabaseHelper.COLUMN_ID,SkypeContactsDatabaseHelper.COLUMN_NAME, SkypeContactsDatabaseHelper.COLUMN_AVAILABILITY}, null));
			
			mTCP = ThinClientProtocol.getInstance(new FetchContactsService());
			timer = new Timer();
			timer.schedule(new CheckPresenceUpdatesTask(), 0, 1000);
			
			serviceIntent = new Intent(getApplicationContext(), FetchContactsService.class);
			serviceIntent.putExtra(FetchContactsService.ACTIVITY_MESSENGER, mMessenger);
			startService(serviceIntent);
		}
		
		@Override
		protected void onStart(){
			super.onStart();
		}


		@Override
		protected void onResume() {
			super.onResume();

		}

		@Override
		protected void onDestroy(){
			super.onDestroy();
			cursor.close();
			stopService(serviceIntent); // STOP SERVICE IN LOGOUT
			timer.cancel();
		}
		
		@Override
		public boolean onCreateOptionsMenu(Menu menu) {
		    MenuInflater inflater = getMenuInflater();
		    inflater.inflate(R.menu.logout, menu);
		    return true;
		}
		
		@Override
		public boolean onOptionsItemSelected(MenuItem item) {
		    switch (item.getItemId()) {
		        case R.id.logout:    
		        	mTCP.logout();
		        	dropTablesFromDatabase();
					finish();
		            break;
		    }
		    return true;
		}
		
		private class CheckPresenceUpdatesTask extends TimerTask{

			@Override
			public void run() {
				Log.i(TAG,"--------------------CHECKING FOR PRESENCE UPDATES--------------------");
				mTCP.getPresenceUpdate();
			} 
			
		}
		
		private void updateDatabaseWithAvailability(int atIndex, int availability) {
			ContentValues values = new ContentValues();
			values.put(SkypeContactsDatabaseHelper.COLUMN_AVAILABILITY, availability);
			getContentResolver().update(Uri.parse(SkypeContactsContentProvider.CONTENT_URI + "/" + atIndex ), values, null, null);		
		}
		
		private void dropTablesFromDatabase(){
			//Deleting or Dropping the Contacts table from the database in delete call of the content provider
			getContentResolver().delete(SkypeContactsContentProvider.CONTENT_URI, null, null);
			cursor.close();
			stopService(serviceIntent);
			timer.cancel();
		}
}