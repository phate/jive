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
  jive_output * const operand);

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

	return jive_unify_node_create(region, attrs->layout, operands[0]);
}

static void
_jive_unify_node_init(jive_unify_node * self,
	struct jive_region * region, const jive_union_layout * layout,
  jive_output * const operand)
{
  const jive_type * itype = jive_output_get_type(operand);

  int i;
  const jive_type * arg_type = 0;
  for(i = 0; i < layout->nelements; i++){
    if(jive_type_equals(&layout->element[i]->base, itype)){
      arg_type = &layout->element[i]->base;
    }
  }

  if(!arg_type){
    jive_context_fatal_error(self->base.graph->context,
      "Type mismatch: input type to unify does not match any type in union layout");
  }

  jive_union_type type;
  jive_union_type_init(&type, layout);
  const jive_type * rtype = &type.base.base;

  _jive_node_init(&self->base, region,
    1, (const jive_type **)&arg_type, &operand,
    1, (const jive_type **)&rtype);

  type.base.base.class_->fini(&type.base.base);
}

jive_node *
jive_unify_node_create(struct jive_region * region, const jive_union_layout * layout,
  jive_output * const operand)
{
	jive_unify_node * node = jive_context_malloc(region->graph->context, sizeof(*node));

	node->base.class_ = &JIVE_UNIFY_NODE;
	_jive_unify_node_init(node, region, layout, operand); 

	return &node->base;
}

jive_output *
jive_unify_create(const jive_union_layout * layout,
  jive_output * const operand)
{
	jive_unify_node * node = (jive_unify_node *)
		jive_unify_node_create(operand->node->region, layout, operand);

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
	jive_context * context = region->graph->context;
	if (operand->class_ != &JIVE_UNION_OUTPUT) {
		jive_context_fatal_error(context, "Type mismatch: need 'record' type as input to 'select' node");
	}
	
	const jive_union_type * operand_type = (const jive_union_type *)
		operand->class_->get_type(operand);

	if (element > operand_type->layout->nelements) {
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "Type mismatch: attempted to select element #%zd from union of %zd elements",
			element, operand_type->layout->nelements);
		jive_context_fatal_error(context, jive_context_strdup(context, tmp));
	}
	
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

