CXX		= g++
INCLUDES        = -I /usr/local/include -I /usr/local/include/openzwave/
CXXFLAGS        = -g -Wall ${INCLUDES}
OBJS            = main.o
PROG            = test
LIBS            = -L /usr/local/lib -l pthread -l hiredis -l openzwave -l platform

all:		$(PROG)

run:
	./$(PROG)

${PROG}:	$(OBJS)
	$(CXX) $(INCLUDES) -o $(PROG) $(OBJS) $(LIBS)

clean:
	rm -fr *.o $(PROG)
