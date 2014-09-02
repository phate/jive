/*
 * Copyright 2013 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#ifndef JIVE_FRONTEND_CFG_NODE_H
#define JIVE_FRONTEND_CFG_NODE_H

#include <jive/common.h>

#include <stdbool.h>
#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

class jive_cfg_node;

namespace jive {
namespace frontend {

class cfg_edge final {
public:
	~cfg_edge() noexcept {};

	cfg_edge(jive_cfg_node * source, jive_cfg_node * sink, size_t index) noexcept;

	void divert(jive_cfg_node * new_sink, size_t new_index) noexcept;

	inline jive_cfg_node * source() const noexcept { return source_; }
	inline jive_cfg_node * sink() const noexcept { return sink_; }
	inline size_t index() const noexcept { return index_; }

private:
	jive_cfg_node * source_;
	jive_cfg_node * sink_;
	size_t index_;
};

}
}

class jive_cfg;

class jive_cfg_node {
public:
	virtual ~jive_cfg_node();

protected:
	jive_cfg_node(jive_cfg * cfg);

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

	inline jive::frontend::cfg_edge * taken_edge() const noexcept { return taken_edge_.get(); }
	inline jive::frontend::cfg_edge * nottaken_edge() const noexcept { return nottaken_edge_.get(); }
	jive_cfg_node * taken_successor() const noexcept;
	jive_cfg_node * nottaken_successor() const noexcept;

	inline size_t npredecessors() const noexcept { return predecessors_.size(); }
	inline std::vector<jive::frontend::cfg_edge *> predecessors() const noexcept {
		return predecessors_; }

	struct {
		struct jive_cfg_node * prev;
		struct jive_cfg_node * next;
	} cfg_node_list;

private:
	std::unique_ptr<jive::frontend::cfg_edge> taken_edge_;
	std::unique_ptr<jive::frontend::cfg_edge> nottaken_edge_;
	std::vector<jive::frontend::cfg_edge *> predecessors_;
	jive_cfg * cfg_;
};

#endif
