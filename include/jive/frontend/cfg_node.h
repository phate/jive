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

	inline struct jive_cfg * cfg() const noexcept { return cfg_; }

	void add_taken_successor(jive_cfg_node * successor) noexcept;
	void add_nottaken_successor(jive_cfg_node * successor) noexcept;

	void remove_taken_successor() noexcept;
	void remove_nottaken_successor() noexcept;
	inline void remove_successors() noexcept { remove_taken_successor(); remove_nottaken_successor(); }

	void remove_taken_predecessors() noexcept;
	void remove_nottaken_predecessors() noexcept;
	inline void remove_predecessors() noexcept {
		remove_taken_predecessors();
		remove_nottaken_predecessors();
	}

	inline void divert_taken_successor(jive_cfg_node * successor) noexcept {
		remove_taken_successor();
		add_taken_successor(successor);
	}

	inline void divert_nottaken_successor(jive_cfg_node * successor) noexcept {
		remove_nottaken_successor();
		add_nottaken_successor(successor);
	}

	void divert_taken_predecessors(jive_cfg_node * node) noexcept;
	void divert_nottaken_predecessors(jive_cfg_node * node) noexcept;
	inline void divert_predecessors(jive_cfg_node * node) noexcept {
		divert_taken_predecessors(node);
		divert_nottaken_predecessors(node);
	}

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

private:
	struct jive_cfg * cfg_;
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
	self->remove_successors();
}

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_taken_successor(struct jive_cfg_node * self, struct jive_cfg_node * successor)
{
	self->divert_taken_successor(successor);
}

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_nottaken_successor(struct jive_cfg_node * self,
	struct jive_cfg_node * successor)
{
	self->divert_nottaken_successor(successor);
}

void
jive_cfg_node_disconnect_taken_predecessors(struct jive_cfg_node * self);

void
jive_cfg_node_disconnect_nottaken_predecessors(struct jive_cfg_node * self);

JIVE_EXPORTED_INLINE void
jive_cfg_node_disconnect_predecessors(struct jive_cfg_node * self)
{
	self->remove_predecessors();
}

void
jive_cfg_node_divert_taken_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node);

void
jive_cfg_node_divert_nottaken_predecessors(struct jive_cfg_node * self,
	struct jive_cfg_node * node);

JIVE_EXPORTED_INLINE void
jive_cfg_node_divert_predecessors(struct jive_cfg_node * self, struct jive_cfg_node * node)
{
	self->divert_predecessors(node);
}

void
jive_cfg_node_destroy(struct jive_cfg_node * self);

#endif
