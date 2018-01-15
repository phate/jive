# Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
# Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

CPPFLAGS+=-Iinclude
CFLAGS+=-Wall -Wpedantic --std=c++14

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
	src/rvsdg/notifiers.c \
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
	src/types/record.c \

# unions
LIBJIVE_SRC += \
	src/types/union.c \

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
	src/arch/instructionset.c \
	src/arch/label-mapper.c \
	src/arch/load.c \
	src/arch/memlayout-simple.c \
	src/arch/memlayout.c \
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
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.lo: %.c
	$(CXX) -c -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.a:
	rm -f $@
	ar clqv $@ $^
	ranlib $@

%.so:
	$(CC) -shared -o $@ $^

.dep/%.la.d: %.c
	@mkdir -p $(dir $@)
	@$(CXX) -MM $(CFLAGS) $(CPPFLAGS) -MT $(<:.c=.la) -MP -MF $@ $<
	@echo MAKEDEP $<

.dep/%.lo.d: %.c
	@mkdir -p $(dir $@)
	@$(CXX) -MM -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -MT $(<:.c=.lo) -MP -MF $@ $<

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
