X11_INCLUDE?=/usr/include
PREFIX?=/usr

override CFLAGS +=-s -I${X11_INCLUDE}
override LDFLAGS +=-L${PREFIX}/lib -lxcb -lxcb-ewmh
#-lxcb-randr

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: helium

# compile just the window manager
helium: $(OBJ)
	$(CC) $(OBJ) -o $@ $(CFLAGS) $(LDFLAGS)
	rm $(OBJ)

# compile all files required for linking
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm helium