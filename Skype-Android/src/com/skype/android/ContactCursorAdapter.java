package com.skype.android;

import com.skype.android.ThinClientProtocol.Availability;
import com.skype.android.databasehelper.SkypeContactsDatabaseHelper;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;

public class ContactCursorAdapter extends SimpleCursorAdapter {
	private int layout;
	private  LayoutInflater inflater;

	public ContactCursorAdapter(Context context, int layout, Cursor c, String[] from, int[] to) {
		super(context, layout, c, from, to);
		this.layout = layout;
		inflater = LayoutInflater.from(context);

	}


	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent) {
		View v = inflater.inflate(layout, parent, false);

		return v;
	}

	@Override
	public void bindView(View v, Context context, Cursor cursor) {

		String name = cursor.getString(cursor.getColumnIndex(SkypeContactsDatabaseHelper.COLUMN_NAME));
		int status = cursor.getInt(cursor.getColumnIndex(SkypeContactsDatabaseHelper.COLUMN_AVAILABILITY));

		Log.i("CONTACT LIST ACTIVITY","---- NAME -----"+name);
		Log.i("CONTACT LIST ACTIVITY","---- STATUS -----"+ status);
		TextView name_text = (TextView) v.findViewById(R.id.name);
		if (name_text != null) {
			name_text.setText(name);
		}

		ImageView statusIcon = (ImageView)v.findViewById(R.id.status);
		switch (status){
		case Availability.OFFLINE:
			statusIcon.setImageResource(R.drawable.unavailable);
			break;
			
		case Availability.ONLINE:
      	  statusIcon.setImageResource(R.drawable.available);
      	  break;
      	  
		case Availability.AWAY:
      	  statusIcon.setImageResource(R.drawable.away);
      	  break;
      	  
		case Availability.DO_NOT_DISTURB:
      	  statusIcon.setImageResource(R.drawable.busy);
      	  break;
      	  
      	 default:
      		statusIcon.setImageResource(R.drawable.ic_launcher);
      	 
		}

	}

}
