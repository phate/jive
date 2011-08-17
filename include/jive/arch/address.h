#ifndef JIVE_ARCH_ADDRESS_H
#define JIVE_ARCH_ADDRESS_H

#include <jive/common.h>
#include <jive/arch/addresstype.h>
#include <jive/bitstring/type.h>
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

/* "arraysubscript" operator: given an address that points to an element of
an array, compute address of element offset by specified distance */

extern const jive_binary_operation_class JIVE_ARRAYSUBSCRIPT_NODE_;
#define JIVE_ARRAYSUBSCRIPT_NODE (JIVE_ARRAYSUBSCRIPT_NODE_.base)

typedef struct jive_arraysubscript_node_attrs jive_arraysubscript_node_attrs;

typedef struct jive_arraysubscript_node jive_arraysubscript_node;

struct jive_arraysubscript_node_attrs {
	jive_node_attrs base;
	jive_type * element_type; /* note: dynamically allocated */
};

struct jive_arraysubscript_node {
	jive_node base;
	jive_arraysubscript_node_attrs attrs;
};

struct jive_node *
jive_arraysubscript_node_create(struct jive_region * region,
	struct jive_output * address, const struct jive_type * element_type, struct jive_output * index);

struct jive_node *
jive_arraysubscript_create(struct jive_region * region,
	struct jive_output * address, const struct jive_type * element_type, struct jive_output * index);

jive_output *
jive_arraysubscript(struct jive_output * address, const struct jive_type * element_type, struct jive_output * index);

JIVE_EXPORTED_INLINE jive_arraysubscript_node *
jive_arraysubscript_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_ARRAYSUBSCRIPT_NODE)
		return (jive_arraysubscript_node *) node;
	else
		return NULL;
}

/* "arrayindex" operator: given an address that points to an element of
an array, compute address of element offset by specified distance */

extern const jive_binary_operation_class JIVE_ARRAYINDEX_NODE_;
#define JIVE_ARRAYINDEX_NODE (JIVE_ARRAYINDEX_NODE_.base)

typedef struct jive_arrayindex_node_attrs jive_arrayindex_node_attrs;

typedef struct jive_arrayindex_node jive_arrayindex_node;

struct jive_arrayindex_node_attrs {
	jive_node_attrs base;
	jive_type * element_type; /* note: dynamically allocated */
	jive_bitstring_type difference_type;
};

struct jive_arrayindex_node {
	jive_node base;
	jive_arrayindex_node_attrs attrs;
};

struct jive_node *
jive_arrayindex_node_create(struct jive_region * region,
	struct jive_output * addr1, struct jive_output * addr2,
	const struct jive_type * element_type, const struct jive_type * difference_type);

struct jive_node *
jive_arrayindex_create(struct jive_region * region,
	struct jive_output * addr1, struct jive_output * addr2,
	const struct jive_type * element_type, const struct jive_type * difference_type);

jive_output *
jive_arrayindex(struct jive_output * addr1, struct jive_output * addr2,
	const struct jive_type * element_type, const struct jive_type * difference_type);

JIVE_EXPORTED_INLINE jive_arrayindex_node *
jive_arrayindex_node_cast(jive_node * node)
{
	if (node->class_ == &JIVE_ARRAYINDEX_NODE)
		return (jive_arrayindex_node *) node;
	else
		return NULL;
}


#endif
