# compiles the program. -o is the ouput binary name
# -l is the libraries needed
all:
	cc -g wm.c structs.h ipc.h resources.h -o helium -I/usr/include/freetype2 -lX11 -lXext -lm -lXft
	cc -g control.c ipc.h structs.h -o heliumc -lX11

# deletes excess files
clean:
	rm helium
	rm heliumc
