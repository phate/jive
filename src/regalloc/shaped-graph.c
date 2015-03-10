/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-graph.h>

#include <jive/common.h>
#include <jive/regalloc/assignment-tracker.h>
#include <jive/regalloc/shaped-variable.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>
#include <jive/vsdg/variable.h>

using namespace std::placeholders;

static void
jive_shaped_graph_gate_interference_add(void * closure, jive::gate * first, jive::gate * second)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	if (first->variable && second->variable) {
		jive_shaped_variable::interference_add(
			shaped_graph->map_variable(first->variable),
			shaped_graph->map_variable(second->variable)
		);
	}
}

static void
jive_shaped_graph_gate_interference_remove(void * closure, jive::gate * first, jive::gate * second)
{
	jive_shaped_graph * shaped_graph = (jive_shaped_graph *) closure;
	if (first->variable && second->variable) {
		jive_shaped_variable::interference_remove(
			shaped_graph->map_variable(first->variable),
			shaped_graph->map_variable(second->variable)
		);
	}
}

void
jive_shaped_graph::add_region_recursive(jive_region * region)
{
	region_map_.insert(std::unique_ptr<jive_shaped_region>(
		new jive_shaped_region(this, region)));

	jive_node * node;
	JIVE_LIST_ITERATE(region->nodes, node, region_nodes_list) {
		node_map_.insert(std::unique_ptr<jive_shaped_node>(
			new jive_shaped_node(this, node)));
	}

	jive_region * subregion;
	JIVE_LIST_ITERATE(region->subregions, subregion, region_subregions_list) {
		add_region_recursive(subregion);
	}
}

jive_shaped_graph::jive_shaped_graph(jive_graph * graph)
	: graph_(graph)
{
	callbacks_.push_back(graph_->on_region_create.connect(
		[this](jive_region * region)
		{
			region_map_.insert(std::unique_ptr<jive_shaped_region>(
				new jive_shaped_region(this, region)));
		}));
	callbacks_.push_back(graph_->on_region_destroy.connect(
		[this](jive_region * region)
		{
			region_map_.erase(region);
		}));
	callbacks_.push_back(graph_->on_region_add_used_ssavar.connect(
		[this](jive_region * region, jive_ssavar * ssavar)
		{
			ssavar_map_.find(ssavar)->xpoints_register_region_arc(
				map_node_location(ssavar->origin->node()),
				map_node_location(jive_region_get_bottom_node(region)));
		}));
	callbacks_.push_back(graph_->on_region_remove_used_ssavar.connect(
		[this](jive_region * region, jive_ssavar * ssavar)
		{
			ssavar_map_.find(ssavar)->xpoints_unregister_region_arc(
				map_node_location(ssavar->origin->node()),
				map_node_location(jive_region_get_bottom_node(region)));
		}));
	callbacks_.push_back(graph_->on_gate_interference_add.connect(
		std::bind(jive_shaped_graph_gate_interference_add, this, _1, _2)));
	callbacks_.push_back(graph_->on_gate_interference_remove.connect(
		std::bind(jive_shaped_graph_gate_interference_remove, this, _1, _2)));
	callbacks_.push_back(graph_->on_variable_create.connect(
		[this](jive_variable * variable)
		{
			jive_shaped_variable_create(this, variable);
		}));
	callbacks_.push_back(graph_->on_variable_destroy.connect(
		[this](jive_variable * variable)
		{
			this->variable_map_.erase(this->variable_map_.find(variable).ptr());
		}));
	callbacks_.push_back(graph_->on_variable_assign_gate.connect(
		[this](jive_variable * variable, jive::gate * gate)
		{
			this->variable_map_.find(variable)->assign_gate(gate);
		}));
	callbacks_.push_back(graph_->on_variable_unassign_gate.connect(
		[this](jive_variable * variable, jive::gate * gate)
		{
			this->variable_map_.find(variable)->unassign_gate(gate);
		}));
	callbacks_.push_back(graph_->on_variable_resource_class_change.connect(
		[this](
			jive_variable * variable,
			const jive_resource_class * old_rescls,
			const jive_resource_class * new_rescls)
		{
			jive_shaped_variable * shaped_variable = this->map_variable(variable);
			if (shaped_variable) {
				shaped_variable->resource_class_change(old_rescls, new_rescls);
			}
		}));
	callbacks_.push_back(graph_->on_variable_resource_name_change.connect(
		[this](
			jive_variable * variable,
			const jive_resource_name * old_resname,
			const jive_resource_name * new_resname)
		{
			jive_shaped_variable * shaped_variable = this->map_variable(variable);
			if (shaped_variable) {
				shaped_variable->resource_name_change(old_resname, new_resname);
			}
		}));
	callbacks_.push_back(graph_->on_ssavar_create.connect(
		[this](jive_ssavar * ssavar) {
			ssavar_map_.insert(std::unique_ptr<jive_shaped_ssavar>(
				new jive_shaped_ssavar(this, ssavar)));
		}));
	callbacks_.push_back(graph_->on_ssavar_destroy.connect(
		[this](jive_ssavar * ssavar) {
			ssavar_map_.erase(ssavar);
		}));
	callbacks_.push_back(graph_->on_node_create.connect(
		[this](jive_node * node)
		{
			node_map_.insert(std::unique_ptr<jive_shaped_node>(
				new jive_shaped_node(this, node)));
		}));
	callbacks_.push_back(graph_->on_node_destroy.connect(
		[this](jive_node * node)
		{
			auto i = this->node_map_.find(node);
			i->remove_from_cut();
			node_map_.erase(i);
		}));
	callbacks_.push_back(graph_->on_ssavar_assign_input.connect(
		[this](jive_ssavar * ssavar, jive::input * input)
		{
			map_ssavar(ssavar)->xpoints_register_arc(
				map_node_location(input->origin()->node()),
				map_node_location(input->node()));
		}));
	callbacks_.push_back(graph_->on_ssavar_unassign_input.connect(
		[this](jive_ssavar * ssavar, jive::input * input)
		{
			map_ssavar(ssavar)->xpoints_unregister_arc(
				map_node_location(input->origin()->node()),
				map_node_location(input->node()));
		}));
	callbacks_.push_back(graph_->on_ssavar_assign_output.connect(
		[this](jive_ssavar * ssavar, jive::output * output)
		{
			jive_shaped_node * shaped_node = map_node_location(output->node());
			if (shaped_node) {
				shaped_node->add_ssavar_after(map_ssavar(ssavar), ssavar->variable, 1);
			}
		}));
	callbacks_.push_back(graph_->on_ssavar_unassign_output.connect(
		[this](jive_ssavar * ssavar, jive::output * output)
		{
			jive_shaped_node * shaped_node = map_node_location(output->node());
			if (shaped_node) {
				shaped_node->remove_ssavar_after(map_ssavar(ssavar), ssavar->variable, 1);
			}
		}));
	callbacks_.push_back(graph_->on_ssavar_variable_change.connect(
		[this](jive_ssavar * ssavar, jive_variable * old_var, jive_variable * new_var)
		{
			map_ssavar(ssavar)->xpoints_variable_change(old_var, new_var);
		}));
	callbacks_.push_back(graph_->on_ssavar_divert_origin.connect(
		[this](jive_ssavar * ssavar, jive::output * old_origin, jive::output * new_origin)
		{
			map_ssavar(ssavar)->notify_divert_origin(old_origin, new_origin);
		}));

	jive_variable * variable;
	JIVE_LIST_ITERATE(graph_->variables, variable, graph_variable_list) {
		jive_shaped_variable * shaped_variable = jive_shaped_variable_create(this, variable);
		jive_ssavar * ssavar;
		JIVE_LIST_ITERATE(variable->ssavars, ssavar, variable_ssavar_list) {
			ssavar_map_.insert(std::unique_ptr<jive_shaped_ssavar>(
				new jive_shaped_ssavar(this, ssavar)));
		}
		jive::gate * gate;
		JIVE_LIST_ITERATE(variable->gates, gate, variable_gate_list) {
			shaped_variable->initial_assign_gate(gate);
		}
	}

	add_region_recursive(graph_->root_region);
}

jive_shaped_graph::~jive_shaped_graph()
{
}

jive_shaped_graph *
jive_shaped_graph_create(jive_graph * graph)
{
	return new jive_shaped_graph(graph);
}

void
jive_shaped_graph_destroy(jive_shaped_graph * self)
{
	delete self;
}
