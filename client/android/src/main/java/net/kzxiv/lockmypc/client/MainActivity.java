package net.kzxiv.lockmypc.client;

import android.os.*;
import android.support.design.widget.*;
import android.support.v7.app.*;
import android.util.Log;
import android.view.*;
import android.widget.*;

public class MainActivity extends AppCompatActivity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		final ListView list = findViewById(R.id.host_list);

		HostAdapter adapter = new HostAdapter(this);
		list.setAdapter(adapter);

		adapter.loadAsync();

		list.setOnItemClickListener(new AdapterView.OnItemClickListener()
		{
			@Override
			public void onItemClick(AdapterView<?> adapterView, View view, int i, long l)
			{
				HostAdapter.Entry entry = (HostAdapter.Entry)view.getTag();
				onEntryClick(i, entry);
			}
		});

		list.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener()
		{
			@Override
			public boolean onItemLongClick(AdapterView<?> adapterView, View view, int i, long l)
			{
				HostAdapter.Entry entry = (HostAdapter.Entry)view.getTag();
				return onEntryLongClick(i, entry);
			}
		});

		FloatingActionButton fab = findViewById(R.id.add_button);
		fab.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				onAddClick();
			}
		});
	}

	private final void onEntryClick(int index, HostAdapter.Entry entry)
	{
		// TODO:
	}

	private final boolean onEntryLongClick(int index, HostAdapter.Entry entry)
	{
		// TODO:
		return false;
	}

	private final void onAddClick()
	{
		// TODO:
	}
}
