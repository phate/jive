/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <stdio.h>

#include <jive/common.h>
#include <jive/rvsdg/graph.h>
#include <jive/rvsdg/label.h>
#include <jive/rvsdg/node.h>
#include <jive/rvsdg/region.h>

namespace jive {

/* label */

label::~label()
{}

jive_label_flags
label::flags() const noexcept
{
	return jive_label_flags_none;
}

/* current label */

current_label::~current_label()
{}

/* fpoffset label */

fpoffset_label::~fpoffset_label()
{}

/* spoffset label */

spoffset_label::~spoffset_label()
{}

/* external label */

external_label::~external_label()
{}

jive_label_flags
external_label::flags() const noexcept
{
	return jive_label_flags_external;
}

}
