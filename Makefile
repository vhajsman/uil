CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -I/usr/include -g
LDFLAGS = -lboost_program_options

UIL_SRC = $(wildcard src/*.cpp src/*/*.cpp)
UIL_OBJ = $(UIL_SRC:.cpp=.o)
UIL_BIN = build/uil

.PHONY: all clean

all: $(UIL_BIN)

$(UIL_BIN): $(UIL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(UIL_OBJ) $(UIL_BIN)
