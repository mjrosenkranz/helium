# compiles the program. -o is the ouput binary name
# -l is the libraries needed
all:
	cc wm.c structs.h ipc.h globals.h -o helium -lX11 -lXext -lm
	cc control.c ipc.h globals.h -o heliumc -lX11

r:
	cc resources.c globals.h -o resman -lX11

cr:
	rm resman

# deletes excess files
clean:
	rm helium
	rm heliumc
