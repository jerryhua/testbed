Description

This module provide a method to change the duration of tcp TIMEWAIT state. But it is not recommend to do this for that it will cause unexpected behavior if the last ack lost.

Noted: It was not test when tcp_tw_recycle is set simultaneously.

Useage:
	make
	insmod tcp_tw.ko
	cat /proc/sys/net/ipv4/tcp_timewait_2msl
	echo 30 > /proc/sys/net/ipv4/tcp_timewait_2msl 

The tcp_timewait_2msl will reset to system default value when module exit.