/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/frontend/cfg.h>
#include <jive/frontend/cfg_node.h>
#include <jive/frontend/cfg-scc.h>
#include <jive/util/hash.h>
#include <jive/util/math.h>
#include <jive/util/stack.h>
#include <jive/util/vector.h>

typedef struct cfg_node_item cfg_node_item;
struct cfg_node_item {
	jive_cfg_node * cfg_node;
	size_t index;
	size_t lowlink;
	struct {
		struct cfg_node_item * prev;
		struct cfg_node_item * next;
	} hash_chain;
};

static inline cfg_node_item *
cfg_node_item_create(struct jive_context * context, struct jive_cfg_node * cfg_node, size_t index,
	size_t lowlink)
{
	cfg_node_item * item = jive_context_malloc(context, sizeof(*item));
	item->cfg_node = cfg_node;
	item->index = index;
	item->lowlink = lowlink;

	return item;
}

JIVE_DECLARE_HASH_TYPE(index_map, cfg_node_item, jive_cfg_node *, cfg_node, hash_chain);
JIVE_DEFINE_HASH_TYPE(index_map, cfg_node_item, jive_cfg_node *, cfg_node, hash_chain);

JIVE_DECLARE_VECTOR_TYPE(cfg_node_item_vector, cfg_node_item *);
JIVE_DEFINE_VECTOR_TYPE(cfg_node_item_vector, cfg_node_item *);

JIVE_DECLARE_STACK_TYPE(cfg_node_stack, jive_cfg_node *);
JIVE_DEFINE_STACK_TYPE(cfg_node_stack, jive_cfg_node *);


static jive_cfg_scc *
jive_cfg_scc_create(struct jive_context * context)
{
	jive_cfg_scc * scc = jive_context_malloc(context, sizeof(*scc));
	jive_cfg_scc_init(scc, context);
	return scc;
}

static void
jive_cfg_scc_destroy(struct jive_cfg_scc * scc)
{
	jive_context * context = scc->context;
	jive_cfg_scc_fini(scc);
	jive_context_free(context, scc);
}

struct jive_cfg_scc_set *
jive_cfg_scc_set_create(struct jive_context * context)
{
	jive_cfg_scc_set * set = jive_context_malloc(context, sizeof(*set));
	jive_cfg_scc_set_init(set, context);
	return set;
}

static void
jive_cfg_scc_set_fini_(struct jive_cfg_scc_set * self)
{
	struct jive_cfg_scc_set_iterator i;
	JIVE_SET_ITERATE(jive_cfg_scc_set, *self, i)
		jive_cfg_scc_destroy(i.entry->item);

	jive_cfg_scc_set_fini(self);
}

void
jive_cfg_scc_set_destroy(struct jive_cfg_scc_set * self)
{
	jive_context * context = self->context;
	jive_cfg_scc_set_fini_(self);
	jive_context_free(context, self);
}

/* Tarjan's SCC algorithm */

static struct cfg_node_item_vector item_vector;
static struct index_map map;
static struct cfg_node_stack node_stack;
static size_t index = 0;

static void
strongconnect(struct jive_cfg_node * node, struct jive_cfg_scc_set * scc_set)
{
	cfg_node_item * item = cfg_node_item_create(node->cfg()->context, node, index, index);
	cfg_node_item_vector_push_back(&item_vector, node->cfg()->context, item);
	index_map_insert(&map, item);
	cfg_node_stack_push(&node_stack, node);
	index++;

	jive_cfg_node * successor = node->taken_successor();
	if (successor != NULL) {
		if (index_map_lookup(&map, successor) == NULL) {
			strongconnect(successor, scc_set);
			item->lowlink = jive_min_unsigned(item->lowlink, index_map_lookup(&map, successor)->lowlink);
		} else if (cfg_node_stack_contains(&node_stack, successor))
			item->lowlink = jive_min_unsigned(item->lowlink, index_map_lookup(&map, successor)->index);
	}

	successor = node->nottaken_successor();
	if (successor != NULL) {
		if (index_map_lookup(&map, successor) == NULL) {
			strongconnect(successor, scc_set);
			item->lowlink = jive_min_unsigned(item->lowlink, index_map_lookup(&map, successor)->lowlink);
		} else if (cfg_node_stack_contains(&node_stack, successor))
			item->lowlink = jive_min_unsigned(item->lowlink, index_map_lookup(&map, successor)->index);
	}

	if (item->lowlink == item->index) {
		jive_cfg_scc * scc = jive_cfg_scc_create(scc_set->context);
		jive_cfg_node * w;
		do {
			w = cfg_node_stack_poptop(&node_stack);
			jive_cfg_scc_insert(scc, w);
		} while (w != node);
		jive_cfg_scc_set_insert(scc_set, scc);
	}
}

void
jive_cfg_find_sccs(struct jive_cfg * cfg, struct jive_cfg_scc_set * scc_set)
{
	jive_context * context = scc_set->context;
	jive_cfg_scc_set_fini_(scc_set);
	jive_cfg_scc_set_init(scc_set, context);

	/* initialization */
	cfg_node_item_vector_init(&item_vector);
	index_map_init(&map, cfg->context);
	cfg_node_stack_init(&node_stack, cfg->context);

	/* find strongly connected components */
	jive_cfg_node * cfg_node;
	JIVE_LIST_ITERATE(cfg->nodes, cfg_node, cfg_node_list) {
		if (index_map_lookup(&map, cfg_node) == NULL)
			strongconnect(cfg_node, scc_set);
	}

	/* finalization */
	size_t n;
	for (n = 0; n < cfg_node_item_vector_size(&item_vector); n++)
		jive_context_free(cfg->context, cfg_node_item_vector_item(&item_vector, n));
	cfg_node_item_vector_fini(&item_vector, cfg->context);
	index_map_fini(&map);
	cfg_node_stack_fini(&node_stack);
}
