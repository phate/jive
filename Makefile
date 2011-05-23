CPPFLAGS+=-Iinclude -iquote include/private
CFLAGS+=-Wall -Werror -Wfatal-errors -g

# VSDG core
LIBJIVE_SRC = src/context.c \
	src/vsdg/node.c src/vsdg/region.c src/vsdg/graph.c src/vsdg/notifiers.c src/vsdg/variable.c src/vsdg/resource.c \
	src/vsdg/gate-interference.c src/vsdg/control.c \
	src/vsdg/operators.c \
	src/vsdg/basetype.c src/vsdg/statetype.c src/vsdg/valuetype.c src/vsdg/anchortype.c src/vsdg/controltype.c \
	src/vsdg/functiontype.c src/vsdg/function.c \
	src/vsdg/recordtype.c src/vsdg/recordlayout.c src/vsdg/record.c \
	src/vsdg/traverser.c \
	src/vsdg/negotiator.c \
	src/vsdg/substitution.c \
	src/vsdg/sequence.c \
	src/vsdg/label.c \
	src/vsdg/objdef.c \

# visualization
LIBJIVE_SRC += \
	src/util/textcanvas.c src/util/buffer.c \
	src/view/nodeview.c src/view/regionview.c src/view/graphview.c src/view.c \
	src/view/reservationtracker.c  \

# bitstrings
LIBJIVE_SRC += \
	src/bitstring/type.c src/bitstring/constant.c src/bitstring/symbolic-constant.c src/bitstring/slice.c src/bitstring/concat.c src/bitstring/arithmetic.c

# arch definitions
LIBJIVE_SRC += \
	src/arch/registers.c src/arch/instruction.c src/arch/stackframe.c \
	src/arch/regselector.c

# shaper
LIBJIVE_SRC += \
	src/regalloc/shaped-graph.c src/regalloc/shaped-region.c src/regalloc/shaped-variable.c src/regalloc/shaped-node.c src/regalloc/xpoint.c
# register allocator
LIBJIVE_SRC += \
	src/regalloc.c src/regalloc/shape.c src/regalloc/color.c src/regalloc/fixup.c src/regalloc/auxnodes.c src/regalloc/reroute.c

include src/backend/i386/Makefile.sub

# LIBJIVE_SRC = \
# 	\
# 	src/bitstring/multiop.c   \
# 	\
# 	src/regalloc/shaping-traverser.c src/regalloc/active-place-tracker.c src/regalloc/shape.c src/regalloc/color.c src/regalloc/regreuse.c src/regalloc/auxnodes.c src/regalloc/fixup.c src/regalloc/stack.c src/regalloc.c \
# 	\
# 	src/backend/i386/registerset.c\
# 	src/backend/i386/instructionset.c\
# 	src/backend/i386/machine.c\
# 	src/backend/i386/stackframe.c\
# 	src/backend/i386/subroutine.c\

# LIBJIVE_SRC = src/context.c \
# 	src/vsdg/basetype.c src/vsdg/statetype.c src/vsdg/valuetype.c src/vsdg/controltype.c \
# 	src/vsdg/crossings.c src/vsdg/resource-interference.c src/vsdg/regcls-count.c \
# 	src/vsdg/cut.c src/vsdg/region.c src/vsdg/control.c \
# 	src/vsdg/node.c src/vsdg/graph.c src/vsdg/traverser.c src/vsdg/notifiers.c src/vsdg/normalization.c \
# 	src/util/textcanvas.c src/util/buffer.c \
# 	src/view/nodeview.c src/view/regionview.c src/view/reservationtracker.c src/view/graphview.c src/view.c \
# 	\
# 	src/bitstring/type.c src/bitstring/multiop.c src/bitstring/constant.c src/bitstring/symbolic-constant.c src/bitstring/slice.c src/bitstring/negate.c \
# 	\
# 	src/regalloc/shaping-traverser.c src/regalloc/active-place-tracker.c src/regalloc/shape.c src/regalloc/color.c src/regalloc/regreuse.c src/regalloc/auxnodes.c src/regalloc/fixup.c src/regalloc/stack.c src/regalloc.c \
# 	\
# 	src/arch/instruction.c src/arch/registers.c src/arch/stackframe.c src/arch/subroutine.c \
# 	\
# 	src/backend/i386/registerset.c\
# 	src/backend/i386/instructionset.c\
# 	src/backend/i386/machine.c\
# 	src/backend/i386/stackframe.c\
# 	src/backend/i386/subroutine.c\

all: check libjive.a # libjive.so

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
