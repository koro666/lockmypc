package net.kzxiv.lockmypc.client;

import android.app.*;
import android.content.*;
import android.os.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;
import java.security.*;

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

	private static final byte[] makePacket(String secret) throws IOException, NoSuchAlgorithmException
	{
		int now = (int)(System.currentTimeMillis() / 1000L);

		byte[] nowBytes = intToBytes(now);
		byte[] secretBytes = secret.getBytes(Charset.forName("UTF-8"));

		MessageDigest digest = MessageDigest.getInstance("SHA-256");
		digest.update(nowBytes);
		digest.update(secretBytes);
		byte[] hash = digest.digest();

		byte[] result = new byte[4 + nowBytes.length + hash.length];
		result[0] = 0x4c;
		result[1] = 0x4f;
		result[2] = 0x43;
		result[3] = 0x4b;

		System.arraycopy(nowBytes, 0, result, 4, nowBytes.length);
		System.arraycopy(hash, 0, result,4 + nowBytes.length, hash.length);
		return result;
	}

	private static final byte[] intToBytes(int input) throws IOException
	{
		ByteArrayOutputStream strm0 = null;
		DataOutputStream strm1 = null;
		try
		{
			strm0 = new ByteArrayOutputStream();
			strm1 = new DataOutputStream(strm0);

			strm1.writeInt(input);

			strm1.flush();
			return strm0.toByteArray();
		}
		finally
		{
			if (strm1 != null)
				strm1.close();
			if (strm0 != null)
				strm0.close();
		}
	}

	private static final void sendPacket(String host, String port, String secret) throws Exception
	{
		byte[] packet = makePacket(secret);
		sendPacket(host, port, packet);
	}

	private static final void sendPacket(String host, String port, byte[] data) throws IOException
	{
		InetAddress address = InetAddress.getByName(host);
		int iport = Integer.parseInt(port);

		DatagramSocket socket = null;
		try
		{
			socket = new DatagramSocket();

			DatagramPacket packet = new DatagramPacket(data, data.length, address, iport);
			socket.send(packet);
		}
		finally
		{
			if (socket != null)
				socket.close();
		}
	}
}
