# compiles the program. -o is the ouput binary name
# -l is the libraries needed
all:
	cc wm.c structs.h -o wndwmngr -lX11 -lXext

# deletes excess files
clean:
	rm wndwmngr
