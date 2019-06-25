package net.kzxiv.lockmypc.client;

import android.content.*;
import android.database.*;
import android.database.sqlite.*;
import android.os.*;
import android.view.*;
import android.widget.*;
import java.util.*;

public class HostAdapter extends BaseAdapter
{
	private final Context _context;
	private final LayoutInflater _inflater;
	private final ArrayList<Entry> _entries;

	public final static class Entry
	{
		private final long _id;
		private String _name;
		private String _host;
		private String _port;
		private String _secret;

		public Entry(long id)
		{
			this(id, null, null, null, null);
		}

		public Entry(long id, String name, String host, String port, String secret)
		{
			_id = id;
			_name = neverNull(name);
			_host = neverNull(host);
			_port = neverNull(port);
			_secret = neverNull(secret);
		}

		@Override
		public String toString()
		{
			return _name;
		}

		public final Entry withId(long id)
		{
			return new Entry(id, _name, _host, _port, _secret);
		}

		public final long getId()
		{
			return _id;
		}

		public final String getName()
		{
			return _name;
		}

		public final void setName(String value)
		{
			_name = neverNull(value);
		}

		public final String getHost()
		{
			return _host;
		}

		public final void setHost(String value)
		{
			_host = neverNull(value);
		}

		public final String getPort()
		{
			return _port;
		}

		public final void setPort(String value)
		{
			_port = neverNull(value);
		}

		public final String getSecret()
		{
			return _secret;
		}

		public final void setSecret(String value)
		{
			_secret = neverNull(value);
		}

		private static final String neverNull(String value)
		{
			return value == null ? "" : value;
		}
	}

	public final static class EntryNameComparator implements Comparator<Entry>
	{
		@Override
		public int compare(Entry left, Entry right)
		{
			return left.getName().compareToIgnoreCase(right.getName());
		}
	}

	public HostAdapter(Context context)
	{
		if (context == null)
			throw new IllegalArgumentException();

		_context = context;
		_inflater = LayoutInflater.from(context);
		_entries = new ArrayList<Entry>();
	}

	@Override
	public boolean hasStableIds()
	{
		return true;
	}

	@Override
	public int getCount()
	{
		return _entries.size();
	}

	@Override
	public Object getItem(int i)
	{
		if (i >= 0 && i < _entries.size())
			return _entries.get(i);
		else
			return null;
	}

	@Override
	public long getItemId(int i)
	{
		if (i >= 0 && i < _entries.size())
			return _entries.get(i).getId();
		else
			return -1L;
	}

	@Override
	public View getView(int i, View view, ViewGroup viewGroup)
	{
		Entry entry = _entries.get(i);
		if (entry == null)
			return null;

		if (view == null)
			view = _inflater.inflate(R.layout.content, null);

		TextView text;
		text = view.findViewById(R.id.host_name);

		String name = entry.getName();
		if (name.length() > 0)
			text.setText(name);
		else
			text.setText(R.string.host_default_name);

		text = view.findViewById(R.id.host_extra);
		text.setText(
			String.format(
				_context.getString(R.string.host_port_format),
				entry.getHost(),
				entry.getPort()));

		view.setTag(entry);
		return view;
	}

	public final void loadAsync()
	{
		new LoadAsyncTask().execute();
	}

	private final class LoadAsyncTask extends AsyncTask<Void, Void, ArrayList<Entry>>
	{
		@Override
		protected ArrayList<Entry> doInBackground(Void... unused)
		{
			ArrayList<Entry> result = new ArrayList<Entry>();

			HostDatabaseOpenHelper helper = null;
			try
			{
				helper = new HostDatabaseOpenHelper(_context);
				SQLiteDatabase db = helper.getReadableDatabase();

				Cursor cursor = db.rawQuery("SELECT id, name, host, port, secret FROM hosts ORDER BY name COLLATE NOCASE ASC", new String[0]);
				try
				{
					while (cursor.moveToNext())
					{
						Entry entry = new Entry(
							cursor.getLong(0),
							cursor.getString(1),
							cursor.getString(2),
							cursor.getString(3),
							cursor.getString(4));

						result.add(entry);
					}

				}
				finally
				{
					cursor.close();
				}
			}
			finally
			{
				if (helper != null)
					helper.close();
			}

			return result;
		}

		@Override
		protected void onPostExecute(ArrayList<Entry> result)
		{
			_entries.clear();
			_entries.addAll(result);
			notifyDataSetChanged();
		}
	}

	public final void addAsync(Entry entry)
	{
		if (entry == null)
			throw new IllegalArgumentException();

		new AddAsyncTask().execute(entry);
	}

	private final class AddAsyncTask extends AsyncTask<Entry, Void, Entry>
	{
		@Override
		protected Entry doInBackground(Entry... entries)
		{
			Entry entry = entries[0];

			HostDatabaseOpenHelper helper = null;
			long id;
			try
			{
				helper = new HostDatabaseOpenHelper(_context);
				SQLiteDatabase db = helper.getWritableDatabase();

				SQLiteStatement stmt = db.compileStatement("INSERT INTO hosts (name, host, port, secret) VALUES(?, ?, ?, ?)");
				try
				{
					stmt.bindString(1, entry.getName());
					stmt.bindString(2, entry.getHost());
					stmt.bindString(3, entry.getPort());
					stmt.bindString(4, entry.getSecret());

					id = stmt.executeInsert();
				}
				finally
				{
					stmt.close();
				}
			}
			finally
			{
				if (helper != null)
					helper.close();
			}

			return entry.withId(id);
		}

		@Override
		protected void onPostExecute(Entry entry)
		{
			_entries.add(entry);
			Collections.sort(_entries, new EntryNameComparator());
			notifyDataSetChanged();
		}
	}

	public final void updateAsync(Entry entry)
	{
		if (entry == null)
			throw new IllegalArgumentException();

		new UpdateAsyncTask().execute(entry);
	}

	private final class UpdateAsyncTask extends AsyncTask<Entry, Void, Void>
	{
		@Override
		protected Void doInBackground(Entry... entries)
		{
			Entry entry = entries[0];

			HostDatabaseOpenHelper helper = null;
			try
			{
				helper = new HostDatabaseOpenHelper(_context);
				SQLiteDatabase db = helper.getWritableDatabase();

				SQLiteStatement stmt = db.compileStatement("UPDATE hosts SET name = ?, host = ?, port = ?, secret = ? WHERE id = ?");
				try
				{
					stmt.bindString(1, entry.getName());
					stmt.bindString(2, entry.getHost());
					stmt.bindString(3, entry.getPort());
					stmt.bindString(4, entry.getSecret());
					stmt.bindLong(5, entry.getId());

					stmt.executeUpdateDelete();
				}
				finally
				{
					stmt.close();
				}
			}
			finally
			{
				if (helper != null)
					helper.close();
			}

			return null;
		}

		@Override
		protected void onPostExecute(Void aVoid)
		{
			Collections.sort(_entries, new EntryNameComparator());
			notifyDataSetChanged();
		}
	}

	public final void removeAsync(Entry entry)
	{
		if (entry == null)
			throw new IllegalArgumentException();

		new RemoveAsyncTask().execute(entry);
	}

	private final class RemoveAsyncTask extends AsyncTask<Entry, Void, Entry>
	{
		@Override
		protected Entry doInBackground(Entry... entries)
		{
			Entry entry = entries[0];

			HostDatabaseOpenHelper helper = null;
			try
			{
				helper = new HostDatabaseOpenHelper(_context);
				SQLiteDatabase db = helper.getWritableDatabase();

				SQLiteStatement stmt = db.compileStatement("DELETE FROM hosts WHERE id = ?");
				try
				{
					stmt.bindLong(1, entry.getId());
					stmt.executeUpdateDelete();
				}
				finally
				{
					stmt.close();
				}
			}
			finally
			{
				if (helper != null)
					helper.close();
			}

			return entry;
		}

		@Override
		protected void onPostExecute(Entry result)
		{
			_entries.remove(result);
			notifyDataSetChanged();
		}
	}

	private final class HostDatabaseOpenHelper extends SQLiteOpenHelper
	{
		public HostDatabaseOpenHelper(Context context)
		{
			super(context, "hosts.sqlite", null, 1);
			setWriteAheadLoggingEnabled(true);
		}

		@Override
		public void onCreate(SQLiteDatabase db)
		{
			db.execSQL("CREATE TABLE hosts (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, name TEXT NOT NULL, host TEXT NOT NULL, port TEXT NOT NULL, secret TEXT NOT NULL)");
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
		{
		}
	}
}
