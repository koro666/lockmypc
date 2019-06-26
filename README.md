LockMyPC
========

LockMyPC allows to remotely lock the screen of a computer from an Android device.

It consists of a client and server part.

How to Use
----------

### Server

First, the server must be running and configured on a computer.

#### Windows

A binary is provided in the Releases section, or it can be built from the sources in the `server/win32` directory.

Running it will show a notification icon. Right-clicking it will show a menu that allows to configure or exit the server.

Configuration is stored in `%LOCALAPPDATA%\LockMyPC\lmpc.ini` by default. If a file of the same name exists next to the executable, it will use that instead.

#### Linux

The binary can be built by running `make` inside the `server/linux` directory.

An example configuration file is provided with the source, and should be copied to `~/.config/lmpc.conf`. This location can be changed by setting the `XDG_CONFIG_DIR` environment variable.

### Client

Second, the client must be installed and configured.

#### Android

A binary is provided in the Releases section, or it can be built from the sources in the `client/android` directory.

Upon startup, the client will show an empty list. Machines can be added by tapping the **+** button and entering the hostname or address, port and secret key. Tapping on a machine in the list will send the lock command.

Security
--------

A shared *secret key* must be known between the client and server. If the client does not know the secret key, it will not be able to generate packets that the server deems valid.

Additionally, each sent packet has a valid duration of one minute, after which the server will reject it as invalid, to prevent replay attacks.

The details of the protocol are outlined below.

Protocol
--------

The client sends a single UDP packet to the server. This packet structure is as follows:

    struct
    {
        uint8_t signature[4];
        int32_t time;
        uint8_t hash[32];
    };

 * The signature is always the four bytes `{ 'L', 'O', 'C', 'K' }`.
 * The time is the current time, encoded as 32-bit Unix time, in network byte order.
 * The hash is a SHA256 computed from the bytes of the time concatenated with the bytes of the secret key as UTF-8 bytes.

Upon reception of the packet, the server will:
 
 * Validate that the signature is correct.
 * Validate that the time is within one minute of its own current time.
 * Recompute the hash and validate that it is the same one.

If all those steps pass, the server will deem the packet as valid and initiate locking the screen.

In order to keep the protocol simple, the server does not reply with a packet of its own; the client has no way of telling if the packet reached the server and/or if the server accepted it.
