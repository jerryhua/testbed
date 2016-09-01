Description

There are one TCP server and one TCP client, which used to test "rfc2525: 2.17 Failure to RST on close with data pending "

Server will send some data to client, where client didn't read it. Client will sleep several seconds before close. When client sleeping, we can use tcpdump to see the interactive between serve and client,and ss on client to see where the data hold on.






