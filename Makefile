CXX := clang
CXXFLAGS := -g -w -fPIC -Wall
INCLUDES := -I

OBJECTS := main.o myfunctions.o

main : $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o customshell

myfunctions : myfunctions.o
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o myfunctions
	
main.o : main.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c main.c -o main.o
	
myfunctions.o : myfunctions.c
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c myfunctions.c -o myfunctions.o
	
clean :
	rm -rf customshell *.o