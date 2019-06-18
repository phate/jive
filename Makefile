# Copyright 2010 2011 2012 Helge Bahmann <hcb@chaoticmind.net>
# Copyright 2011 2012 2013 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

define HELP_TEXT
clear
echo "Makefile for the Jive RVSDG API"
echo "Version 1.0 - 2019-06-18"
endef
.PHONY: help
help:
	@$(HELP_TEXT)
	@$(HELP_TEXT_JIVE)
	@echo "all                    Compiles jive and checks the result"
	@echo "clean                  Calls clean for jive and tests"


JIVE_ROOT ?= .

CFLAGS+=-Wall -Wpedantic --std=c++14
CPPFLAGS+=-I$(JIVE_ROOT)/include

.PHONY: all
all: jive check

include Makefile.sub
include tests/Makefile.sub

.PHONY: doc
doc:
	doxygen doxygen.conf

.PHONY: clean
clean: jive-clean jive-test-clean

%.lo: %.c
	$(CXX) -c -DPIC -fPIC $(CFLAGS) $(CPPFLAGS) -o $@ $<

%.la: %.c
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

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

DEPEND = $(patsubst %.c, .dep/%.la.d, $(SOURCES)) $(patsubst %.c, .dep/%.lo.d, $(SOURCES))
depend: $(DEPEND)
ifeq ($(shell if [ -e .dep ] ; then echo yes ; fi),yes)
-include $(DEPEND)
endif

ifeq ($(shell if [ -e .Makefile.override ] ; then echo yes ; fi),yes)
include .Makefile.override
endif
