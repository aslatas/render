SRC =     $(wildcard *.cpp)
CFLAGS=-I. -I../ext -I../include 
OBJ = ${SRC:.cpp=.o}
LINK_TARGET=main.exe

all: $(LINK_TARGET)
	echo Make build "all" done.

run: all
	./main.exe

$(LINK_TARGET) : $(OBJ)
	g++ -g -o $@ $^
	
%.o : %.cpp
	g++ -Wall -g -std=c++17 -c $(CFLAGS) $< -o $@

clean: 
	rm -rf *.o $(LINK_TARGET)
	echo Make clean done
