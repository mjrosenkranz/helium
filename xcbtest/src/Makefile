CFLAGS = -Wall -I/opt/local/include
LDFLAGS = -L/opt/local/lib -lxcb

all: helium

helium: main.o client.o
	g++ ${CFLAGS} $^ -o $@ ${LDFLAGS}

%.o: %.cpp
	g++ -c $< -o $@ ${CFLAGS} ${LDFLAGS}

opts:
	@echo "cflag ${CFLAGS}"
	@echo "ldflag ${LDFLAGS}"

clean:
	rm -f *.o
	rm -f helium

run: all
	@Xephyr -br -ac -noreset -screen 800x600 :1 &>/dev/null &
	@sleep 1
	@DISPLAY=:1 ./helium
	@killall Xephyr
