CC	=gcc-3.4 -march=i386
CFLAGS	=-m32 -g -Wall -O -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -I../include
AS	=as --32
AR	=ar
LD	=ld
LDFLAGS =-m elf_i386
CPP	=gcc-3.4 -E -nostdinc -I../include

.c.o:
	$(CC) $(CFLAGS) \
	-c -o $*.o $<
.s.o:
	$(AS) -o $*.o $<
.c.s:
	$(CC) $(CFLAGS) \
	-S -o $*.s $<

OBJS	= memory.o page.o

all: mm.o

mm.o: $(OBJS)
	$(LD) -m elf_i386 -r -o mm.o $(OBJS)

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do $(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile

### Dependencies:
memory.o: memory.c ../include/signal.h ../include/sys/types.h \
  ../include/asm/system.h ../include/linux/sched.h \
  ../include/linux/head.h ../include/linux/fs.h ../include/linux/mm.h \
  ../include/linux/kernel.h
