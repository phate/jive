# Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
# Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

CPPFLAGS+=-Iinclude
CFLAGS+=-Wall --std=c++14 -xc++

# RVSDG core
LIBJIVE_SRC = \
	src/common.c \
	src/rvsdg/binary.c \
	src/rvsdg/control.c \
	src/rvsdg/controltype.c \
	src/rvsdg/equivalence.c \
	src/rvsdg/gamma.c \
	src/rvsdg/gate.c \
	src/rvsdg/graph.c \
	src/rvsdg/label.c \
	src/rvsdg/negotiator.c \
	src/rvsdg/node-normal-form.c \
	src/rvsdg/node.c \
	src/rvsdg/nullary.c \
	src/rvsdg/operation.c \
	src/rvsdg/phi.c \
	src/rvsdg/region.c \
	src/rvsdg/resource.c \
	src/rvsdg/simple-normal-form.c \
	src/rvsdg/simple-node.c \
	src/rvsdg/statemux.c \
	src/rvsdg/splitnode.c \
	src/rvsdg/structural-normal-form.c \
	src/rvsdg/structural-node.c \
	src/rvsdg/theta.c \
	src/rvsdg/tracker.c \
	src/rvsdg/traverser.c \
	src/rvsdg/type.c \
	src/rvsdg/unary.c \

#evaluation
LIBJIVE_SRC += \
	src/evaluator/eval.c \
	src/evaluator/literal.c \

# visualization
LIBJIVE_SRC += \
	src/util/buffer.c \
	src/util/callbacks.c \
	src/view.c \

# bitstrings
LIBJIVE_SRC += \
	src/types/bitstring/arithmetic.c \
	src/types/bitstring/bitoperation-classes.c \
	src/types/bitstring/comparison.c \
	src/types/bitstring/concat.c \
	src/types/bitstring/constant.c \
	src/types/bitstring/slice.c \
	src/types/bitstring/type.c \
	src/types/bitstring/value-representation.c \

# floats
LIBJIVE_SRC += \
	src/types/float/arithmetic.c \
	src/types/float/comparison.c \
	src/types/float/fltconstant.c \
	src/types/float/fltoperation-classes.c \
	src/types/float/flttype.c \

# records
LIBJIVE_SRC += \
	src/types/record/rcdgroup.c \
	src/types/record/rcdselect.c \
	src/types/record/rcdtype.c \

# unions
LIBJIVE_SRC += \
	src/types/union/unnchoose.c \
	src/types/union/unntype.c \
	src/types/union/unnunify.c \

# functions
LIBJIVE_SRC += \
	src/types/function/fctapply.c \
	src/types/function/fctlambda.c \
	src/types/function/fcttype.c \

# arch definitions
LIBJIVE_SRC += \
	src/arch/address-transform.c \
	src/arch/address.c \
	src/arch/addresstype.c \
	src/arch/call.c \
	src/arch/compilate.c \
	src/arch/dataobject.c \
	src/arch/immediate-node.c \
	src/arch/immediate-type.c \
	src/arch/instruction.c \
	src/arch/label-mapper.c \
	src/arch/load-normal-form.c \
	src/arch/load.c \
	src/arch/memlayout-simple.c \
	src/arch/memlayout.c \
	src/arch/memorytype.c \
	src/arch/registers.c \
	src/arch/regselector.c \
	src/arch/regvalue.c \
	src/arch/sizeof.c \
	src/arch/stackslot.c \
	src/arch/store.c \
	src/arch/subroutine.c \
	src/arch/subroutine/nodes.c \

include src/backend/i386/Makefile.sub

SOURCES += $(LIBJIVE_SRC)

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

HEADERS = $(shell find include -name "*.h")

libjive.a: $(patsubst %.c, %.la, $(LIBJIVE_SRC))
libjive.so: $(patsubst %.c, %.lo, $(LIBJIVE_SRC))

doc:
	doxygen doxygen.conf

clean: depclean
	find . -name *.o -o -name *.lo -o -name *.la -o -name *.so -o -name *.a | xargs rm -rf
	rm -rf $(TESTPROGS)
	rm -rf a.out

include tests/Makefile.sub

%.la: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.lo: %.c
	$(CC) -c -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.a:
	rm -f $@
	ar clqv $@ $^
	ranlib $@

%.so:
	$(CC) -shared -o $@ $^

.dep/%.la.d: %.c
	@mkdir -p $(dir $@)
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS) -MT $(<:.c=.la) -MP -MF $@ $<
	@echo MAKEDEP $<

.dep/%.lo.d: %.c
	@mkdir -p $(dir $@)
	@$(CC) -MM -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -MT $(<:.c=.lo) -MP -MF $@ $<

depclean:
	rm -rf .dep

DEPEND = $(patsubst %.c, .dep/%.la.d, $(SOURCES)) $(patsubst %.c, .dep/%.lo.d, $(SOURCES))
depend: $(DEPEND)
ifeq ($(shell if [ -e .dep ] ; then echo yes ; fi),yes)
-include $(DEPEND)
endif

.PHONY: doc

ifeq ($(shell if [ -e .Makefile.override ] ; then echo yes ; fi),yes)
include .Makefile.override
endif
