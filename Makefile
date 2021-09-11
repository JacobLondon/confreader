
CC     = gcc
CWARNS = -Wall -Wextra
CFLAGS = $(CWARNS) -std=c99

CONF_PRE_T = test

CONF_O = confreader.o test.o

ifeq ($(OS),Windows_NT)
	CONF_T  = $(CONF_PRE_T).exe
else
	CONF_T  = $(CONF_PRE_T)
endif

.PHONY: clean

all: debug

release: CFLAGS += -O2
release: $(CONF_T)

debug: CFLAGS += -ggdb -O0
debug: $(CONF_T)

$(CONF_T): $(CONF_O)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o $(CONF_T)

# find . -type f | grep '\.c$' | xargs gcc -MM
confreader.o: confreader.c confreader.h
test.o: test.c
