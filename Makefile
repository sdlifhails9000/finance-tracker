CPP = clang++
CPPFLAGS =

INC = ./
CPPFLAGS += -I$(INC) 

ULIBS = ssl crypto sqlite3
LIBS = $(ULIBS:%=-l%)

SRC_CPP = main.cpp categories.cpp filters.cpp moneypools.cpp transactions.cpp utils.cpp ui.cpp users.cpp
OBJS = $(SRC_CPP:.cpp=.o)
EXE = main

.PHONY: all
all: $(EXE) $(OBJS)

$(EXE): $(OBJS)
	$(CPP) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $<

.PHONY: run
run:
	./$(EXE)

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS)
