package net.kzxiv.lockmypc.client;

import android.content.*;
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
			_name = neverNull(name, "Untitled");
			_host = neverNull(host, "host");
			_port = neverNull(port, "43666");
			_secret = neverNull(secret, "default");
		}

		@Override
		public String toString()
		{
			return _name;
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

		private static final String neverNull(String value, String other)
		{
			return value == null ? other : value;
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

		text.setText(entry.getName());

		text = view.findViewById(R.id.host_extra);
		text.setText(
			String.format(
				_context.getString(R.string.host_port_format),
				entry.getHost(),
				entry.getPort()));

		view.setTag(entry);
		return view;
	}

	public final void load()
	{
		ArrayList<Entry> result = loadInternal();
		_entries.clear();
		_entries.addAll(result);
		notifyDataSetChanged();
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
			return loadInternal();
		}

		@Override
		protected void onPostExecute(ArrayList<Entry> result)
		{
			_entries.clear();
			_entries.addAll(result);
			notifyDataSetChanged();
		}
	}

	public final ArrayList<Entry> loadInternal()
	{
		ArrayList<Entry> result = new ArrayList<Entry>();

		// TODO:
		try { Thread.sleep(500); } catch (InterruptedException ex) {}
		for (long l = 0; l < 200; ++l)
			result.add(new Entry(l, String.format("Host #%d", l), null, null, null));

		return result;
	}
}
