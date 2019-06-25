package net.kzxiv.lockmypc.client;

import android.content.*;
import android.os.*;
import android.support.design.widget.*;
import android.support.v7.app.*;
import android.text.*;
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
				onEntryClick(view, (HostAdapter.Entry)view.getTag());
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

			String name = entry.getName();
			if (name.length() > 0)
				menu.setHeaderTitle(name);
			else
				menu.setHeaderTitle(R.string.host_default_name);
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
					removeEntry(entry);
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

	private final void onEntryClick(final View view, final HostAdapter.Entry entry)
	{
		Intent intent = new Intent(this, PacketService.class);

		intent.putExtra(PacketService.EXTRA_HOST, entry.getHost());
		intent.putExtra(PacketService.EXTRA_PORT, entry.getPort());
		intent.putExtra(PacketService.EXTRA_SECRET, entry.getSecret());
		intent.putExtra(PacketService.EXTRA_RESULT, new ResultReceiver(new Handler())
		{
			@Override
			protected void onReceiveResult(int result, Bundle bundle)
			{
				String name = entry.getName();
				if (name.length() <= 0)
					name = getString(R.string.host_default_name);

				Snackbar bar;
				if (result == 0)
				{
					bar = Snackbar.make(view, String.format(getString(R.string.packet_success_format), name), Snackbar.LENGTH_SHORT);
				}
				else
				{
					String error = bundle.getString(PacketService.EXTRA_ERROR);
					bar = Snackbar.make(view, String.format(getString(R.string.packet_failure_format), name, error), Snackbar.LENGTH_LONG);
				}

				bar.show();
			}
		});

		startService(intent);
	}

	private final void onAddClick()
	{
		HostAdapter.Entry entry = new HostAdapter.Entry(-1);
		entry.setSecret(getString(R.string.host_default_secret));
		editEntry(entry, true);
	}

	private final void removeEntry(final HostAdapter.Entry entry)
	{
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle(R.string.host_remove_long);

		String name = entry.getName();
		builder.setMessage(
			String.format(
				getString(R.string.host_dialog_remove_format),
				name.length() > 0 ? name : getString(R.string.host_default_name)));

		builder.setPositiveButton(R.string.host_dialog_remove,
			new AlertDialog.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialogInterface, int i)
				{
					if (i == DialogInterface.BUTTON_POSITIVE)
						_adapter.removeAsync(entry);
				}
			});

		builder.setNegativeButton(R.string.host_dialog_keep, null);

		AlertDialog dialog = builder.create();
		dialog.show();
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
					String text = edit.getText().toString();
					entry.setName(text);

					edit = (EditText)dialog.findViewById(R.id.host_host);
					text = edit.getText().toString();
					entry.setHost(text.length() > 0 ? text : getString(R.string.host_default_host));

					edit = (EditText)dialog.findViewById(R.id.host_port);
					text = edit.getText().toString();
					entry.setPort(text.length() > 0 ? text : getString(R.string.host_default_port));

					edit = (EditText)dialog.findViewById(R.id.host_secret);
					text = edit.getText().toString();
					entry.setSecret(text);

					if (isNew)
						_adapter.addAsync(entry);
					else
						_adapter.updateAsync(entry);
				}
			});

		builder.setNegativeButton(R.string.host_dialog_cancel, null);

		final AlertDialog dialog = builder.create();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
			dialog.create();
		else
			dialog.show();

		EditText edit = (EditText)dialog.findViewById(R.id.host_name);
		edit.setText(entry.getName());

		edit = (EditText)dialog.findViewById(R.id.host_host);
		edit.setText(entry.getHost());

		edit = (EditText)dialog.findViewById(R.id.host_port);
		edit.setText(entry.getPort());

		edit = (EditText)dialog.findViewById(R.id.host_secret);
		edit.setText(entry.getSecret());
		edit.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);

		CheckBox check = (CheckBox)dialog.findViewById(R.id.host_secret_show);
		check.setOnCheckedChangeListener(new CheckBox.OnCheckedChangeListener()
		{
			@Override
			public void onCheckedChanged(CompoundButton button, boolean b)
			{
				EditText edit = (EditText)dialog.findViewById(R.id.host_secret);
				edit.setInputType(InputType.TYPE_CLASS_TEXT | (b ? InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD : InputType.TYPE_TEXT_VARIATION_PASSWORD));
			}
		});

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
			dialog.show();
	}
}
