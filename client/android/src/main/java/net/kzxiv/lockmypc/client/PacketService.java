package net.kzxiv.lockmypc.client;

import android.app.*;
import android.content.*;
import android.os.*;

public class PacketService extends IntentService
{
	public static final String EXTRA_HOST = "net.kzxiv.lockmypc.client.packet.HOST";
	public static final String EXTRA_PORT = "net.kzxiv.lockmypc.client.packet.PORT";
	public static final String EXTRA_SECRET = "net.kzxiv.lockmypc.client.packet.SECRET";
	public static final String EXTRA_RESULT = "net.kzxiv.lockmypc.client.packet.RESULT";
	public static final String EXTRA_ERROR = "net.kzxiv.lockmypc.client.packet.ERROR";

	public PacketService()
	{
		super("LockMyPC Packet Service");
	}

	@Override
	protected void onHandleIntent(Intent intent)
	{
		String host = intent.getStringExtra(EXTRA_HOST);
		String port = intent.getStringExtra(EXTRA_PORT);
		String secret = intent.getStringExtra(EXTRA_SECRET);

		ResultReceiver receiver = intent.getParcelableExtra(EXTRA_RESULT);

		int result = 0;
		String error = "";
		try
		{
			sendPacket(host, port, secret);
		}
		catch (Exception ex)
		{
			result = 1;
			error = ex.getMessage();
		}

		Bundle bundle = new Bundle();
		bundle.putString(EXTRA_ERROR, error);

		receiver.send(result, bundle);
	}

	private static final void sendPacket(String host, String port, String secret) throws Exception
	{
		// TODO:
		throw new Exception("Not implemented yet.");
	}
}
