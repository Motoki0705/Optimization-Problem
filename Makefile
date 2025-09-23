CC ?= cc
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
	$(CC) $(CFLAGS) -o examples/gd_quadratic examples/gd_quadratic.c $(LIB) -lm
	$(CC) $(CFLAGS) -o examples/gd_sincos examples/gd_sincos.c $(LIB) -lm

clean:
	rm -f $(OBJS) $(LIB) examples/gd_quadratic examples/gd_sincos

.PHONY: all clean examples
