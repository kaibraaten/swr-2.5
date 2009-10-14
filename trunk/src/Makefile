CC = gcc
BINARY = swr
WFLAGS = -Werror -Wall
#WFLAGS =  -Wall -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Werror -Wconversion -Wwrite-strings
CCFLAGS = -g3 -MD $(WFLAGS) -I.
LFLAGS = -lcrypt -lm
DEPS = 
ODIR = obj
_OBJ = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	build.o clans.o comm.o const.o db.o fight.o \
	handler.o hashstr.o id.o interp.o magic.o makeobjs.o \
	misc.o mud_comm.o mud_prog.o player.o requests.o \
	reset.o save.o shops.o skills.o special.o tables.o track.o update.o \
	space.o morespace.o swskills.o planets.o vote.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all:		 libs $(BINARY)

libs:
#		make -C esock

$(BINARY):	$(OBJ)
		$(CC) -o $@ $^ $(LFLAGS)

$(ODIR)/%.o:	 %.c $(DEPS)
		$(CC) -c -o $@ $< $(CCFLAGS)

-include *.d

clean :
		-rm -f $(BINARY) *~ $(ODIR)/*.o core*
