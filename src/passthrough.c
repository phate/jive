#include <jive/internal/passthroughstr.h>
#include <jive/internal/graphstr.h>

const jive_node_passthrough_info *
jive_node_passthrough_info_get_ro(jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	return head->gates;
}

jive_node_passthrough_info *
jive_node_passthrough_info_get(jive_node * node)
{
	jive_node_head * head = head_of_node(node);
	if (!head->gates) {
		jive_node_passthrough_info * passthrough_info;
		passthrough_info = jive_malloc(node->graph, sizeof(*passthrough_info));
		passthrough_info->input.first = passthrough_info->input.last = 0;
		passthrough_info->output.first = passthrough_info->output.last = 0;
		head->gates = passthrough_info;
	}
	return head->gates;
}


jive_passthrough *
jive_passthrough_create(jive_graph * graph, unsigned int nbits, jive_cpureg_class_t regcls, const char * description)
{
	jive_passthrough_head * head;
	jive_passthrough * passthrough;
	head = jive_malloc(graph, sizeof(*head)+sizeof(*passthrough));
	passthrough = (jive_passthrough *) (head+1);
	head->inputs.first = head->inputs.last = 0;
	head->outputs.first = head->outputs.last = 0;
	
	passthrough->nbits = nbits;
	passthrough->regcls = regcls;
	
	if (description) passthrough->description = jive_strdup(graph, description);
	else passthrough->description = 0;
	
	return passthrough;
}

jive_value *
jive_output_passthrough_create(jive_node * node, jive_passthrough * passthrough)
{
	jive_node_passthrough_info * passthrough_info = jive_node_passthrough_info_get(node);
	jive_passthrough_head * g_head = head_of_passthrough(passthrough);
	jive_output_passthrough_head * o_head;
	jive_value_bits * o;
	
	o_head = jive_malloc(node->graph, sizeof(*o_head)+sizeof(*o));
	o = (jive_value_bits *) (o_head+1);
	
	/* link into list of outputs per node */
	o_head->node.prev = passthrough_info->output.last;
	o_head->node.next = 0;
	if (passthrough_info->output.last) passthrough_info->output.last->node.next = o_head;
	else passthrough_info->output.first = o_head;
	
	/* link into list of outputs per gate */
	o_head->gate.prev = g_head->outputs.last;
	o_head->gate.next = 0;
	if (g_head->outputs.last) g_head->outputs.last->node.next = o_head;
	else g_head->outputs.first = o_head;
	
	jive_value_bits_init(o, node, passthrough->nbits);
	// FIXME: set o->value_range
	jive_value_get_extra((jive_value *)o)->passthrough = passthrough;
	jive_value_set_cpureg_class((jive_value *)o, passthrough->regcls);
	
	return (jive_value *) o;
}

jive_operand_bits *
jive_input_passthrough_create(jive_node * node, jive_passthrough * passthrough, jive_value * value)
{
	jive_node_passthrough_info * passthrough_info = jive_node_passthrough_info_get(node);
	jive_passthrough_head * g_head = head_of_passthrough(passthrough);
	jive_input_passthrough_head * i_head;
	jive_operand_bits * i;
	
	i_head = jive_malloc(node->graph, sizeof(*i_head)+sizeof(*i));
	i = (jive_operand_bits *) (i_head+1);
	
	/* link into list of inputs per node */
	i_head->node.prev = passthrough_info->input.last;
	i_head->node.next = 0;
	if (passthrough_info->input.last) passthrough_info->input.last->node.next = i_head;
	else passthrough_info->input.first = i_head;
	
	/* link into list of inputs per gate */
	i_head->gate.prev = g_head->inputs.last;
	i_head->gate.next = 0;
	if (g_head->inputs.last) g_head->inputs.last->node.next = i_head;
	else g_head->inputs.first = i_head;
	
	jive_operand_bits_init(i, value, 0);
	jive_node_add_operand(node, (jive_operand *)i);
	jive_input_get_extra((jive_operand *)i)->passthrough = passthrough;
	jive_operand_set_cpureg_class((jive_operand *)i, passthrough->regcls);
	
	jive_operand_value_connect(
		value->node, (jive_value *)value,
		node, (jive_operand *)i);
	return i;
}
