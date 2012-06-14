PROG = db_test

CC	= $(CROSS_COMPILE)gcc
LD	= $(CROSS_COMPILE)gcc
CFLAGS	= -Wall -Isqlite-autoconf-3071300
LIBS	= -lpthread -ldl


OBJS := db_test.o sqlite-autoconf-3071300/sqlite3.o

all: $(PROG)

$(PROG): $(OBJS)
	gcc $(OBJS) -o $@ $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-$(RM) $(OBJS) db_test
