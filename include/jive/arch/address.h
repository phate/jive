#ifndef JIVE_ARCH_ADDRESS_H
#define JIVE_ARCH_ADDRESS_H

#include <jive/common.h>
#include <jive/arch/addresstype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/operators.h>
#include <jive/vsdg/recordlayout.h>

/* "memberof" operator: given an address that is the start of a record
in memory, compute address of specified member of record */

extern const jive_unary_operation_class JIVE_MEMBEROF_NODE_;
#define JIVE_MEMBEROF_NODE (JIVE_MEMBEROF_NODE_.base)

typedef struct jive_memberof_node_attrs jive_memberof_node_attrs;

typedef struct jive_memberof_node jive_memberof_node;

struct jive_memberof_node_attrs {
	jive_node_attrs base;
	const jive_record_layout * record_layout;
	size_t index;
};

struct jive_memberof_node {
	jive_node base;
	jive_memberof_node_attrs attrs;
};

struct jive_node *
jive_memberof_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_record_layout * record_layout, size_t index);

struct jive_node *
jive_memberof_create(struct jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index);

jive_output *
jive_memberof(jive_output * address,
	const jive_record_layout * record_layout, size_t index);

JIVE_EXPORTED_INLINE jive_memberof_node *
jive_memberof_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_MEMBEROF_NODE)
		return (jive_memberof_node *) node;
	else
		return NULL;
}

/* "containerof" operator: given an address that is the start of a record
member in memory, compute address of containing record */

extern const jive_unary_operation_class JIVE_CONTAINEROF_NODE_;
#define JIVE_CONTAINEROF_NODE (JIVE_CONTAINEROF_NODE_.base)

typedef struct jive_containerof_node_attrs jive_containerof_node_attrs;

typedef struct jive_containerof_node jive_containerof_node;

struct jive_containerof_node_attrs {
	jive_node_attrs base;
	const jive_record_layout * record_layout;
	size_t index;
};

struct jive_containerof_node {
	jive_node base;
	jive_containerof_node_attrs attrs;
};

struct jive_node *
jive_containerof_node_create(struct jive_region * region,
	struct jive_output * address,
	const struct jive_record_layout * record_layout, size_t index);

struct jive_node *
jive_containerof_create(struct jive_region * region,
	jive_output * address,
	const jive_record_layout * record_layout, size_t index);

jive_output *
jive_containerof(jive_output * address,
	const jive_record_layout * record_layout, size_t index);

JIVE_EXPORTED_INLINE jive_containerof_node *
jive_containerof_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_CONTAINEROF_NODE)
		return (jive_containerof_node *) node;
	else
		return NULL;
}

#endif
