CC=gcc
STD=c11
CFLAGS=-c -Wall -std=$(STD) -lm
LDFLAGS=-lm
SOURCES=main.c lexer.c parse.c ast.c symbol.c init.c math.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=arm
RM=rm -f

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY : clean
clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)
