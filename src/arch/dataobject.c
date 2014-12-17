/*
 * Copyright 2010 2011 2012 2013 2014 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/dataobject.h>

#include <jive/arch/memlayout.h>
#include <jive/arch/memorytype.h>
#include <jive/types/bitstring/constant.h>
#include <jive/types/bitstring/type.h>
#include <jive/types/record/rcdgroup.h>
#include <jive/types/record/rcdtype.h>
#include <jive/types/union/unntype.h>
#include <jive/types/union/unnunify.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/controltype.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node-private.h>
#include <jive/vsdg/statetype.h>

namespace jive {

dataobj_head_op::~dataobj_head_op() noexcept
{
}

size_t
dataobj_head_op::narguments() const noexcept
{
	return types_.size();
}

const base::type &
dataobj_head_op::argument_type(size_t index) const noexcept
{
	return *types_[index];
}

size_t
dataobj_head_op::nresults() const noexcept
{
	return 1;
}

const base::type &
dataobj_head_op::result_type(size_t index) const noexcept
{
	static const ctl::type type;
	return type;
}

jive_node *
dataobj_head_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

std::string
dataobj_head_op::debug_string() const
{
	return "DATA_HEAD";
}

std::unique_ptr<jive::operation>
dataobj_head_op::copy() const
{
	return std::unique_ptr<jive::operation>(new dataobj_head_op(*this));
}

dataobj_tail_op::~dataobj_tail_op() noexcept
{
}

size_t
dataobj_tail_op::narguments() const noexcept
{
	return 1;
}

const base::type &
dataobj_tail_op::argument_type(size_t index) const noexcept
{
	static const ctl::type type;
	return type;
}

jive_node *
dataobj_tail_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

std::string
dataobj_tail_op::debug_string() const
{
	return "DATA_TAIL";
}

std::unique_ptr<jive::operation>
dataobj_tail_op::copy() const
{
	return std::unique_ptr<jive::operation>(new dataobj_tail_op(*this));
}

dataobj_op::~dataobj_op() noexcept
{
}

size_t
dataobj_op::nresults() const noexcept
{
	return 1;
}

const base::type &
dataobj_op::result_type(size_t index) const noexcept
{
	/* FIXME: a data object should not have a memory type as output */
	static const jive::mem::type objstate_type;
	return objstate_type;
}

jive_node *
dataobj_op::create_node(
	jive_region * region,
	size_t narguments,
	jive::output * const arguments[]) const
{
	return jive_opnode_create(*this, region, arguments, arguments + narguments);
}

std::string
dataobj_op::debug_string() const
{
	return "DATA";
}

std::unique_ptr<jive::operation>
dataobj_op::copy() const
{
	return std::unique_ptr<jive::operation>(new dataobj_op(*this));
}

}


static inline bool
is_powerof2(size_t v)
{
	return (v & (v-1)) == 0;
}

static void
squeeze_data_items(std::vector<jive::output *> & items)
{
	size_t k = 0;
	for (size_t n = 0; n < items.size(); n++) {
		if (items[n])
			items[k++] = items[n];
	}
	items.resize(k);
}

static std::vector<jive::output *>
flatten_data_items(
	jive::output * data,
	jive_memlayout_mapper * layout_mapper)
{
	std::vector<jive::output *> items;
	const jive::base::type * type_ = &data->type();
	if (dynamic_cast<const jive::bits::type*>(type_)) {
		const jive::bits::type * type = static_cast<const jive::bits::type*>(type_);
		
		if (type->nbits() < 8 || !is_powerof2(type->nbits())) {
			throw jive::compiler_error(
				"Type mismatch: primitive data items must be power-of-two bytes in size");
		}

		items.resize(type->nbits() / 8, nullptr);
		items[0] = data;
	} else if (dynamic_cast<const jive::rcd::type*>(type_)) {
		const jive::rcd::type * type = static_cast<const jive::rcd::type*>(type_);
		const jive::rcd::declaration * decl = type->declaration();
		const jive_record_memlayout * layout = jive_memlayout_mapper_map_record(layout_mapper, decl);

		if (!dynamic_cast<const jive::rcd::group_op *>(&data->node()->operation())) {
			throw jive::compiler_error("Type mismatch: can only serialize simple record compounds");
		}

		jive_graph * graph = data->node()->graph;
		
		jive::output * zero_pad = jive_bitconstant(graph, 8, "00000000");
		items.resize(layout->base.total_size, zero_pad);

		for (size_t k = 0; k < decl->nelements; k++) {
			std::vector<jive::output *> sub_items = flatten_data_items(
				data->node()->inputs[k]->origin(),
				layout_mapper);
			
			if (sub_items.size() + layout->element[k].offset > items.size()) {
				throw jive::compiler_error("Invalid record layout: element exceeds record");
			}
			
			for (size_t n = 0; n < sub_items.size(); n++) {
				if (items[n + layout->element[k].offset] != zero_pad) {
					throw jive::compiler_error("Invalid record layout: members overlap");
				}
				items[n + layout->element[k].offset] = sub_items[n];
			}
		}
	} else if (dynamic_cast<const jive::unn::type*>(type_)) {
		const jive::unn::type * type = static_cast<const jive::unn::type*>(type_);
		const jive::unn::declaration * decl = type->declaration();
		const jive_union_memlayout * layout = jive_memlayout_mapper_map_union(layout_mapper, decl);
		
		if (!dynamic_cast<const jive::unn::unify_op *>(&data->node()->operation())) {
			throw jive::compiler_error("Type mismatch: can only serialize simple union compounds");
		}
		
		jive_graph * graph = data->node()->graph;
		
		jive::output * zero_pad = jive_bitconstant(graph, 8, "00000000");
		items.resize(layout->base.total_size, zero_pad);
		
		std::vector<jive::output *> sub_items = flatten_data_items(
			data->node()->inputs[0]->origin(), layout_mapper);
		
		if (sub_items.size() > items.size()) {
			throw jive::compiler_error("Invalid union layout: element exceeds union");
		}
		
		std::copy(sub_items.begin(), sub_items.end(), items.begin());
	} else {
		throw jive::compiler_error("Type mismatch: can only serialize record and primitive data items");
	}

	return items;
}

static jive::output *
jive_dataobj_internal(
	jive::output * data,
	jive_memlayout_mapper * layout_mapper,
	jive_region * parent,
	jive_region * region)
{
	std::vector<jive::output *> data_items = flatten_data_items(data, layout_mapper);
	squeeze_data_items(data_items);
	std::vector<std::unique_ptr<const jive::base::type>> types;
	jive::output * arguments[data_items.size()];
	for (size_t n = 0; n < data_items.size(); ++n) {
		types.emplace_back(data_items[n]->type().copy());
		arguments[n] = data_items[n];
	}

	jive_node * head = jive::dataobj_head_op(std::move(types)).create_node(
		region, data_items.size(), arguments);
	jive_node * tail = jive::dataobj_tail_op().create_node(
		region, 1, &head->outputs[0]);
	jive_node * obj = jive::dataobj_op().create_node(
		parent, 1, &tail->outputs[0]);

	return obj->outputs[0];
}

jive::output *
jive_dataobj(jive::output * data, jive_memlayout_mapper * layout_mapper)
{
	jive_region * parent = data->node()->graph->root_region;
	jive_region * region = jive_region_create_subregion(parent);
	region->attrs.section = jive_region_section_data;

	return jive_dataobj_internal(data, layout_mapper, parent, region);
}

jive::output *
jive_rodataobj(jive::output * data, jive_memlayout_mapper * layout_mapper)
{
	jive_region * parent = data->node()->graph->root_region;
	jive_region * region = jive_region_create_subregion(parent);
	region->attrs.section = jive_region_section_rodata;

	return jive_dataobj_internal(data, layout_mapper, parent, region);
}

jive::output *
jive_bssobj(jive::output * data, jive_memlayout_mapper * layout_mapper)
{
	jive_region * parent = data->node()->graph->root_region;
	jive_region * region = jive_region_create_subregion(parent);
	region->attrs.section = jive_region_section_bss;

	return jive_dataobj_internal(data, layout_mapper, parent, region);
}
