CROSS_COMPILE ?=

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)gcc
CFLAGS	:= -W -Wall -Wno-multichar -O3 #-Werror
CFLAGS	+= -I../db/sqlite-autoconf-3071300
LDFLAGS :=
LIBS	:= -lusb -lpthread -ldl

OBJ_CM160 := cm160.o usb_utils.o db.o ../db/sqlite-autoconf-3071300/sqlite3.o

all: cm160

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cm160: $(OBJ_CM160)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_CM160) $(LIBS)

clean:
	-$(RM) *.o
	-$(RM) cm160
