CXX		= g++
INCLUDES        = -I /usr/local/include -I /usr/local/include/openzwave/
CXXFLAGS        = -g -Wall ${INCLUDES}
SRCROOT		= ./src/
BLDROOT		= ./build/
OBJS		= main.o component_ozw.o component_redis.o
PROG            = ozw-redis
LIBS            = -L /usr/local/lib -l pthread -l hiredis -l openzwave

all: $(PROG)

${PROG}: $(OBJS)
	$(CXX) $(INCLUDES) -o $(PROG) $(addprefix $(BLDROOT),$(OBJS)) $(LIBS)

main.o: $(SRCROOT)main.cpp
	$(CXX) $(INCLUDES) -c $(SRCROOT)main.cpp -o $(BLDROOT)main.o

component_ozw.o: $(SRCROOT)component_ozw.cpp
	$(CXX) $(INCLUDES) -c $(SRCROOT)component_ozw.cpp -o $(BLDROOT)component_ozw.o

component_redis.o: $(SRCROOT)component_redis.cpp
	$(CXX) $(INCLUDES) -c $(SRCROOT)component_redis.cpp -o $(BLDROOT)component_redis.o

clean:
	rm -fr *.o $(PROG) $(SRCROOT)*.o $(BLDROOT)*.o