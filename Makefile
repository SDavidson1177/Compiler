CXX=gcc
CXXFLAGS=-std=c++14
DEPS = 
OBJ = start.o

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

all: $(OBJ)
	$(CXX) -o lang $^ $(CXXFLAGS)