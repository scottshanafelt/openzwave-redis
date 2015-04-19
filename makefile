CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs)

SRCS=openzwave-redis.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: openzwave-redis

openzwave-redis: $(OBJS)
	$(CXX) $(LDFLAGS) -o openzwave-redis $(OBJS) $(LDLIBS)

openzwave-redis.o: openzwave-redis.cpp openzwave-redis.hpp

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) openzwave-redis
