/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_NODE_H
#define JIVE_FRONTEND_CFG_NODE_H

#include <jive/common.h>

#include <stdbool.h>
#include <stddef.h>

#include <string>

class jive_cfg_node {
public:
	virtual ~jive_cfg_node() noexcept;

protected:
	jive_cfg_node(struct jive_cfg * cfg) noexcept;

public:
	virtual std::string debug_string() const = 0;

	struct jive_cfg * cfg;

	struct {
		struct jive_cfg_node * first;
		struct jive_cfg_node * last;
	} taken_predecessors;

	struct {
		struct jive_cfg_node * prev;
		struct jive_cfg_node * next;
	} taken_predecessors_list;

	struct {
		struct jive_cfg_node * first;
		struct jive_cfg_node * last;
	} nottaken_predecessors;

	struct {
		struct jive_cfg_node * prev;
		struct jive_cfg_node * next;
	}	nottaken_predecessors_list;

	struct jive_cfg_node * taken_successor;
	struct jive_cfg_node * nottaken_successor;

	struct {
		struct jive_cfg_node * prev;
		struct jive_cfg_node * next;
	} cfg_node_list;
};

void
jive_cfg_node_connect_taken_successor(struct jive_cfg_node * self,
	struct jive_cfg_node * successor);

void
jive_cfg_node_connect_nottaken_successor(struct jive_cfg_node * self,
	struct jive_cfg_node * successor);

void
jive_cfg_node_disconnect_taken_successor(struct jive_cfg_node * self);

void
jive_cfg_node_disconnect_nottaken_successor(struct jive_cfg_node * self);

JIVE_EXPORTED_INLINE void
jive_cfg_node_disconnect_successors(struct jive_cfg_node * self)
{
	jive_cfg_node_disconnect_taken_successor(self);
	jive_cfg_node_disconnect_nottaken_successor(self);
}

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_taken_successor(struct jive_cfg_node * self, struct jive_cfg_node * successor)
{
	jive_cfg_node_disconnect_taken_successor(self);
	jive_cfg_node_connect_taken_successor(self, successor);
}

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_nottaken_successor(struct jive_cfg_node * self,
	struct jive_cfg_node * successor)
{
	jive_cfg_node_disconnect_nottaken_successor(self);
	jive_cfg_node_connect_nottaken_successor(self, successor);
}

void
jive_cfg_node_disconnect_taken_predecessors(struct jive_cfg_node * self);

void
jive_cfg_node_disconnect_nottaken_predecessors(struct jive_cfg_node * self);

JIVE_EXPORTED_INLINE void
jive_cfg_node_disconnect_predecessors(struct jive_cfg_node * self)
{
	jive_cfg_node_disconnect_taken_predecessors(self);
	jive_cfg_node_disconnect_nottaken_predecessors(self);
}

void
jive_cfg_node_divert_taken_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node);

void
jive_cfg_node_divert_nottaken_predecessors(struct jive_cfg_node * self,
	struct jive_cfg_node * node);

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node)
{
	jive_cfg_node_divert_taken_predecessors(self, node);
	jive_cfg_node_divert_nottaken_predecessors(self, node);
}

void
jive_cfg_node_destroy(struct jive_cfg_node * self);

#endif
