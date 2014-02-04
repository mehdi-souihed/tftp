Trivial File Transfer Protocol
====

This protocol can be considered the little cousin of the bigger File Transfer Protocol.

It implements RFC 1350 http://www.ietf.org/rfc/rfc1350.txt

Any questions, suggestions, remarks are more than welcome


==========================================================

INSTALL :

make

Two executables will be created :

  - triviald : the TFTP server
  - client/trivial : the TFTP client

==========================================================

Test script :

go to the test/ folder and run test.pl


==========================================================
How to run it :

First you have to launch the server via the terminaland specify a port where to listen, for instance :

./triviald -p 5001 &

Then you have to run the client specifying the file ,the operation, the host and the port, for instance :

./trivial -p 5001 -H 127.0.0.1 -w dummy_file

The above will send dummy_file to the server and save it in the current directory.

You can have help on the options available with the -h option







