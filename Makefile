# Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
# Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

CPPFLAGS+=-Iinclude
CFLAGS+=-Wall -Werror -Wfatal-errors -g

# VSDG core
LIBJIVE_SRC = src/context.c \
	src/vsdg/node.c src/vsdg/region.c src/vsdg/graph.c src/vsdg/notifiers.c src/vsdg/variable.c src/vsdg/resource.c \
	src/vsdg/gate-interference.c src/vsdg/control.c \
	src/vsdg/gamma.c \
	src/vsdg/operators.c \
	src/vsdg/operators/nullary.c \
	src/vsdg/operators/unary.c \
	src/vsdg/operators/binary.c \
	src/vsdg/basetype.c src/vsdg/statetype.c src/vsdg/valuetype.c src/vsdg/anchortype.c src/vsdg/anchor.c \
	src/vsdg/controltype.c \
	src/vsdg/theta.c \
	src/vsdg/tracker.c \
	src/vsdg/traverser.c \
	src/vsdg/negotiator.c \
	src/vsdg/substitution.c \
	src/vsdg/sequence.c \
	src/vsdg/label.c \
	src/vsdg/objdef.c \
	src/vsdg/splitnode.c \
	src/vsdg/equivalence.c \
	src/vsdg/phi.c \

# visualization
LIBJIVE_SRC += \
	src/util/textcanvas.c src/util/buffer.c \
	src/view/nodeview.c src/view/regionview.c src/view/graphview.c src/view.c \
	src/view/reservationtracker.c  \

# bitstrings
LIBJIVE_SRC += \
	src/types/bitstring/type.c \
	src/types/bitstring/constant.c \
	src/types/bitstring/symbolic-constant.c \
	src/types/bitstring/slice.c \
	src/types/bitstring/concat.c \
	src/types/bitstring/arithmetic/bitand.c \
	src/types/bitstring/arithmetic/bitor.c \
	src/types/bitstring/arithmetic/bitxor.c \
	src/types/bitstring/arithmetic/bitnot.c \
	src/types/bitstring/arithmetic/bitnegate.c \
	src/types/bitstring/arithmetic/bitsum.c \
	src/types/bitstring/arithmetic/bitdifference.c \
	src/types/bitstring/arithmetic/bitproduct.c \
	src/types/bitstring/arithmetic/bitshiproduct.c \
	src/types/bitstring/arithmetic/bituhiproduct.c \
	src/types/bitstring/arithmetic/bitsquotient.c \
	src/types/bitstring/arithmetic/bituquotient.c \
	src/types/bitstring/arithmetic/bitumod.c \
	src/types/bitstring/arithmetic/bitsmod.c \
	src/types/bitstring/arithmetic/bitshl.c \
	src/types/bitstring/arithmetic/bitshr.c \
	src/types/bitstring/arithmetic/bitashr.c \
	src/types/bitstring/comparison/bitequal.c \
	src/types/bitstring/comparison/bitnotequal.c \
	src/types/bitstring/comparison/bitsless.c \
	src/types/bitstring/comparison/bituless.c \
	src/types/bitstring/comparison/bitslesseq.c \
	src/types/bitstring/comparison/bitulesseq.c \
	src/types/bitstring/comparison/bitsgreater.c \
	src/types/bitstring/comparison/bitugreater.c \
	src/types/bitstring/comparison/bitsgreatereq.c \
	src/types/bitstring/comparison/bitugreatereq.c \
	src/types/bitstring/symbolic-expression.c \
	src/types/bitstring/bitoperation-classes.c \

# records
LIBJIVE_SRC += \
	src/types/record/rcdtype.c \
	src/types/record/rcdgroup.c \
	src/types/record/rcdselect.c \

# unions
LIBJIVE_SRC += \
	src/types/union/unntype.c \
	src/types/union/unnunify.c \
	src/types/union/unnchoose.c \

# functions
LIBJIVE_SRC += \
	src/types/function/fcttype.c \
	src/types/function/fctlambda.c \
	src/types/function/fctapply.c \
	src/types/function/symbolic.c \

# arch definitions
LIBJIVE_SRC += \
	src/arch/registers.c src/arch/instruction.c src/arch/stackslot.c \
	src/arch/memory.c \
	src/arch/subroutine.c \
	src/arch/regselector.c \
	src/arch/load.c \
	src/arch/store.c \
	src/arch/addresstype.c \
	src/arch/address.c \
	src/arch/memlayout.c \
	src/arch/memlayout-simple.c \
	src/arch/dataobject.c \
	src/arch/sizeof.c \
	src/arch/call.c \
	src/arch/address-transform.c \
	src/arch/regvalue.c \
	src/arch/compilate.c \
	src/arch/codegen.c \
	src/arch/label-mapper.c \
	
# shaper
LIBJIVE_SRC += \
	src/regalloc/shaped-graph.c src/regalloc/shaped-region.c src/regalloc/shaped-variable.c src/regalloc/shaped-node.c src/regalloc/xpoint.c \
	src/regalloc/notifiers.c
# register allocator
LIBJIVE_SRC += \
	src/regalloc.c src/regalloc/shape.c src/regalloc/color.c src/regalloc/fixup.c src/regalloc/auxnodes.c src/regalloc/reroute.c src/regalloc/reuse.c \
	src/regalloc/selector.c src/regalloc/stackframe.c

# serialization
LIBJIVE_SRC += \
	src/serialization/arch.c \
	src/serialization/bitstring.c \
	src/serialization/driver.c \
	src/serialization/grammar.c \
	src/serialization/instrcls-registry.c \
	src/serialization/nodecls-registry.c \
	src/serialization/rescls-registry.c \
	src/serialization/symtab.c \
	src/serialization/token-stream.c \
	src/serialization/typecls-registry.c \
	src/serialization/vsdg.c \

# support exported inlines
LIBJIVE_SRC += \
	src/exported_inlines.c

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

src/exported_inlines.c: $(HEADERS)
	@( \
		echo "#define JIVE_EXPORTED_INLINE" ; \
		find include -name "*.h" | \
				sed -e "s:include/\(.*\):#include <\\1>:" \
	) > $@

libjive.a: $(patsubst %.c, %.la, $(LIBJIVE_SRC))
libjive.so: $(patsubst %.c, %.lo, $(LIBJIVE_SRC))

doc:
	doxygen doxygen.conf

clean: depclean
	find . -name *.o -o -name *.lo -o -name *.la -o -name *.so -o -name *.a | xargs rm -rf
	rm -rf $(TESTPROGS)

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
