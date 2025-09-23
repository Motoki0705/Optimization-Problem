CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2 -Iinclude -Isrc
AR ?= ar
ARFLAGS ?= rcs

LIB = libgd.a
SRCS =     src/model.c     src/loss.c     src/optim.c     src/trainer.c     src/callbacks.c     src/utils.c
OBJS =     src/model.o     src/loss.o     src/optim.o     src/trainer.o     src/callbacks.o     src/utils.o

all: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

examples: $(LIB)
	$(CC) $(CFLAGS) -o examples/gd1d.o examples/gd1d.c $(LIB) -lm
	$(CC) $(CFLAGS) -o examples/gd2d.o examples/gd2d.c $(LIB) -lm
clean:
	rm -f $(OBJS) $(LIB) examples/gd_quadratic examples/gd_sincos

.PHONY: all clean examples
