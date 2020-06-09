
CXX := g++
CXXFLAGS := -Wall -g

LIBS = -lxcb -lxcb-ewmh

TARGETS = helium hctrl

SRCDIR := src
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGETS)

test:
	@echo $(SRCS)
	@echo $(OBJS)

$(SRCDIR)/%.o: $(SRCDIR)%.cpp
	$(CXX) -c $< -o $@

helium: $(OBJS)
	$(CXX) -o $@ $(LIBS) $(CXXFLAGS) $^

hctrl: hctrlsrc/hctrl.o
	$(CXX) -o $@ $(LIBS) $(CXXFLAGS) $^

hctrlsrc/hctrl.o:
	$(CXX) -c hctrlsrc/hctrl.cpp -o $@

clean:
	@echo "removing object files:"
	rm -rf $(SRCDIR)/*.o
	rm -rf hctrlsrc/*.o
	rm -f $(TARGETS)

server:
	@echo "starting server"
	@pidof Xephyr &> /dev/null || Xephyr -screen 800x600 :1 &> /dev/null & 

sxhkd:
	@echo "starting sxhkd"
	@pkill sxhkd &> /dev/null
	@DISPLAY=:1 sxhkd -c examples/sxhkdrc  &> /dev/null & 

run: server sxhkd hctrl helium
	DISPLAY=:1 ./helium
