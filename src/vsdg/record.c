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
_jive_group_node_init(jive_group_node * self,
	struct jive_region * region, const jive_record_layout * layout,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
_jive_group_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static char *
_jive_group_node_get_label(const jive_node * self_);

static const jive_node_attrs *
jive_group_node_get_attrs_(const jive_node * self);

static bool
_jive_group_node_match_attrs(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_GROUP_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_group_node_get_label, /* override */
	.get_attrs = jive_group_node_get_attrs_, /* override */
	.match_attrs = _jive_group_node_match_attrs, /* override */
	.create = _jive_group_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static char *
_jive_group_node_get_label(const jive_node * self_)
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
_jive_group_node_match_attrs(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_group_node_attrs * first = (const jive_group_node_attrs *)jive_node_get_attrs(self);
	const jive_group_node_attrs * second = (const jive_group_node_attrs *)second_;
	
	if(first->layout != second->layout) return false;

	return true;
}

static jive_node *
_jive_group_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_group_node_attrs * attrs = (const jive_group_node_attrs *)attrs_ ;

	return jive_group_node_create(region, attrs->layout, noperands, operands);
}

static void
_jive_group_node_init(jive_group_node * self,
	struct jive_region * region, const jive_record_layout * layout,
	size_t narguments, jive_output * const arguments[])
{
	if (layout->nelements != narguments) {
		jive_context_fatal_error(self->base.graph->context, "Type mismatch: number of parameters to group does not match record layout");
	}
	JIVE_DEBUG_ASSERT(layout->nelements == narguments);

	int i;
	const jive_type * arg_types[layout->nelements];
	for(i = 0; i < layout->nelements; i++){
		arg_types[i] = &layout->element[i].type->base;
	}

	jive_record_type type;
	jive_record_type_init(&type, layout);
	const jive_type * rtype = &type.base.base ;

	_jive_node_init(&self->base, region,
		layout->nelements, arg_types, arguments,
		1, (const jive_type **)&rtype);

	type.base.base.class_->fini(&type.base.base);

	self->attrs.layout = layout;
}

jive_node *
jive_group_node_create(struct jive_region * region, const jive_record_layout * layout,
	size_t narguments, jive_output * const arguments[])
{
	jive_group_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_GROUP_NODE;
	_jive_group_node_init(node, region, layout, narguments, arguments);

	return &node->base;
}

jive_output *
jive_group_create(const jive_record_layout * layout,
	size_t narguments, jive_output * arguments[const])
{
	jive_region * region = jive_region_innermost(narguments, arguments);

	jive_group_node * node = (jive_group_node *)
		jive_group_node_create(region, layout, narguments, arguments);
	
	return (jive_output*)node->base.outputs[0];
}


static char *
_jive_select_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_select_node_get_attrs(const jive_node * self);

static jive_node *
_jive_select_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
_jive_select_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

const jive_node_class JIVE_SELECT_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_select_node_get_label, /* override */
	.get_attrs = _jive_select_node_get_attrs, /* override */
	.match_attrs = _jive_select_node_match_attrs, /* override */
	.create = _jive_select_node_create,	/* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
_jive_select_node_init(jive_select_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	jive_context * context = region->graph->context;
	if (operand->class_ != &JIVE_RECORD_OUTPUT) {
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");
	}
	const jive_record_type * operand_type = (const jive_record_type *)
		operand->class_->get_type(operand);

	if (element > operand_type->layout->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "Type mismatch: attempted to select element #%zd from record of %zd elements",
			element, operand_type->layout->nelements);
		jive_context_fatal_error(context, jive_context_strdup(context, tmp));
	}
	
	self->attrs.element = element;

	_jive_node_init(&self->base, region,
		1, (const jive_type * []){&operand_type->base.base}, &operand,
		1, (const jive_type **) &operand_type->layout->element[element]);
}

static char *
_jive_select_node_get_label(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "SELECT(%zd)", self->attrs.element);
	return strdup(tmp);
}

static const jive_node_attrs *
_jive_select_node_get_attrs(const jive_node * self_)
{
	const jive_select_node * self = (const jive_select_node *)self_;

	return &self->attrs.base;
} 

static bool
_jive_select_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_select_node_attrs * first = &((const jive_select_node *)self)->attrs;
	const jive_select_node_attrs * second = (const jive_select_node_attrs *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static jive_node *
_jive_select_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
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
	_jive_select_node_init(node, region, member, operand);

	return node;	
}

jive_output *
jive_select_create(size_t member, jive_output * operand)
{
	jive_select_node * node = jive_select_node_create(operand->node->region, member, operand);

	return (jive_output *)node->base.outputs[0];
}

