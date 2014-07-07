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
#include <vector>


class jive_cfg_node {
public:
	virtual ~jive_cfg_node();

protected:
	jive_cfg_node(struct jive_cfg * cfg);

public:
	virtual std::string debug_string() const = 0;

	inline struct jive_cfg * cfg() const noexcept { return cfg_; }

	void add_taken_successor(jive_cfg_node * successor);
	void add_nottaken_successor(jive_cfg_node * successor);

	void remove_taken_successor();
	void remove_nottaken_successor();
	inline void remove_successors() { remove_taken_successor(); remove_nottaken_successor(); }

	void remove_predecessors();

	inline void divert_taken_successor(jive_cfg_node * successor) {
		remove_taken_successor();
		add_taken_successor(successor);
	}

	inline void divert_nottaken_successor(jive_cfg_node * successor) {
		remove_nottaken_successor();
		add_nottaken_successor(successor);
	}

	void divert_predecessors(jive_cfg_node * node);

	inline jive_cfg_node * taken_successor() const noexcept { return taken_successor_; }
	inline jive_cfg_node * nottaken_successor() const noexcept { return nottaken_successor_; }

	inline std::vector<jive_cfg_node*> predecessors() const noexcept { return predecessors_; }

	struct {
		struct jive_cfg_node * prev;
		struct jive_cfg_node * next;
	} cfg_node_list;

private:
	jive_cfg_node * taken_successor_;
	jive_cfg_node * nottaken_successor_;
	std::vector<jive_cfg_node*> predecessors_;
	jive_cfg * cfg_;
};

#endif
