CROSS_COMPILE ?=

CM160 = cm160

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)gcc
CFLAGS	:= -W -Wall -Wno-multichar -O3 #-Werror
LDFLAGS :=
LIBS	:= -lusb -lconfig -lmosquitto -lpthread -ldl

OBJ_CM160 := cm160.o usb_utils.o demonize.o

all: $(CM160) $(DB_IMPORT)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(CM160): $(OBJ_CM160)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_CM160) $(LIBS)

clean:
	-$(RM) *.o
	-$(RM) $(CM160)
