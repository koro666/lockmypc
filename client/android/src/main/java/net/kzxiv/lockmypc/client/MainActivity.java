package net.kzxiv.lockmypc.client;

import android.content.*;
import android.os.*;
import android.support.design.widget.*;
import android.support.v7.app.*;
import android.view.*;
import android.widget.*;

import android.support.v7.app.AlertDialog;

public class MainActivity extends AppCompatActivity
{
	private HostAdapter _adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		final ListView list = findViewById(R.id.host_list);

		_adapter = new HostAdapter(this);
		list.setAdapter(_adapter);

		_adapter.loadAsync();

		list.setOnItemClickListener(new AdapterView.OnItemClickListener()
		{
			@Override
			public void onItemClick(AdapterView<?> adapterView, View view, int i, long l)
			{
				onEntryClick((HostAdapter.Entry)view.getTag());
			}
		});

		registerForContextMenu(list);

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

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo)
	{
		if (v.getId() == R.id.host_list)
		{
			AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) menuInfo;
			HostAdapter.Entry entry = (HostAdapter.Entry)info.targetView.getTag();

			getMenuInflater().inflate(R.menu.menu_host, menu);
			menu.setHeaderTitle(entry.getName());
		}
		else
		{
			super.onCreateContextMenu(menu, v, menuInfo);
		}
	}

	@Override
	public boolean onContextItemSelected(MenuItem item)
	{
		int id = item.getItemId();

		if (id == R.id.host_modify || id == R.id.host_remove)
		{
			AdapterView.AdapterContextMenuInfo info = (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
			HostAdapter.Entry entry = (HostAdapter.Entry)info.targetView.getTag();

			switch (id)
			{
				case R.id.host_modify:
					editEntry(entry, false);
					return true;
				case R.id.host_remove:
					_adapter.removeAsync(entry);
					return true;
				default:
					return false;
			}
		}
		else
		{
			return super.onContextItemSelected(item);
		}
	}

	private final void onEntryClick(HostAdapter.Entry entry)
	{
		// TODO:
	}

	private final void onAddClick()
	{
		HostAdapter.Entry entry = new HostAdapter.Entry(-1);
		editEntry(entry, true);
	}

	private final void editEntry(final HostAdapter.Entry entry, final boolean isNew)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(isNew ? R.string.host_add_long : R.string.host_modify_long);
		builder.setView(R.layout.dialog_edit);

		builder.setPositiveButton(
			isNew ? R.string.host_dialog_add : R.string.host_dialog_save,
			new AlertDialog.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialogInterface, int i)
				{
					if (i != DialogInterface.BUTTON_POSITIVE)
						return;

					AlertDialog dialog = (AlertDialog)dialogInterface;

					EditText edit = (EditText)dialog.findViewById(R.id.host_name);
					entry.setName(edit.getText().toString());

					edit = (EditText)dialog.findViewById(R.id.host_host);
					entry.setHost(edit.getText().toString());

					edit = (EditText)dialog.findViewById(R.id.host_port);
					entry.setPort(edit.getText().toString());

					edit = (EditText)dialog.findViewById(R.id.host_secret);
					entry.setSecret(edit.getText().toString());

					if (isNew)
						_adapter.addAsync(entry);
					else
						_adapter.updateAsync(entry);
				}
			});

		builder.setNegativeButton(R.string.host_dialog_cancel, null);

		AlertDialog dialog = builder.create();
		dialog.create();

		EditText edit = (EditText)dialog.findViewById(R.id.host_name);
		edit.setText(entry.getName());

		edit = (EditText)dialog.findViewById(R.id.host_host);
		edit.setText(entry.getHost());

		edit = (EditText)dialog.findViewById(R.id.host_port);
		edit.setText(entry.getPort());

		edit = (EditText)dialog.findViewById(R.id.host_secret);
		edit.setText(entry.getSecret());

		dialog.show();
	}
}
