
CXX := g++
CXXFLAGS := -Wall -Wextra -g

LIBS = -lxcb

TARGETS = helium hctrl
OBJS = helium.o client.o util.o

all: $(TARGETS)

%.o: %.cpp
	$(CXX) -c $<

helium: $(OBJS)
	$(CXX) -o $@ $(LIBS) $(CXXFLAGS) $^

hctrl: hctrl.o
	$(CXX) -o $@ $(LIBS) $(CXXFLAGS) $^

clean:
	@echo "removing object files:"
	rm -rf *.o
	rm -f $(TARGETS)

server:
	@echo "starting server"
	@pidof Xephyr &> /dev/null || Xephyr -screen 800x600 :1 &> /dev/null & 

run: server helium
	DISPLAY=:1 ./helium
