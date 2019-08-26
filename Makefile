# compiles the program. -o is the ouput binary name
# -l is the libraries needed
#PREFIX?=/usr/X11R6
X11_INCLUDE?=/usr/include
PREFIX?=/usr

override CFLAGS +=-s -I${X11_INCLUDE} -I${X11_INCLUDE}/freetype2
override LDFLAGS +=-L${PREFIX}/lib -lX11 -lXext -lm -lXft

SRC = wm handle hints cwindow vector
OBJ = $(patsubst %, src/%.o, $(SRC))

all: helium ipc

%.o: %.c
	@echo $@
	$(CC) -o $@ -c $(CFLAGS) $<

helium: $(OBJ)
	$(CC) $(OBJ) -g -o helium $(CFLAGS) $(LDFLAGS)

ipc: src/ipc.o
	$(CC) src/ipc.o -g -o heliumc $(CFLAGS) $(LDFLAGS)

# deletes excess files
clean:
	rm -f helium
	rm -f heliumc
	rm -f src/*.o
