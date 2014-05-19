/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/vsdg/objdef.h>

#include <string.h>

#include <jive/context.h>
#include <jive/util/buffer.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/label.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/statetype.h>

static void
jive_objdef_node_init_(
	jive_objdef_node * self,
	jive_region * region,
	jive_output * obj)
{
	/* FIXME: this is horribly wrong, but we don't have another type right now for putting in here,
						this entire node needs to be remodeled
	*/
	jive_control_type stype;
	const jive_type * stype_ptr = &stype;
	const jive_type * type = jive_output_get_type(obj);
	jive_node_init_(self, region,
		1, &type, &obj,
		1, &stype_ptr);
	
	if (obj->node()->ninputs < 1 || !dynamic_cast<jive_anchor_input*>(obj->node()->inputs[0])) {
		jive_context_fatal_error(region->graph->context,
			"Type mismatch: object definitions can only be applied to region anchor nodes");
	}
}

static void
jive_objdef_node_fini_(jive_node * self_)
{
	jive_node_fini_(self_);
}

static void
jive_objdef_node_get_label_(const jive_node * self_, struct jive_buffer * buffer)
{
	const jive_objdef_node * self = (const jive_objdef_node *) self_;
	jive_buffer_putstr(buffer, self->operation().name().c_str());
}

static bool
jive_objdef_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	return false;
}

static jive_node *
jive_objdef_node_create_(jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive::objdef_operation * attrs = (const jive::objdef_operation *) attrs_;
	
	jive_objdef_node * other = new jive_objdef_node(*attrs);
	other->class_ = &JIVE_OBJDEF_NODE;
	jive_objdef_node_init_(other, region, operands[0]);
	
	return other;
}

const jive_node_class JIVE_OBJDEF_NODE = {
	parent : &JIVE_NODE,
	name : "OBJDEF",
	fini : jive_objdef_node_fini_, /* override */
	get_default_normal_form : jive_node_get_default_normal_form_, /* inherit */
	get_label : jive_objdef_node_get_label_, /* override */
	match_attrs : jive_objdef_node_match_attrs_, /* override */
	check_operands : NULL,
	create : jive_objdef_node_create_, /* override */
};

jive_node *
jive_objdef_node_create(
	jive_output * output,
	const char * name,
	const struct jive_linker_symbol * symbol)
{
	jive_region * region = output->node()->region;
	jive_objdef_node * self = new jive_objdef_node(
		jive::objdef_operation(name, symbol));
	self->class_ = &JIVE_OBJDEF_NODE;
	jive_objdef_node_init_(self, region, output);
	
	return self;
}
