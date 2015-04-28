/*
 * Copyright 2010 2011 2012 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/regalloc/shaped-region.h>

#include <jive/regalloc/shaped-graph.h>
#include <jive/regalloc/xpoint.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/region.h>

jive_cut::~jive_cut() noexcept
{
}

jive_shaped_node *
jive_cut::insert(jive_shaped_node * before, jive_node * node)
{
	jive_shaped_node * shaped_node = shaped_region().shaped_graph().map_node(node);
	shaped_node->add_to_cut(this, before);
	return shaped_node;
}


void
jive_cut::remove_nodes() noexcept
{
	while (!nodes_.empty()) {
		nodes_.begin()->remove_from_cut();
	}
}

jive_cut *
jive_cut::split(jive_shaped_node * at)
{
	nodes_list::iterator iter_at = nodes_.make_element_iterator(at);
	JIVE_DEBUG_ASSERT(at == nullptr || at->cut() == this);

	jive_shaped_region::cut_list & cuts = shaped_region().cuts();

	if (iter_at != nodes_.begin()) {
		jive_cut * above = shaped_region().create_cut(
			cuts.make_element_iterator(this));
		above->nodes_.splice(above->nodes_.begin(), nodes_, nodes_.begin(), iter_at);
		for (jive_shaped_node & shaped_node : above->nodes_) {
			shaped_node.cut_ = above;
		}
	}

	if (!nodes_.empty()) {
		jive_cut * below = shaped_region().create_cut(
			std::next(cuts.make_element_iterator(this)));
		below->nodes_.swap(nodes_);
		for (jive_shaped_node & shaped_node : below->nodes_) {
			shaped_node.cut_ = below;
		}
	}

	return this;
}


jive_cut *
jive_shaped_region::create_cut(cut_list::iterator before)
{
	return cuts_.insert(before, std::unique_ptr<jive_cut>(new jive_cut(this))).ptr();
}

jive_shaped_node *
jive_shaped_region::first_in_region() noexcept
{
	for (jive_cut & cut : cuts_) {
		if (!cut.nodes_.empty()) {
			return cut.nodes_.begin().ptr();
		}
	}
	return nullptr;
}

jive_shaped_node *
jive_shaped_region::last_in_region() noexcept
{
	auto i = cuts_.end();
	while (i != cuts_.begin()) {
		--i;
		if (!i->nodes_.empty()) {
			return std::prev(i->nodes_.end()).ptr();
		}
	}
	return nullptr;
}

jive_shaped_region::~jive_shaped_region() noexcept
{
}

void
jive_shaped_region::clear_cuts() noexcept
{
	for (jive_cut & cut : cuts_) {
		cut.remove_nodes();
	}

	cuts_.clear();
}

void
jive_shaped_region::add_active_top(
	jive_shaped_ssavar * shaped_ssavar,
	size_t count)
{
	if (active_top_.ssavar_add(shaped_ssavar, count) == 0) {
		shaped_graph().on_shaped_region_ssavar_add(this, shaped_ssavar);
	}
}

void
jive_shaped_region::remove_active_top(
	jive_shaped_ssavar * shaped_ssavar,
	size_t count) noexcept
{
	if (active_top_.ssavar_remove(shaped_ssavar, count) == count) {
		shaped_graph().on_shaped_region_ssavar_remove(this, shaped_ssavar);
	}
}

void
jive_shaped_region::debug_stream(const std::string& indent, std::ostream& os) const
{
	for (const jive_cut & cut : cut_list()) {
		for (const jive_shaped_node & shaped_node : cut.nodes_) {
			jive_node * node = shaped_node.node();
			for (size_t n = 0; n < node->ninputs; ++n) {
				jive::input * input = node->inputs[n];
				if (dynamic_cast<const jive::achr::type*>(&input->type())) {
					os << indent << "subregion\n";
					jive_region * sub_region = input->origin()->node()->region;
					shaped_graph().map_region(sub_region)->debug_stream(indent + " ", os);
				}
			}
			os << indent << "pre : " << shaped_node.get_active_before().debug_string() << "\n";
			os << indent << node << "\n";
			os << indent << "post: " << shaped_node.get_active_after().debug_string() << "\n";
		}
	}
}

