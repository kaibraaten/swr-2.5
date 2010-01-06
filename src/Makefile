BINARY = swr
WFLAGS = -Werror -Wall -Wformat-security -Wpointer-arith -Wcast-align -Wwrite-strings -Wshadow

#Comment out the next line if your system doesn't have dlsym functionality.
HAS_DLSYM = -DSWR2_HAS_DLSYM

CCFLAGS = -g3 -MMD $(WFLAGS) -Iswr_support $(HAS_DLSYM)
LFLAGS = -lm -Lswr_support -lswr_support -Wl,-rpath,./src/swr_support

ifdef HAS_DLSYM
LFLAGS += -ldl -rdynamic
endif

DEPS = 
ODIR = obj
_OBJ = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
	build.o clans.o comm.o const.o copyover.o db.o fight.o \
	handler.o interp.o magic.o makeobjs.o \
	misc.o morespace.o mud_comm.o mud_prog.o nanny.o planets.o \
	player.o reset.o save.o shops.o skills.o space.o \
	special.o swskills.o \
	tables.o track.o update.o vector3_aux.o vote.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all:		 libs $(BINARY)

libs:
		$(MAKE) -C swr_support

$(BINARY):	$(OBJ)
		@echo "Linking" $(BINARY)
		@$(CC) -o $@ $^ $(LFLAGS)
		@mv $(BINARY) ../
		@echo "Done."

$(ODIR)/%.o:	 %.c $(DEPS)
		@echo "Compiling" $<
		@$(CC) -c -o $@ $< $(CCFLAGS)

-include $(ODIR)/*.d

clean :
		-rm -f $(BINARY) *~ $(ODIR)/*.o core* $(ODIR)/*.d

mos:
		$(MAKE) -f Makefile.MOS

