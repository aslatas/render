# Assign files to wildcards
CPP_SOURCES = src/main.cpp 
HEADERS = $(wildcard ./src/*.h)
INCLUDE = -I../include -I../ext

# Compile flags
C = g++
CFLAGS = -std=c++17 -g -Wall -w

# Convert .cpp files to .o to give a list of object
# files to build
OBJ = ${CPP_SOURCES:.cpp=.o}

# Default make target
all: main.exe

run: main.exe
	./main.exe

main.exe : $(OBJ)
	g++ $(INCLUDE) -o $@ $^

%.o : %.cpp $(HEADERS)
	$(C) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean: 
	rm -rf *.o *.exe src/*.o
