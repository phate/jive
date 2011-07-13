#include <jive/vsdg/union.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/uniontype.h>
#include <jive/context.h>
#include <jive/common.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/graph.h>

#include <stdio.h>
#include <string.h>

static void
_jive_unify_node_init(jive_unify_node * self,
	struct jive_region * region, const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[]);

static jive_node *
_jive_unify_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static char *
_jive_unify_node_get_label(const jive_node * self_);

static bool
_jive_unify_node_match_attrs(const jive_node * self, const jive_node_attrs * second);

const jive_node_class JIVE_UNIFY_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_unify_node_get_label, /* override */
	.get_attrs = _jive_node_get_attrs, /* inherit */
	.match_attrs = _jive_unify_node_match_attrs, /* override */
	.create = _jive_unify_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static char *
_jive_unify_node_get_label(const jive_node * self_)
{
	return strdup("UNIFY");
}

static bool
_jive_unify_node_match_attrs(const jive_node * self, const jive_node_attrs * second_)
{
	const jive_unify_node_attrs * first = (const jive_unify_node_attrs *)jive_node_get_attrs(self);
	const jive_unify_node_attrs * second = (const jive_unify_node_attrs *)second_;

	if(first->layout != second->layout) return false;

	return true;
}

static jive_node *
_jive_unify_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, jive_output * const operands[])
{
	const jive_unify_node_attrs * attrs = (const jive_unify_node_attrs *)attrs_ ;

	return jive_unify_node_create(region, attrs->layout, noperands, operands);
}

static void
_jive_unify_node_init(jive_unify_node * self,
	struct jive_region * region, const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[])
{
	//FIXME: this is ugly
	JIVE_DEBUG_ASSERT(layout->nelements == narguments);

	int i;
	const jive_type * arg_types[layout->nelements];
	for(i = 0; i < layout->nelements; i++){
		arg_types[i] = &layout->element[i]->base;
	}

	jive_union_type type;
	jive_union_type_init(&type, layout);
	const jive_type * rtype = &type.base.base ;

	_jive_node_init(&self->base, region,
		layout->nelements, arg_types, arguments,
		1, (const jive_type **)&rtype);

	type.base.base.class_->fini(&type.base.base);
}

jive_node *
jive_unify_node_create(struct jive_region * region, const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[])
{
	jive_unify_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_UNIFY_NODE;
	_jive_unify_node_init(node, region, layout, narguments, arguments);

	return &node->base;
}

jive_output *
jive_unify_create(const jive_union_layout * layout,
	size_t narguments, jive_output * const arguments[])
{
	jive_unify_node * node = (jive_unify_node *)
		jive_unify_node_create(arguments[0]->node->region, layout, narguments, arguments);

	return (jive_output*)node->base.outputs[0];
}

static char *
_jive_choose_node_get_label(const jive_node * self);

static const jive_node_attrs *
_jive_choose_node_get_attrs(const jive_node * self);

static jive_node *
_jive_choose_node_create(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, jive_output * const operands[]);

static bool
_jive_choose_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs);

const jive_node_class JIVE_CHOOSE_NODE = {
	.parent = &JIVE_NODE,
	.fini = _jive_node_fini, /* inherit */
	.get_label = _jive_choose_node_get_label, /* override */
	.get_attrs = _jive_choose_node_get_attrs, /* override */
	.match_attrs = _jive_choose_node_match_attrs, /* overrride */
	.create = _jive_choose_node_create, /* override */
	.get_aux_rescls = _jive_node_get_aux_rescls /* inherit */
};

static void
_jive_choose_node_init(jive_choose_node * self, struct jive_region * region,
	size_t element, jive_output * operand)
{
	//FIXME: this is ugly, some other error would be nice
	JIVE_DEBUG_ASSERT(operand->class_ == &JIVE_UNION_OUTPUT);
	const jive_union_type * operand_type = (const jive_union_type *)
		operand->class_->get_type(operand);

	//FIXME: this is ugly, some other error would be nice
	JIVE_DEBUG_ASSERT(element < operand_type->layout->nelements);

	self->attrs.element = element;

	_jive_node_init(&self->base, region,
		1, (const jive_type * []){&operand_type->base.base}, &operand,
		1, (const jive_type **) &operand_type->layout->element[element]);
}

static char *
_jive_choose_node_get_label(const jive_node * self_)
{
	const jive_choose_node * self = (const jive_choose_node *) self_;

	char tmp[32];
	snprintf(tmp, sizeof(tmp), "CHOOSE(%zd)", self->attrs.element);
	return strdup(tmp);
}

static const jive_node_attrs *
_jive_choose_node_get_attrs(const jive_node * self_)
{
	const jive_choose_node * self = (const jive_choose_node *)self_;

	return &self->attrs.base;
}

static bool
_jive_choose_node_match_attrs(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_choose_node_attrs * first = &((const jive_choose_node *)self)->attrs;
	const jive_choose_node_attrs * second = (const jive_choose_node_attrs *) attrs;

	if(first->element != second->element)
		return false;

	return true;
}

static jive_node *
_jive_choose_node_create(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_choose_node_attrs * attrs = (const jive_choose_node_attrs *)attrs_;

	return &jive_choose_node_create(region, attrs->element, operands[0])->base;
}

jive_choose_node *
jive_choose_node_create(struct jive_region * region, size_t member, jive_output * operand)
{
	jive_choose_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_CHOOSE_NODE;
	_jive_choose_node_init(node, region, member, operand);

	return node;
}

jive_output *
jive_choose_create(size_t member, jive_output * operand)
{
	jive_choose_node * node = jive_choose_node_create(operand->node->region, member, operand);

	return (jive_output *)node->base.outputs[0];
}

