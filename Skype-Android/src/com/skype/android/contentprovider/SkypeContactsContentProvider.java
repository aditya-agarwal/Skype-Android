package com.skype.android.contentprovider;

import com.skype.android.databasehelper.SkypeContactsDatabaseHelper;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;

public class SkypeContactsContentProvider extends ContentProvider {
	SkypeContactsDatabaseHelper mDBHelper;

	private static final int CONTACTS = 10;
	private static final int CONTACTS_ID = 20;
	private static final String AUTHORITY = "com.skype.android.contacts.contentprovider";

	private static final String BASE_PATH = "contacts";
	public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY
			+ "/" + BASE_PATH);

	private static final UriMatcher sURIMatcher = new UriMatcher(
			UriMatcher.NO_MATCH);

	static {
		sURIMatcher.addURI(AUTHORITY, BASE_PATH, CONTACTS);
		sURIMatcher.addURI(AUTHORITY, BASE_PATH + "/#", CONTACTS_ID);
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs) {
		SQLiteDatabase db = mDBHelper.getWritableDatabase();
		db.execSQL("DROP TABLE IF EXISTS " + SkypeContactsDatabaseHelper.TABLE_CONTACTS);
		return 0;
	}

	@Override
	public String getType(Uri uri) {
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues initialValues) {
		// TODO Auto-generated method stub
		// A map to hold the new record's values.
		ContentValues values;
		int uriType = sURIMatcher.match(uri);
		// If the incoming values map is not null, uses it for the new values.
		if (initialValues != null) {
			values = new ContentValues(initialValues);

		} else {
			// Otherwise, create a new value map
			values = new ContentValues();
		}


		SQLiteDatabase db = mDBHelper.getWritableDatabase();
		long id = 0;
		switch (uriType) {
		case CONTACTS:
			id = db.insert(SkypeContactsDatabaseHelper.TABLE_CONTACTS, null, values);
			break;
		default:
			throw new IllegalArgumentException("Unknown URI: " + uri);
		}
		getContext().getContentResolver().notifyChange(uri, null);
		return Uri.parse(BASE_PATH + "/" + id);
	}

	@Override
	public boolean onCreate() {
		mDBHelper = new SkypeContactsDatabaseHelper(getContext());
		return true;
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
			String[] selectionArgs, String sortOrder) {

		SQLiteQueryBuilder queryBuilder = new SQLiteQueryBuilder();


		queryBuilder.setTables(SkypeContactsDatabaseHelper.TABLE_CONTACTS);

		int uriType = sURIMatcher.match(uri);
		switch (uriType) {
		case CONTACTS:
			break;
		case CONTACTS_ID:

			queryBuilder.appendWhere(SkypeContactsDatabaseHelper.COLUMN_INDEX + "="
					+ uri.getLastPathSegment());
			break;
		default:
			throw new IllegalArgumentException("Unknown URI: " + uri);
		}

		SQLiteDatabase db = mDBHelper.getWritableDatabase();
		Cursor cursor = queryBuilder.query(db, projection, selection,
				selectionArgs, null, null, sortOrder);

		cursor.setNotificationUri(getContext().getContentResolver(), uri);

		return cursor;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		
		int uriType = sURIMatcher.match(uri);
		SQLiteDatabase sqlDB = mDBHelper.getWritableDatabase();
		int rowsUpdated = 0;
		switch (uriType) {
		case CONTACTS:
			break;
		case CONTACTS_ID:
			String id = uri.getLastPathSegment();
			
			rowsUpdated = sqlDB.update(SkypeContactsDatabaseHelper.TABLE_CONTACTS, 
					values,
					SkypeContactsDatabaseHelper.COLUMN_ID + "=" + id, 
					null);
			break;
		default:
			throw new IllegalArgumentException("Unknown URI: " + uri);
		}
		getContext().getContentResolver().notifyChange(uri, null);
		return rowsUpdated;
	}

}
