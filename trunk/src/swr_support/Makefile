ODIR = obj
_OBJ = bet.o file_io.o hashstr.o misc.o os_unix.o sha256.o string_handling.o time_fun.o vector3.o
OBJ  = $(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS =
#CC = gcc
WFLAGS =  -Wall -Wshadow -Wformat-security -Winline -Wpointer-arith -Wcast-align -Wredundant-decls -Werror -Wwrite-strings
CFLAGS = -O2 -MMD -pedantic -ansi $(GCC_MARCH) $(WFLAGS) -I.

ifndef CYGWIN
CFLAGS += -fPIC

#Comment out next line to link as static library
LINK_SHARED_LIBRARY = 1
endif

ifdef LINK_SHARED_LIBRARY
LIB_EXT = so
else
LIB_EXT = a
endif

LIBNAME = libswr_support.$(LIB_EXT)

all:		$(LIBNAME)

$(ODIR)/%.o:	%.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

$(LIBNAME):	$(OBJ)
ifdef LINK_SHARED_LIBRARY
		$(CC) -shared -Wl,-soname,$(LIBNAME) -o $(LIBNAME) $^
else
		ar cru $@ $^
		ranlib $@
endif

clean:
		rm -f *.so *.a *~ core* $(ODIR)/*.o $(ODIR)/*.d

-include $(ODIR)/*.d

mos:
		$(MAKE) -f Makefile.MOS

