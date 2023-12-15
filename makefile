CC = g++
CPPFLAGS = -Wall

PROG = ifacepicker

all: $(PROG)

$(PROG): main.cpp
	$(CC) $(CPPFLAGS) -o $(PROG) main.cpp

clean:
	rm -f $(PROG)
