CC = gcc
BINARY = swr
WFLAGS = -Werror -Wall -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls -Wwrite-strings

CCFLAGS = -g3 -MMD $(WFLAGS) -I.
LFLAGS = -lcrypt -lm
DEPS = 
ODIR = obj
_OBJ = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	build.o clans.o comm.o const.o copyover.o db.o fight.o \
	handler.o hashstr.o interp.o magic.o makeobjs.o \
	misc.o mud_comm.o mud_prog.o nanny.o player.o \
	reset.o save.o shops.o skills.o special.o tables.o track.o update.o \
	space.o morespace.o swskills.o planets.o vector3.o vector3_aux.o vote.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all:		 libs $(BINARY)

libs:
#		make -C esock

$(BINARY):	$(OBJ)
		@echo "Linking" $(BINARY)
		@$(CC) -o $@ $^ $(LFLAGS)
		@mv $(BINARY) ../bin
		@echo "Done."

$(ODIR)/%.o:	 %.c $(DEPS)
		@echo "Compiling" $@
		@$(CC) -c -o $@ $< $(CCFLAGS)

-include $(ODIR)/*.d

clean :
		-rm -f $(BINARY) *~ $(ODIR)/*.o core*
