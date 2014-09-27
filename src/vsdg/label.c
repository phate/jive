/*
 * Copyright 2010 2011 2012 2013 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/label.h>

#include <stdio.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

/* label, abstract base type */

const jive_label_class JIVE_LABEL = {
	parent : 0,
	fini : 0,
};

/* special "current" label */

const jive_label_class JIVE_LABEL_CURRENT = {
	parent : &JIVE_LABEL,
	fini : 0,
};

const jive_label jive_label_current = {
	class_ : &JIVE_LABEL_CURRENT,
	flags : jive_label_flags_none,
};

/* special "fpoffset" label */

const jive_label_class JIVE_LABEL_FPOFFSET = {
	parent : &JIVE_LABEL,
	fini : 0,
};

const jive_label jive_label_fpoffset = {
	class_ : &JIVE_LABEL_FPOFFSET,
	flags : jive_label_flags_none,
};

/* special "spoffset" label */

const jive_label_class JIVE_LABEL_SPOFFSET = {
	parent : &JIVE_LABEL,
	fini : 0,
};

const jive_label jive_label_spoffset = {
	class_ : &JIVE_LABEL_SPOFFSET,
	flags : jive_label_flags_none,
};

/* external labels */

static void
jive_label_external_fini_(jive_label * self_)
{
}

const jive_label_class JIVE_LABEL_EXTERNAL = {
	parent : &JIVE_LABEL,
	fini : jive_label_external_fini_,
};


void
jive_label_external_init(
	jive_label_external * self,
	struct jive_context * context,
	const char * name,
	const struct jive_linker_symbol * symbol)
{
	self->base.class_ = &JIVE_LABEL_EXTERNAL;
	self->base.flags = jive_label_flags_external;
	self->context = context;
	self->asmname = std::string(name);
	self->symbol = symbol;
}
