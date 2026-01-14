CPP = g++

ULIBS = ssl crypto sqlite3
LIBS = $(ULIBS:%=-l%)

SRC_CPP = main.cpp
OBJS = $(SRC_CPP:.cpp=.o)
EXE = main

.PHONY: all
all: $(EXE) $(OBJS)

$(EXE): $(OBJS)
	$(CPP) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CPP) -c -o $@ $<

.PHONY: run
run:
	./$(EXE)

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS)
