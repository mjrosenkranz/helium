# compiles the program. -o is the ouput binary name
# -l is the libraries needed
PREFIX?=/usr/X11R6
all:
	$(CC) -g wm.c structs.h ipc.h resources.h -o helium\
	-I$(PREFIX)/include -I/usr/include/freetype2 -L$(PREFIX)/lib \
	-lX11 -lXext -lm -lXft
	$(CC) -g control.c ipc.h structs.h -o heliumc -lX11

# deletes excess files
clean:
	rm helium
	rm heliumc
