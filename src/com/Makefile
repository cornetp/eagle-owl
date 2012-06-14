CROSS_COMPILE ?=

CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)gcc
CFLAGS	:= -W -Wall -Wno-multichar -O3 #-Werror
LDFLAGS :=
LIBS	:= -lusb

OBJ_CM160 := cm160.o

all: cm160

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cm160: $(OBJ_CM160)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_CM160) $(LIBS)

clean:
	-$(RM) *.o
	-$(RM) cm160
