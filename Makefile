# Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
# Copyright 2011 2012 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

CPPFLAGS+=-Iinclude
CFLAGS+=-g --std=c++0x -xc++ -fpermissive

# VSDG core
LIBJIVE_SRC = \
	src/collector.c \
	src/context.c \
	src/vsdg/anchor.c \
	src/vsdg/anchortype.c \
	src/vsdg/basetype.c \
	src/vsdg/control.c \
	src/vsdg/controltype.c \
	src/vsdg/equivalence.c \
	src/vsdg/gamma.c \
	src/vsdg/gate-interference.c \
	src/vsdg/graph.c \
	src/vsdg/label.c \
	src/vsdg/negotiator.c \
	src/vsdg/node.c \
	src/vsdg/notifiers.c \
	src/vsdg/objdef.c \
	src/vsdg/operators.c \
	src/vsdg/operators/base.c \
	src/vsdg/operators/binary.c \
	src/vsdg/operators/nullary.c \
	src/vsdg/operators/unary.c \
	src/vsdg/phi.c \
	src/vsdg/region.c \
	src/vsdg/resource.c \
	src/vsdg/splitnode.c \
	src/vsdg/statetype.c \
	src/vsdg/substitution.c \
	src/vsdg/theta.c \
	src/vsdg/tracker.c \
	src/vsdg/traverser.c \
	src/vsdg/valuetype.c \
	src/vsdg/variable.c \

# visualization
LIBJIVE_SRC += \
	src/util/buffer.c \
	src/util/textcanvas.c \
	src/view.c \
	src/view/graphview.c \
	src/view/nodeview.c \
	src/view/regionview.c \
	src/view/reservationtracker.c  \

# bitstrings
LIBJIVE_SRC += \
	src/types/bitstring/arithmetic/bitand.c \
	src/types/bitstring/arithmetic/bitashr.c \
	src/types/bitstring/arithmetic/bitdifference.c \
	src/types/bitstring/arithmetic/bitnegate.c \
	src/types/bitstring/arithmetic/bitnot.c \
	src/types/bitstring/arithmetic/bitor.c \
	src/types/bitstring/arithmetic/bitproduct.c \
	src/types/bitstring/arithmetic/bitshiproduct.c \
	src/types/bitstring/arithmetic/bitshl.c \
	src/types/bitstring/arithmetic/bitshr.c \
	src/types/bitstring/arithmetic/bitsmod.c \
	src/types/bitstring/arithmetic/bitsquotient.c \
	src/types/bitstring/arithmetic/bitsum.c \
	src/types/bitstring/arithmetic/bituhiproduct.c \
	src/types/bitstring/arithmetic/bitumod.c \
	src/types/bitstring/arithmetic/bituquotient.c \
	src/types/bitstring/arithmetic/bitxor.c \
	src/types/bitstring/bitoperation-classes.c \
	src/types/bitstring/comparison/bitequal.c \
	src/types/bitstring/comparison/bitnotequal.c \
	src/types/bitstring/comparison/bitsgreater.c \
	src/types/bitstring/comparison/bitsgreatereq.c \
	src/types/bitstring/comparison/bitsless.c \
	src/types/bitstring/comparison/bitslesseq.c \
	src/types/bitstring/comparison/bitugreater.c \
	src/types/bitstring/comparison/bitugreatereq.c \
	src/types/bitstring/comparison/bituless.c \
	src/types/bitstring/comparison/bitulesseq.c \
	src/types/bitstring/concat.c \
	src/types/bitstring/constant.c \
	src/types/bitstring/slice.c \
	src/types/bitstring/symbolic-constant.c \
	src/types/bitstring/type.c \

# floats
LIBJIVE_SRC += \
	src/types/float/arithmetic/fltdifference.c \
	src/types/float/arithmetic/fltnegate.c \
	src/types/float/arithmetic/fltproduct.c \
	src/types/float/arithmetic/fltquotient.c \
	src/types/float/arithmetic/fltsum.c \
	src/types/float/comparison/fltequal.c \
	src/types/float/comparison/fltgreater.c \
	src/types/float/comparison/fltgreatereq.c \
	src/types/float/comparison/fltless.c \
	src/types/float/comparison/fltlesseq.c \
	src/types/float/comparison/fltnotequal.c \
	src/types/float/fltconstant.c \
	src/types/float/fltoperation-classes.c \
	src/types/float/flttype.c \

# reals
LIBJIVE_SRC += \
	src/types/real/arithmetic/rldifference.c \
	src/types/real/arithmetic/rlnegate.c \
	src/types/real/arithmetic/rlproduct.c \
	src/types/real/arithmetic/rlquotient.c \
	src/types/real/arithmetic/rlsum.c \
	src/types/real/comparison/rlequal.c \
	src/types/real/comparison/rlgreater.c \
	src/types/real/comparison/rlgreatereq.c \
	src/types/real/comparison/rlless.c \
	src/types/real/comparison/rllesseq.c \
	src/types/real/comparison/rlnotequal.c \
	src/types/real/rlconstant.c \
	src/types/real/rloperation-classes.c \
	src/types/real/rltype.c \

# doubles
LIBJIVE_SRC += \
	src/types/double/arithmetic/dbldifference.c \
	src/types/double/arithmetic/dblnegate.c \
	src/types/double/arithmetic/dblproduct.c \
	src/types/double/arithmetic/dblquotient.c \
	src/types/double/arithmetic/dblsum.c \
	src/types/double/comparison/dblequal.c \
	src/types/double/comparison/dblgreater.c \
	src/types/double/comparison/dblgreatereq.c \
	src/types/double/comparison/dblless.c \
	src/types/double/comparison/dbllesseq.c \
	src/types/double/comparison/dblnotequal.c \
	src/types/double/dblconstant.c	\
	src/types/double/dbloperation-classes.c \
	src/types/double/dbltype.c \

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
	src/types/function/fctsymbolic.c \
	src/types/function/fcttype.c \

# integrals
LIBJIVE_SRC += \
	src/types/integral/arithmetic/itgand.c \
	src/types/integral/arithmetic/itgdifference.c \
	src/types/integral/arithmetic/itgmodulo.c \
	src/types/integral/arithmetic/itgnegate.c \
	src/types/integral/arithmetic/itgnot.c \
	src/types/integral/arithmetic/itgor.c \
	src/types/integral/arithmetic/itgproduct.c \
	src/types/integral/arithmetic/itgquotient.c \
	src/types/integral/arithmetic/itgsum.c \
	src/types/integral/arithmetic/itgxor.c \
	src/types/integral/comparison/itgequal.c \
	src/types/integral/comparison/itgless.c \
	src/types/integral/comparison/itglesseq.c \
	src/types/integral/comparison/itggreater.c \
	src/types/integral/comparison/itggreatereq.c \
	src/types/integral/comparison/itgnotequal.c \
	src/types/integral/itgconstant.c \
	src/types/integral/itgoperation-classes.c \
	src/types/integral/itgtype.c \

# arch definitions
LIBJIVE_SRC += \
	src/arch/registers.c src/arch/instruction.c src/arch/stackslot.c \
	src/arch/memorytype.c \
	src/arch/subroutine.c \
	src/arch/subroutine/nodes.c \
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
	src/arch/sequence.c \
	src/arch/label-mapper.c \
	src/arch/immediate-node.c \
	src/arch/immediate-type.c \

# frontend
LIBJIVE_SRC += \
	src/frontend/clg_node.c \
	src/frontend/clg.c \
	src/frontend/cfg_node.c \
	src/frontend/basic_block.c \
	src/frontend/cfg.c \
	src/frontend/cfg-scc.c \
	src/frontend/tac/three_address_code.c \
	src/frontend/tac/variable.c \
	src/frontend/tac/assignment.c \
	src/frontend/tac/reference.c \
	src/frontend/tac/call.c \
	src/frontend/tac/bitstring/bitconstant.c \
	src/frontend/tac/bitstring/arithmetic/bitsum.c \
	src/frontend/tac/bitstring/comparison/bitequal.c \
	src/frontend/tac/bitstring/comparison/bitnotequal.c \

	
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
