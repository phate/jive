/*
 * Copyright 2010 2011 2012 2013 2014 2015 Helge Bahmann <hcb@chaoticmind.net>
 * Copyright 2011 2012 2014 2015 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <jive/arch/sequence.h>

#include <jive/arch/dataobject.h>
#include <jive/arch/instruction.h>
#include <jive/arch/label-mapper.h>
#include <jive/types/bitstring.h>
#include <jive/vsdg.h>
#include <jive/vsdg/anchortype.h>
#include <jive/vsdg/objdef.h>

void
jive_seq_point_init(jive_seq_point * self, jive_seq_region * seq_region, jive_node * node)
{
	jive_seq_graph * seq = seq_region->seq_graph;
	self->seq_region = seq_region;
	self->size = 0;
	
	self->local_symbol = false;
	
	jive_address_init(&self->address, jive_stdsectionid_invalid, 0);
	self->node = node;
	if (self->node)
		seq->node_map.insert(self);
}

void
jive_seq_point_fini_(jive_seq_point * self)
{
	jive_seq_graph * seq_graph = self->seq_region->seq_graph;
	if (self->node)
		seq_graph->node_map.erase(self);
	JIVE_LIST_REMOVE(seq_graph->points, self, seqpoint_list);
}

const jive_seq_point_class JIVE_SEQ_POINT = {
	parent : 0,
	fini : jive_seq_point_fini_
};

static void
jive_seq_node_fini_(jive_seq_point * self_)
{
	jive_seq_node * self = (jive_seq_node *) self_;
	jive_seq_point_fini_(&self->base);
}

const jive_seq_point_class JIVE_SEQ_NODE = {
	parent : &JIVE_SEQ_POINT,
	fini : jive_seq_node_fini_
};

static jive_seq_point *
jive_seq_node_create(jive_seq_graph * seq, jive_seq_region * seq_region, jive_node * node)
{
	jive_seq_node * self = new jive_seq_node;
	self->base.class_ = &JIVE_SEQ_NODE;
	
	jive_seq_point_init(&self->base, seq_region, node);
	
	return &self->base;
}

static void
jive_seq_point_attach_symbol(
	jive_seq_point * self,
	const jive_linker_symbol * symbol,
	jive_seq_graph * seq_graph)
{
	self->named_symbols.push_back(symbol);
}

static jive_seq_region *
sequentialize_region(
	jive_seq_graph * seq,
	jive_seq_point * before,
	jive::bottomup_region_traverser * region_trav,
	jive_region * region)
{
	jive_seq_region * seq_region = new jive_seq_region;
	seq_region->region = region;
	seq_region->seq_graph = seq;
	seq_region->first_point = 0;
	seq_region->last_point = 0;
	seq_region->inlined = false;
	seq->region_map.insert(std::unique_ptr<jive_seq_region>(seq_region));
	
	jive::bottomup_slave_traverser * trav = region_trav->map_region(region);
	
	jive_node * node = trav->next();
	while (node) {
		jive_seq_point * current;
		if (auto i_op = dynamic_cast<const jive::instruction_op *>(&node->operation())) {
			const jive_instruction_class * icls = i_op->icls();
			const jive_register_name * inregs[icls->ninputs];
			const jive_register_name * outregs[icls->noutputs];
			jive_seq_imm immediates[icls->nimmediates];
			size_t n;
			for (n = 0; n < icls->ninputs; ++n)
				inregs[n] = (const jive_register_name *)node->input(n)->ssavar()->variable->resname;
			for (n = 0; n < icls->noutputs; ++n)
				outregs[n] = (const jive_register_name *)node->output(n)->ssavar()->variable->resname;
			current = &jive_seq_instruction_create(
				seq_region,
				icls,
				inregs,
				outregs,
				immediates,
				node)->base;
		} else if (dynamic_cast<const jive::dataobj_head_op *>(&node->operation())) {
			jive_seq_data * data = jive_seq_data_create(
				seq_region, node->ninputs(), node);
			size_t n;
			for (n = 0; n < node->ninputs(); ++n) {
				jive::input * input = node->input(n);
				const jive::bits::constant_op * cop = dynamic_cast<const jive::bits::constant_op *>(
					&input->origin()->node()->operation());
				jive_seq_dataitem * item = &data->items[n];
				if (cop) {
					switch (cop->value().nbits()) {
						case 8:
							item->format = jive_seq_dataitem_fmt_8;
							break;
						case 16:
							item->format = jive_seq_dataitem_fmt_le16;
							break;
						case 32:
							item->format = jive_seq_dataitem_fmt_le32;
							break;
						case 64:
							item->format = jive_seq_dataitem_fmt_le64;
							break;
						default:
							item->format = jive_seq_dataitem_fmt_none;
							break;
					}
					item->value = cop->value().to_uint();
				} else {
					item->format = jive_seq_dataitem_fmt_none;
				}
			}
			current = &data->base;
		} else {
			current = jive_seq_node_create(seq, seq_region, node);
		}
		
		JIVE_LIST_INSERT(seq->points, before, current, seqpoint_list);
		
		size_t n;
		for(n = 0; n < node->ninputs(); n++) {
			jive::input * input = node->input(n);
			if (dynamic_cast<const jive::achr::type*>(&input->type())) {
				jive_seq_region * seq_subregion;
				if (n == 0) {
					seq_subregion = sequentialize_region(seq, current, region_trav,
						input->producer()->region());
					
					if (!seq_region->last_point)
						seq_region->last_point = seq_subregion->last_point;
					
					current = seq_subregion->first_point;
					
					seq_subregion->inlined = true;
				} else {
					seq_subregion = sequentialize_region(seq, 0, region_trav, input->producer()->region());
					seq_subregion->inlined = false;
				}
			}
		}
		
		seq_region->first_point = current;
		if (!seq_region->last_point)
			seq_region->last_point = current;
		
		before = current;
		
		jive::input * control_input = 0;
		for(n = 0; n < node->ninputs(); n++) {
			jive::input * input = node->input(n);
			if (dynamic_cast<const jive::ctl::type*>(&input->type())) {
				control_input = input;
				break;
			}
		}
		
		if (control_input) {
			node = control_input->producer();
			region_trav->pass(node);
		} else {
			node = trav->next();
		}
	}
	
	return seq_region;
}

static jive_seq_label
jive_seq_graph_convert_label(
	const jive_seq_graph * self,
	const jive_label * label,
	jive_seq_point * current)
{
	jive_seq_label result;
	if (!label) {
		result.type = jive_seq_label_type_none;
	} else if (jive_label_isinstance(label, &JIVE_LABEL_CURRENT)) {
		result.type = jive_seq_label_type_internal;
		result.internal = current;
		current->local_symbol = true;
	} else if (jive_label_isinstance(label, &JIVE_LABEL_EXTERNAL)) {
		result.type = jive_seq_label_type_external;
		result.external = ((const jive_label_external *) label)->symbol;
	} else {
		result.type = jive_seq_label_type_none;
	}
	return result;
}

jive_seq_graph *
jive_graph_sequentialize(jive_graph * graph)
{
	jive_seq_graph * seq = new jive_seq_graph;
	seq->graph = graph;
	seq->points.first = 0;
	seq->points.last = 0;
	seq->addrs_changed = true;
	
	/* sequentialize nodes of the graph */
	{
		jive::bottomup_region_traverser region_trav(graph);
		sequentialize_region(seq, 0, &region_trav, graph->root_region);
	}
	
	/* now that we have a sequence, we can fix up the labels: local
	labels can be mapped to their respective sequence point such that
	instruction immediates can be filled in; exported labels can now be
	attached to the points where the objects they represent are
	sequenced */
	jive_seq_point * seq_point;
	JIVE_LIST_ITERATE(seq->points, seq_point, seqpoint_list) {
		const jive::objdef_operation * odef =
			seq_point-> node ?
			dynamic_cast<const jive::objdef_operation *>(&seq_point->node->operation()) :
			nullptr;
		if (odef) {
			jive_node * anchor = seq_point->node->producer(0);
			jive_region * region = anchor->producer(0)->region();
			jive_seq_region * seq_region = jive_seq_graph_map_region(seq, region);
			jive_seq_point_attach_symbol(seq_region->first_point,
				odef->symbol(), seq);
		}
		jive_seq_instruction * seq_instr =
			jive_seq_instruction_cast(seq_point);
		if (!seq_instr)
			continue;
		const jive::instruction_op * i_op = dynamic_cast<const jive::instruction_op *>(
			&seq_point->node->operation());
		if (!i_op) {
			continue;
		}
		const jive_instruction_class * icls = i_op->icls();
		for (size_t n = 0; n < icls->nimmediates; ++ n) {
			jive::immediate imm = jive_instruction_node_get_immediate(seq_point->node, n);
			seq_instr->imm[n].value = imm.offset();
			seq_instr->imm[n].add_label = jive_seq_graph_convert_label(seq, imm.add_label(), seq_point);
			seq_instr->imm[n].sub_label = jive_seq_graph_convert_label(seq, imm.sub_label(), seq_point);
			seq_instr->imm[n].modifier = imm.modifier();
		}
	}
	
	return seq;
}

void
jive_seq_graph_destroy(jive_seq_graph * seq)
{
	jive_seq_point * sp, * next_p;
	JIVE_LIST_ITERATE_SAFE(seq->points, sp, next_p, seqpoint_list) {
		jive_seq_point_destroy(sp);
	}

	delete seq;
}

static void
jive_seq_instruction_fini_(jive_seq_point * self_)
{
	jive_seq_instruction * self = (jive_seq_instruction *) self_;
	delete[] self->inputs;
	delete[] self->outputs;
	delete[] self->imm;
	jive_seq_point_fini_(&self->base);
}

const jive_seq_point_class JIVE_SEQ_INSTRUCTION = {
	parent : &JIVE_SEQ_POINT,
	fini : jive_seq_instruction_fini_
};

jive_seq_instruction *
jive_seq_instruction_create_shell(
	jive_seq_region * seq_region,
	const jive_instruction_class * icls,
	jive_node * node)
{
	jive_seq_instruction * seq_instr = new jive_seq_instruction;
	seq_instr->base.class_ = &JIVE_SEQ_INSTRUCTION;
	jive_seq_point_init(&seq_instr->base, seq_region, node);
	seq_instr->icls = icls;
	seq_instr->inputs = new const jive_register_name*[icls->ninputs];
	seq_instr->outputs = new const jive_register_name*[icls->noutputs];
	seq_instr->imm = new jive_seq_imm[icls->nimmediates];
	seq_instr->flags = jive_instruction_encoding_flags_none;
	
	return seq_instr;
}

jive_seq_instruction *
jive_seq_instruction_create(
	jive_seq_region * seq_region,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[],
	jive_node * node)
{
	jive_seq_instruction * seq_instr = jive_seq_instruction_create_shell(
		seq_region, icls, node);
	
	size_t n;
	for (n = 0; n < icls->ninputs; n++)
		seq_instr->inputs[n] = inputs[n];
	for (n = 0; n < icls->noutputs; n++)
		seq_instr->outputs[n] = outputs[n];
	for (n = 0; n < icls->nimmediates; n++)
		seq_instr->imm[n] = immediates[n];
	
	return seq_instr;
}

jive_seq_instruction *
jive_seq_instruction_create_before(
	jive_seq_point * before,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[])
{
	jive_seq_graph * seq = before->seq_region->seq_graph;
	jive_seq_instruction * seq_instr = jive_seq_instruction_create(
		before->seq_region, icls, inputs, outputs, immediates, NULL);
	
	JIVE_LIST_INSERT(seq->points, before, &seq_instr->base, seqpoint_list);
	if (before->seq_region->first_point == before)
		before->seq_region->first_point = &seq_instr->base;
	
	return seq_instr;
}

jive_seq_instruction *
jive_seq_instruction_create_after(
	jive_seq_point * after,
	const jive_instruction_class * icls,
	const jive_register_name * const * inputs,
	const jive_register_name * const * outputs,
	const jive_seq_imm immediates[])
{
	jive_seq_graph * seq = after->seq_region->seq_graph;
	jive_seq_instruction * seq_instr = jive_seq_instruction_create(
		after->seq_region, icls, inputs, outputs, immediates, NULL);
	
	JIVE_LIST_INSERT(seq->points, after->seqpoint_list.next, &seq_instr->base, seqpoint_list);
	
	return seq_instr;
}

static void
jive_seq_data_fini_(jive_seq_point * self_)
{
	jive_seq_data * self = (jive_seq_data *) self_;
	delete[] self->items;
	jive_seq_point_fini_(&self->base);
}

const jive_seq_point_class JIVE_SEQ_DATA = {
	parent : &JIVE_SEQ_POINT,
	fini : jive_seq_data_fini_
};

jive_seq_data *
jive_seq_data_create(
	jive_seq_region * seq_region,
	size_t nitems,
	jive_node * node)
{
	jive_seq_data * seq_data = new jive_seq_data;
	seq_data->base.class_ = &JIVE_SEQ_DATA;
	jive_seq_point_init(&seq_data->base, seq_region, node);
	seq_data->nitems = nitems;
	seq_data->items = new jive_seq_dataitem[nitems];
	
	return seq_data;
}
