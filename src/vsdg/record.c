#include <jive/vsdg/record.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/recordtype.h>
#include <jive/vsdg/node-private.h>


static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static char *
jive_group_node_get_label_(const jive_node * self_);

static const jive_node_attrs *
jive_group_node_get_attrs_(const jive_node * self);

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_GROUP_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_node_fini_, /* inherit */
	.get_label = jive_group_node_get_label_, /* override */
	.get_attrs = jive_group_node_get_attrs_, /* override */
	.match_attrs = jive_group_node_match_attrs_, /* override */
	.create = jive_group_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static char *
jive_group_node_get_label_(const jive_node * self_)
{
	return strdup("GROUP");
}

static const jive_node_attrs *
jive_group_node_get_attrs_(const jive_node * self_)
{
	const jive_group_node * self = (const jive_group_node*)self_;

	return &self->attrs.base;
}

static bool
jive_group_node_match_attrs_(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_group_node_attrs * first = (const jive_group_node_attrs *)jive_node_get_attrs(self);
	const jive_group_node_attrs * second = (const jive_group_node_attrs *)second_;
	
	return first->decl == second->decl;
}

static jive_node *
jive_group_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_group_node_attrs * attrs = (const jive_group_node_attrs *)attrs_ ;

	return jive_group_node_create(region, attrs->decl, noperands, operands);
}

static void
jive_group_node_init_(jive_group_node * self,
	struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[])
{
	if (decl->nelements != narguments) {
		jive_context_fatal_error(self->base.graph->context, "Type mismatch: number of parameters to group does not match record layout");
	}

	size_t n;
	const jive_type * arg_types[narguments];
	for(n = 0; n < narguments; n++) {
		arg_types[n] = &decl->elements[n]->base;
	}

	jive_record_type type;
	jive_record_type_init(&type, decl);
	const jive_type * rtype = &type.base.base ;

	jive_node_init_(&self->base, region,
		narguments, arg_types, arguments,
		1, &rtype);

	type.base.base.class_->fini(&type.base.base);

	self->attrs.decl = decl;
}

jive_node *
jive_group_node_create(struct jive_region * region, const jive_record_declaration * decl,
	size_t narguments, jive_output * const arguments[])
{
	jive_group_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_GROUP_NODE;
	jive_group_node_init_(node, region, decl, narguments, arguments);

	return &node->base;
}

jive_output *
jive_group_create(const jive_record_declaration * decl,
	size_t narguments, jive_output * arguments[const])
{
	jive_region * region = jive_region_innermost(narguments, arguments);

	jive_group_node * node = (jive_group_node *)
		jive_group_node_create(region, decl, narguments, arguments);
	
	return node->base.outputs[0];
}


static char *
jive_select_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_select_node_get_attrs_(const jive_node * self);

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

const jive_node_class JIVE_SELECT_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_node_fini_, /* inherit */
	.get_label = jive_select_node_get_label_, /* override */
	.get_attrs = jive_select_node_get_attrs_, /* override */
	.match_attrs = jive_select_node_match_attrs_, /* override */
	.create = jive_select_node_create_,	/* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_select_node_init_(jive_select_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	jive_context * context = region->graph->context;
	if (operand->class_ != &JIVE_RECORD_OUTPUT) {
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");
	}
	const jive_record_type * operand_type = (const jive_record_type *)
		operand->class_->get_type(operand);

	if (element > operand_type->decl->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "Type mismatch: attempted to select element #%zd from record of %zd elements",
			element, operand_type->decl->nelements);
		jive_context_fatal_error(context, jive_context_strdup(context, tmp));
	}
	
	self->attrs.element = element;

	const jive_type * output_type = &operand_type->decl->elements[element]->base;
	jive_node_init_(&self->base, region,
		1, (const jive_type * []){&operand_type->base.base}, &operand,
		1, &output_type);
}

static char *
jive_select_node_get_label_(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SELECT(%zd)", self->attrs.element);
	return strdup(tmp);
}

static const jive_node_attrs *
jive_select_node_get_attrs_(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *)self_;

	return &self->attrs.base;
} 

static bool
jive_select_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_select_node_attrs * first = &((const jive_select_node *)self)->attrs;
	const jive_select_node_attrs * second = (const jive_select_node_attrs *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static jive_node *
jive_select_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_select_node_attrs * attrs = (const jive_select_node_attrs *)attrs_;

	return &jive_select_node_create(region, attrs->element, operands[0])->base;	
}

jive_select_node *
jive_select_node_create(struct jive_region * region, size_t member, jive_output * operand)
{
	jive_select_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_SELECT_NODE;
	jive_select_node_init_(node, region, member, operand);

	return node;	
}

jive_output *
jive_select_create(size_t member, jive_output * operand)
{
	jive_select_node * node = jive_select_node_create(operand->node->region, member, operand);

	return (jive_output *)node->base.outputs[0];
}

