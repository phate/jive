#include <jive/vsdg/label.h>

#include <stdio.h>
#include <string.h>

#include <jive/common.h>
#include <jive/context.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/sequence.h>
#include <jive/bitstring/type.h>
#include <jive/arch/addresstype.h>
#include <jive/vsdg/node-private.h>

/* label, abstract base type */

static void
jive_label_init_(jive_label * self, const jive_label_class * cls, jive_label_flags flags)
{
	self->class_ = cls;
	self->flags = flags;
}

/* special "current" label */

static jive_addr
jive_label_current_get_address_(const jive_label * self, const jive_seq_node * for_node)
{
	return for_node->address;
}

static const char *
jive_label_current_get_asmname_(const jive_label * self)
{
	return ".";
}

const jive_label_class JIVE_LABEL_CURRENT = {
	.fini = 0,
	.get_address = jive_label_current_get_address_,
	.get_asmname = jive_label_current_get_asmname_,
};

const jive_label jive_label_current = {
	.class_ = &JIVE_LABEL_CURRENT,
	.flags = jive_label_flags_none,
};

/* special "fpoffset" label */

static jive_addr
jive_label_fpoffset_get_address_(const jive_label * self, const jive_seq_node * for_node)
{
	return 0;
}

static const char *
jive_label_fpoffset_get_asmname_(const jive_label * self)
{
	return NULL;
}

const jive_label_class JIVE_LABEL_FPOFFSET = {
	.fini = 0,
	.get_address = jive_label_fpoffset_get_address_,
	.get_asmname = jive_label_fpoffset_get_asmname_,
};

const jive_label jive_label_fpoffset = {
	.class_ = &JIVE_LABEL_FPOFFSET,
	.flags = jive_label_flags_none,
};

/* special "spoffset" label */

static jive_addr
jive_label_spoffset_get_address_(const jive_label * self, const jive_seq_node * for_node)
{
	return 0;
}

static const char *
jive_label_spoffset_get_asmname_(const jive_label * self)
{
	return NULL;
}

const jive_label_class JIVE_LABEL_SPOFFSET = {
	.fini = 0,
	.get_address = jive_label_spoffset_get_address_,
	.get_asmname = jive_label_spoffset_get_asmname_,
};

const jive_label jive_label_spoffset = {
	.class_ = &JIVE_LABEL_SPOFFSET,
	.flags = jive_label_flags_none,
};

/* internal labels */

static void
jive_label_internal_init_(jive_label_internal * self, const jive_label_class * cls, jive_label_flags flags, jive_graph * graph)
{
	jive_label_init_(&self->base, cls, flags);
	self->graph = graph;
	JIVE_LIST_PUSH_BACK(graph->labels, self, graph_label_list);
	self->asmname = 0;
}

static void
jive_label_internal_fini_(jive_label * self_)
{
	jive_label_internal * self = (jive_label_internal *) self_;
	if (self->asmname)
		jive_context_free(self->graph->context, self->asmname);
	JIVE_LIST_REMOVE(self->graph->labels, self, graph_label_list);
}

static jive_addr
jive_label_internal_get_address_(const jive_label * self_, const jive_seq_node * for_node)
{
	const jive_label_internal * self = (const jive_label_internal *) self_;
	jive_seq_node * seq_node = jive_label_internal_get_attach_node(self, for_node->seq_region->seq_graph);
	
	if (seq_node) {
		return seq_node->address;
	} else {
		return 0;
	}
}

/* node labels */

static const char *
jive_label_node_get_asmname_(const jive_label * self_)
{
	jive_label_node * self = (jive_label_node *) self_;
	if (!self->base.asmname) {
		char tmp[64];
		snprintf(tmp, sizeof(tmp), ".L%p", self->node);
		self->base.asmname = jive_context_strdup(self->base.graph->context, tmp);
	}
	return self->base.asmname;
}

static jive_seq_node *
jive_label_node_get_attach_node_(const jive_label_internal * self_, const jive_seq_graph * seq_graph)
{
	const jive_label_node * self = (const jive_label_node *) self_;
	return jive_seq_graph_map_node(seq_graph, self->node);
}

const jive_label_internal_class JIVE_LABEL_NODE_ = {
	.base = {
		.fini = jive_label_internal_fini_,
		.get_address = jive_label_internal_get_address_,
		.get_asmname = jive_label_node_get_asmname_
	},
	.get_attach_node = jive_label_node_get_attach_node_
};

jive_label *
jive_label_node_create(jive_node * node)
{
	jive_graph * graph = node->region->graph;
	jive_label_node * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_label_internal_init_(&self->base, &JIVE_LABEL_NODE, jive_label_flags_none, graph);
	self->node = node;
	
	return &self->base.base;
}

/* region labels */

static const char *
jive_label_region_start_get_asmname_(const jive_label * self_)
{
	jive_label_region * self = (jive_label_region *) self_;
	if (!self->base.asmname) {
		char tmp[64];
		snprintf(tmp, sizeof(tmp), ".L%p_start", self->region);
		self->base.asmname = jive_context_strdup(self->base.graph->context, tmp);
	}
	return self->base.asmname;
}

static jive_seq_node *
jive_label_region_start_get_attach_node_(const jive_label_internal * self_, const jive_seq_graph * seq_graph)
{
	const jive_label_region * self = (const jive_label_region *) self_;
	jive_seq_region * seq_region = jive_seq_graph_map_region(seq_graph, self->region);
	if (seq_region) {
		return seq_region->first_node;
	} else
		return 0;
}

const jive_label_internal_class JIVE_LABEL_REGION_START_ = {
	.base = {
		.fini = jive_label_internal_fini_,
		.get_address = jive_label_internal_get_address_,
		.get_asmname = jive_label_region_start_get_asmname_
	},
	.get_attach_node = jive_label_region_start_get_attach_node_
};

jive_label *
jive_label_region_start_create(jive_region * region)
{
	jive_graph * graph = region->graph;
	jive_label_region * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_label_internal_init_(&self->base, &JIVE_LABEL_REGION_START, jive_label_flags_none, graph);
	self->region = region;
	
	return &self->base.base;
}

jive_label *
jive_label_region_start_create_exported(jive_region * region, const char * name)
{
	jive_graph * graph = region->graph;
	jive_label_region * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_label_internal_init_(&self->base, &JIVE_LABEL_REGION_START, jive_label_flags_none, graph);
	self->region = region;
	self->base.asmname = jive_context_strdup(graph->context, name);
	self->base.base.flags |= jive_label_flags_global;
	
	return &self->base.base;
}

/* region labels */

static const char *
jive_label_region_end_get_asmname_(const jive_label * self_)
{
	jive_label_region * self = (jive_label_region *) self_;
	if (!self->base.asmname) {
		char tmp[64];
		snprintf(tmp, sizeof(tmp), ".L%p_end", self->region);
		self->base.asmname = jive_context_strdup(self->base.graph->context, tmp);
	}
	return self->base.asmname;
}

static jive_seq_node *
jive_label_region_end_get_attach_node_(const jive_label_internal * self_, const jive_seq_graph * seq_graph)
{
	const jive_label_region * self = (const jive_label_region *) self_;
	jive_seq_region * seq_region = jive_seq_graph_map_region(seq_graph, self->region);
	if (seq_region) {
		return seq_region->last_node;
	} else
		return 0;
}

const jive_label_internal_class JIVE_LABEL_REGION_END_ = {
	.base = {
		.fini = jive_label_internal_fini_,
		.get_address = jive_label_internal_get_address_,
		.get_asmname = jive_label_region_end_get_asmname_
	},
	.get_attach_node = jive_label_region_end_get_attach_node_
};

jive_label *
jive_label_region_end_create(jive_region * region)
{
	jive_graph * graph = region->graph;
	jive_label_region * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_label_internal_init_(&self->base, &JIVE_LABEL_REGION_END, jive_label_flags_none, graph);
	self->region = region;
	
	return &self->base.base;
}

jive_label *
jive_label_region_end_create_exported(jive_region * region, const char * name)
{
	jive_graph * graph = region->graph;
	jive_label_region * self = jive_context_malloc(graph->context, sizeof(*self));
	jive_label_internal_init_(&self->base, &JIVE_LABEL_REGION_END, jive_label_flags_none, graph);
	self->region = region;
	self->base.asmname = jive_context_strdup(graph->context, name);
	self->base.base.flags |= jive_label_flags_global;
	
	return &self->base.base;
}

/* external labels */

static jive_addr
jive_label_external_get_address_(const jive_label * self_, const jive_seq_node * for_node)
{
	const jive_label_external * self = (const jive_label_external *) self_;
	return self->address;
}

static const char *
jive_label_external_get_asmname_(const jive_label * self_)
{
	const jive_label_external * self = (const jive_label_external *) self_;
	return self->asmname;
}

static void
jive_label_external_fini_(jive_label * self_)
{
	jive_label_external * self = (jive_label_external *) self_;
	jive_label_external_fini(self);
}

const jive_label_class JIVE_LABEL_EXTERNAL = {
	.fini = jive_label_external_fini_,
	.get_address = jive_label_external_get_address_,
	.get_asmname = jive_label_external_get_asmname_,
};


void
jive_label_external_init(jive_label_external * self, struct jive_context * context, const char * name, jive_addr address)
{
	self->base.class_ = &JIVE_LABEL_EXTERNAL;
	self->base.flags = jive_label_flags_external;
	self->context = context;
	self->asmname = jive_context_strdup(context, name);
	self->address = address;
}

void
jive_label_external_fini(jive_label_external * self)
{
	jive_context_free(self->context, self->asmname);
}

/* label_to_address node */

static void
jive_label_to_address_node_fini_(jive_node * self);

static char *
jive_label_to_address_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_label_to_address_node_get_attrs_(const jive_node * self);

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LABEL_TO_ADDRESS_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_label_to_address_node_fini_, /* override */
	.get_label = jive_label_to_address_node_get_label_, /* override */
	.get_attrs = jive_label_to_address_node_get_attrs_, /* override */
	.match_attrs = jive_label_to_address_node_match_attrs_, /* override */
	.create = jive_label_to_address_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_label_to_address_node_init_(
	jive_label_to_address_node * self,
	jive_graph * graph,
	const jive_label * label)
{
	JIVE_DECLARE_ADDRESS_TYPE(addrtype);
	jive_node_init_(&self->base, graph->root_region,
		0, NULL, NULL,
		1, &addrtype);
	
	self->attrs.label = label;
}

static void
jive_label_to_address_node_fini_(jive_node * self_)
{
	jive_label_to_address_node * self = (jive_label_to_address_node *) self_;
	
	jive_node_fini_(&self->base);
}

static char *
jive_label_to_address_node_get_label_(const jive_node * self_)
{
	//const jive_label_to_address_node * self = (const jive_label_to_address_node *) self_;
	
	/* FIXME: would indeed be nice to retrieve the label string here */
	return strdup("label");
}

static jive_node *
jive_label_to_address_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_label_to_address_node_attrs * attrs = (const jive_label_to_address_node_attrs *)attrs_;
	
	return jive_label_to_address_node_create(region->graph, attrs->label);
}

static const jive_node_attrs *
jive_label_to_address_node_get_attrs_(const jive_node * self_)
{
	const jive_label_to_address_node * self = (const jive_label_to_address_node *) self_;

	return &self->attrs.base;
}

static bool
jive_label_to_address_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_label_to_address_node_attrs * first =
		&((const jive_label_to_address_node *)self)->attrs;
	const jive_label_to_address_node_attrs * second =
		(const jive_label_to_address_node_attrs *) attrs;
	
	return first->label == second->label;
}

jive_node *
jive_label_to_address_node_create(struct jive_graph * graph, const jive_label * label)
{
	jive_label_to_address_node * node = jive_context_malloc(graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LABEL_TO_ADDRESS_NODE;
	jive_label_to_address_node_init_(node, graph, label);

	return &node->base;
}

jive_output *
jive_label_to_address_create(struct jive_graph * graph, const jive_label * label)
{
	return jive_label_to_address_node_create(graph, label)->outputs[0];
}

/* label_to_bitstring_node */

static void
jive_label_to_bitstring_node_fini_(jive_node * self);

static char *
jive_label_to_bitstring_node_get_label_(const jive_node * self);

static const jive_node_attrs *
jive_label_to_bitstring_node_get_attrs_(const jive_node * self);

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs);

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs,
	size_t noperands, struct jive_output * const operands[]);

const jive_node_class JIVE_LABEL_TO_BITSTRING_NODE = {
	.parent = &JIVE_NODE,
	.fini = jive_label_to_bitstring_node_fini_, /* override */
	.get_label = jive_label_to_bitstring_node_get_label_, /* override */
	.get_attrs = jive_label_to_bitstring_node_get_attrs_, /* override */
	.match_attrs = jive_label_to_bitstring_node_match_attrs_, /* override */
	.create = jive_label_to_bitstring_node_create_, /* override */
	.get_aux_rescls = jive_node_get_aux_rescls_ /* inherit */
};

static void
jive_label_to_bitstring_node_init_(
	jive_label_to_bitstring_node * self,
	jive_graph * graph,
	const jive_label * label, size_t nbits)
{
	JIVE_DECLARE_BITSTRING_TYPE(btype, nbits);
	jive_node_init_(&self->base, graph->root_region,
		0, NULL, NULL,
		1, &btype);

	self->attrs.nbits = nbits;
	self->attrs.label = label;
}

static void
jive_label_to_bitstring_node_fini_(jive_node * self_)
{
	jive_label_to_bitstring_node * self = (jive_label_to_bitstring_node *) self_;

	jive_node_fini_(&self->base);
}

static char *
jive_label_to_bitstring_node_get_label_(const jive_node * self_)
{
	//const jive_label_to_bitstring_node * self = (const jive_label_to_bitstring_node *) self_;

	return strdup("label");
}

static jive_node *
jive_label_to_bitstring_node_create_(struct jive_region * region, const jive_node_attrs * attrs_,
	size_t noperands, struct jive_output * const operands[])
{
	const jive_label_to_bitstring_node_attrs * attrs =
		(const jive_label_to_bitstring_node_attrs *) attrs_;

	return jive_label_to_bitstring_node_create(region->graph, attrs->label, attrs->nbits);
}

static const jive_node_attrs *
jive_label_to_bitstring_node_get_attrs_(const jive_node * self_)
{
	const jive_label_to_bitstring_node * self = (const jive_label_to_bitstring_node *) self_;

	return &self->attrs.base;
}

static bool
jive_label_to_bitstring_node_match_attrs_(const jive_node * self, const jive_node_attrs * attrs)
{
	const jive_label_to_bitstring_node_attrs * first =
		&((const jive_label_to_bitstring_node *)self)->attrs;
	const jive_label_to_bitstring_node_attrs * second =
		(const jive_label_to_bitstring_node_attrs *) attrs;

	return (first->label == second->label) && (first->nbits == second->nbits);
}

jive_node *
jive_label_to_bitstring_node_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	jive_label_to_bitstring_node * node = jive_context_malloc(graph->context, sizeof(*node));
	node->base.class_ = &JIVE_LABEL_TO_BITSTRING_NODE;
	jive_label_to_bitstring_node_init_(node, graph, label, nbits);

	return &node->base;
}

jive_output *
jive_label_to_bitstring_create(struct jive_graph * graph, const jive_label * label, size_t nbits)
{
	return jive_label_to_bitstring_node_create(graph, label, nbits)->outputs[0];
}
