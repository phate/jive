#include <jive/regalloc/fixup.h>
#include <jive/vsdg/graph.h>
#include <jive/vsdg/node.h>
#include <jive/vsdg/traverser.h>
#include <jive/vsdg/cut.h>
#include <jive/vsdg/regcls-count-private.h>
#include <jive/arch/instruction.h>
#include <jive/regalloc/auxnodes.h>

static void
pre_op_transfer(jive_node * node, const jive_cpureg * new_cpureg)
{
	jive_output * origin = node->inputs[0]->origin;
	
	jive_node * xfer_node = jive_aux_valuecopy_node_create(node->region, jive_resource_get_regcls(origin->resource), origin);
	jive_input * xfer_input = xfer_node->inputs[0];
	jive_output * xfer_output = xfer_node->outputs[0];
	
	jive_resource * old_res = origin->resource;
	jive_resource_unassign_input(old_res, node->inputs[0]);
	jive_resource_assign_input(old_res, xfer_input);
	
	jive_resource * new_res = jive_type_create_resource(jive_output_get_type(xfer_output), node->graph);
	jive_resource_assign_output(new_res, xfer_output);
	jive_resource_assign_input(new_res, node->inputs[0]);
	
	jive_input_divert_origin(node->inputs[0], xfer_output);
	jive_cut_insert(node->shape_location->cut, node->shape_location->cut_nodes_list.next, xfer_node);
	
	jive_value_resource_recompute_regcls((jive_value_resource *) new_res);
	jive_value_resource_set_cpureg((jive_value_resource *) new_res, new_cpureg);
}

static void
post_op_transfer(jive_node * node, const jive_cpureg * new_cpureg)
{
	jive_output * origin = node->outputs[0];

	jive_node * xfer_node = jive_aux_valuecopy_node_create(node->region, jive_resource_get_regcls(origin->resource), origin);
	jive_input * xfer_input = xfer_node->inputs[0];
	jive_output * xfer_output = xfer_node->outputs[0];
	
	jive_resource * old_res = origin->resource;
	jive_resource_unassign_output(old_res, origin);
	jive_resource_assign_output(old_res, xfer_output);
	
	jive_resource * new_res = jive_type_create_resource(jive_output_get_type(xfer_output), node->graph);
	jive_resource_assign_output(new_res, origin);
	jive_resource_assign_input(new_res, xfer_input);
	
	jive_input * user, * next_user;
	JIVE_LIST_ITERATE_SAFE(origin->users, user, next_user, output_users_list) {
		if (user == xfer_input) continue;
		jive_input_divert_origin(user, xfer_output);
	}
	jive_cut_insert(node->shape_location->cut, node->shape_location->cut_nodes_list.next, xfer_node);
	
	jive_value_resource_recompute_regcls((jive_value_resource *) new_res);
	jive_value_resource_set_cpureg((jive_value_resource *) new_res, new_cpureg);
}

static void
process_node(jive_node * node_)
{
	if (!jive_node_isinstance(node_, &JIVE_INSTRUCTION_NODE)) return;
	jive_instruction_node * node = (jive_instruction_node *) node_;
	const struct jive_instruction_class * icls = node->attrs.icls;
	
	if ((icls->flags & jive_instruction_write_input) == 0) return;
	
	const jive_cpureg * inreg0 = jive_resource_get_cpureg(node->base.inputs[0]->resource);
	const jive_cpureg * outreg0 = jive_resource_get_cpureg(node->base.outputs[0]->resource);
	
	if (inreg0 == outreg0) return;
	
	const jive_regcls * regcls = icls->inregs[0];
	
	if (icls->flags & jive_instruction_commutative) {
		const jive_cpureg * inreg1 = jive_resource_get_cpureg(node->base.inputs[1]->resource);
		/* if it is possible to satify constraints by simply swapping inputs, do it */
		if (outreg0 == inreg1) {
			jive_input_swap(node->base.inputs[0], node->base.inputs[1]);
			return;
		}
		/* if swapping makes the first operand overwritable, do it */
		if (jive_resource_crosses(node->base.inputs[0]->resource, &node->base) &&
		   !jive_resource_crosses(node->base.inputs[1]->resource, &node->base))
			jive_input_swap(node->base.inputs[0], node->base.inputs[1]);

	}
	
	if (jive_resource_is_active_after(node->base.inputs[0]->resource, &node->base)) {
		/* input register #0 is used after this instruction, therefore
		must not overwrite it; move calculation into a temporary
		register, preferably using the final destination register
		outright */
		
		const jive_cpureg * reg = 0;
		if (jive_regcls_count_check_add(&node->base.use_count_before, outreg0->regcls) == 0)
			reg = outreg0;
		
		if (!reg) {
			size_t n;
			for(n=0; n<regcls->nregs; n++) {
				if (jive_regcls_count_check_add(&node->base.use_count_before, regcls->regs[n].regcls)) continue;
				if (jive_regcls_count_check_add(&node->base.use_count_after, regcls->regs[n].regcls)) continue;
				reg = &regcls->regs[n];
			}
		}
		
		pre_op_transfer(&node->base, reg);
		inreg0 = reg;
	}
	
	if (inreg0 == outreg0) return;
	
	/* if still not quite right, move result to destination
	register after the operation */
	post_op_transfer(&node->base, inreg0);
	outreg0 = inreg0;
}

void
jive_regalloc_fixup(jive_graph * graph)
{
	jive_traverser * traverser = jive_bottomup_traverser_create(graph);
	
	jive_node * node;
	while( (node = jive_traverser_next(traverser)) != 0) {
		process_node(node);
	}
	jive_traverser_destroy(traverser);
}

