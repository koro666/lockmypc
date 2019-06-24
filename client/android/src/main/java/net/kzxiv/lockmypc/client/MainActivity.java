package net.kzxiv.lockmypc.client;

import android.os.*;
import android.support.design.widget.*;
import android.support.v7.app.*;
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

		// TODO:

		FloatingActionButton fab = findViewById(R.id.add_button);
		fab.setOnClickListener(new View.OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				// TODO:
			}
		});
	}
}
