# makefile for magicfilter-2

OBJS=getline.o xyzzy.o actions.o magic.o trace.o reject.o options.o
LIB=libmf.a
LFLAGS+=-Lfile -L.
CFLAGS+= -Ifile -DHAS_OPTIONS
LIBES=-lmf -lfile -lbasis
RANLIB=:
PROGS=mpp traditional magicfilter

all: $(PROGS)

mpp: magic.m4 mkmpp
	./mkmpp magic.m4

traditional: $(LIB) fe.c
	$(CC) $(LFLAGS) -DNOFE -o traditional fe.c $(LIBES)


magicfilter: $(LIB) fe.c
	$(CC) $(LFLAGS) -o magicfilter fe.c $(LIBES)

$(OBJS): rule.h magicfilter.h

$(LIB): $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

magic.c: magic.m4
	./mkmagic magic.m4

clean:
	rm -f $(OBJS) magic.c $(PROGS) $(LIB)
