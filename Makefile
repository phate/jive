CPPFLAGS+=-Iinclude -iquote include/private
CFLAGS+=-Wall -Werror -Wfatal-errors -g

LIBJIVE_SRC=src/context.c src/graph.c src/graphdebug.c src/bitstring.c src/loadstore.c src/instruction.c src/subroutine.c src/buffer.c src/passthrough.c src/regalloc/cut.c src/regalloc/shape.c src/regalloc/assign.c src/regalloc/util.c src/regalloc/spill.c src/regalloc.c

#src/ra-common.c src/ra-graphcut-cache.c src/ra-shape.c src/regalloc.c

LIBJIVE_SRC += src/i386/machine.c src/i386/instructions.c src/i386/abi.c src/i386/encoding.c

all: check

libjive.a: $(patsubst %.c, %.la, $(LIBJIVE_SRC))
libjive.so: $(patsubst %.c, %.lo, $(LIBJIVE_SRC))

doc:
	doxygen doxygen.conf

clean:
	find . -name *.o -o -name *.lo -o -name *.la -o -name *.so -o -name *.a | xargs rm -rf
	rm -rf $(TESTPROGS)

include tests/Makefile.sub

%.la: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

%.lo: %.c
	$(CC) -c -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ $^

%.a:
	rm -f $@
	ar clqv $@ $^
	ranlib $@

%.so:
	$(CC) -shared -o $@ $^

.PHONY: doc
