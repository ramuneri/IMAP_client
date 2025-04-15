![alt text](image.png)

## L2B

Internet Message Access Protocol version 4 - LIST Command Extensions

LIST and LSUB - commands for listing mailboxes.

As wehave added extensions, such as Mailbox Referrals, that have required
specialized lists we have had to expand the number of list commands,
since each extension must add its function to both LIST and LSUB, and
these commands are not, as they are defined, extensible.  If we've
needed the extensions to work together, we've had to add a set of
commands to mix the different options, the set increasing in size
with each new extension.  This document describes an extension to the
base LIST command that will allow these additions to be done with
mutually compatible options to the LIST command, avoiding the
exponential increase in specialized list commands.

----------------------------------------

Using the sockets API, create an IMAP client part of the protocol.
Regarding the client for testing: it is (almost) always possible to download a good "server" or "client" for demonstration purposes.

I will use C language.

----------------------------------------

cd /mnt/c/Users/ramun/Desktop/netw/IMAP_client

gcc -o imap_client imap_client.c
./imap_client

----------------------------------------

Is Dovecot running?
sudo systemctl status dovecot

Start Dovecot:
sudo systemctl start dovecot

Confirm that server is listening: 
ss -tlnp | grep 143

Restart DoveCot:
sudo systemctl restart dovecot

----------------------------------------

testuser abc
testuser2 abc

- Running the Dovecot IMAP server locally

Message example (send just from Ubundu terminal):
echo "Test message content" | mail testuser

----------------------------------------

A001 ... meaning:
- IMAP requires every command sent by the client to be prefixed with a unique "tag"
- The server echoes back the same tag so the client knows - “This response goes with that request.”
- Tags are chosen by the client (server never creates them)
- Technically we can not reuse tags