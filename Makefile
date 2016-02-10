CC = g++

CC_FLAGS:= -Wall -O3 -std=c++11
LD_FLAGS:= -lboost_filesystem -lboost_regex `pkg-config --libs libtorrent-rasterbar` 


SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

%.o: %.cpp
	@echo [CXX] $<
	$(CC) $(CC_FLAGS) -o $@ -c $<

client: $(OBJS)
	@echo [LD] $@
	$(CC) -o $@ $^ $(LD_FLAGS) $(CC_FLAGS)

.PHONY: clean all

all: client

clean:
	$(RM) $(OBJS)  client

