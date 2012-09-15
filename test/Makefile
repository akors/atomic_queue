
CXXFLAGS += -std=c++11 -I..

ifeq ($(CXX),clang++)
    CXXFLAGS += -stdlib=libc++
else
    CXXFLAGS += -D_GLIBCXX_USE_SCHED_YIELD
endif


ALL_TESTS := $(patsubst %.cpp,%,$(wildcard *.cpp))


all: $(ALL_TESTS)

clean:
	$(RM) $(ALL_TESTS)
	$(RM) *.o

