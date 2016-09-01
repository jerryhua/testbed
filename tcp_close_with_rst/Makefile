#
CC      = gcc
CFLAGS  = -W -Wall 
#CFLAGS += -DBIGENDIAN
#CFLAGS += -DALIGNMENT_WORKAROUND

# When debugging, disable -O2 and enable -g.
#CFLAGS += -O2
#CFLAGS += -g

# Pass linker flags here
LDFLAGS =

SRCS := $(wildcard *.c) 
DEPS := $(patsubst %.c,%.d,$(SRCS)) 
OBJS := $(patsubst %.c,%.o,$(SRCS)) 


PROGRAMS := close_with_rst_server close_with_rst_client

all : $(PROGRAMS)

#
# Programs
#
close_with_rst_server : $(OBJS) $(DEPS)
	$(CC) close_with_rst_server.o -o close_with_rst_server $(LDFLAGS)

close_with_rst_client : $(OBJS) $(DEPS)
	$(CC) close_with_rst_client.o -o close_with_rst_client $(LDFLAGS)
	
#
# Objects
#
%.d:%.c 
	$(CC) -M $(CFLAGS) $< > $@  
 
%.o:%.c 
	$(CC) -c $(CFLAGS) $< -o $@ 

#
# Commands
#

strip : $(PROGRAMS)
	strip $(PROGRAMS)

clean :
	$(RM) *.o *.d $(PROGRAMS)
