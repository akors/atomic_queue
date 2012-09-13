
CXXFLAGS += -std=c++11 -I..

ALL_TESTS := $(patsubst %.cpp,%,$(wildcard *.cpp))


all: $(ALL_TESTS)


$(ALL_TESTS): ../atomic_queue.hpp

clean:
	$(RM) $(ALL_TESTS)
	$(RM) *.o

