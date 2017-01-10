Description

This module provide a method to change the duration of tcp TIMEWAIT state. But it is not recommend to do this for that it will cause unexpected behavior if the last ack lost.


Noted: 
This module will not take effect when tcp_tw_recycle is set simultaneously.

It works well on kernel version 2.6.32 and not test on other kernel versions yet.

Useage:

	make

	insmod tcp_tw.ko

	cat /proc/sys/net/ipv4/tcp_tw_timewait

	echo 30 > /proc/sys/net/ipv4/tcp_tw_timewait

It means set length of tcp time_wait as 30 seconds.

The tcp_tw_timewait will reset to system default value when module exit.


CONTACT

 Email: huaguibin@gmail.com

If you have any questions, please email me. 

