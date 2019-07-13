# compiles the program. -o is the ouput binary name
# -l is the libraries needed
all:
	cc wm.c structs.h -o helium -lX11 -lXext
	cc ipc.c -o heliumc
c:
	cc control.c ipc.h -o heliumc

# deletes excess files
clean:
	rm helium
	rm heliumc
