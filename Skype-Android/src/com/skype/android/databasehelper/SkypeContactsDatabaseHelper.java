package com.skype.android.databasehelper;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class SkypeContactsDatabaseHelper extends SQLiteOpenHelper {
	
	public static final String DATABASE_NAME = "skype_contacts.db";

    /**
     * The database version
     */
    private static final int DATABASE_VERSION = 1;

	
	// Database table
		public static final String TABLE_CONTACTS = "Contacts";
		public static final String COLUMN_ID = "_id";
		public static final String COLUMN_INDEX = "_index";
		public static final String COLUMN_NAME = "name";
		public static final String COLUMN_FULLNAME = "fullname";
		public static final String COLUMN_AVAILABILITY = "availability";

		// Database creation SQL statement
		private static final String DATABASE_CREATE = "create table " 
				+ TABLE_CONTACTS
				+ "(" 
				+ COLUMN_ID + " integer primary key autoincrement, "
				+ COLUMN_INDEX + " integer, " 
				+ COLUMN_NAME + " text, " 
				+ COLUMN_FULLNAME + " text," 
				+ COLUMN_AVAILABILITY + " integer " 
				+ ");";


	public SkypeContactsDatabaseHelper(Context context) {
		super(context, DATABASE_NAME, null, DATABASE_VERSION);
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
		db.execSQL(DATABASE_CREATE);
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		// TODO Auto-generated method stub
		
	}

}
