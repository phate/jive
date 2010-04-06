#include <string.h>

#include <jive/nodeclass.h>
#include <jive/loadstore.h>
#include <jive/internal/loadstorestr.h>

const jive_node_class JIVE_MEMACCESS = {
	0, "MEMACCESS", sizeof(jive_memaccess_node), 0,
	
	.equiv = 0,
	.repr = 0
};

static void
jive_memread_invalidate(void * _node)
{
	jive_value_bits * port = &((jive_memread_node *)_node)->output;
	jive_output_edge_iterator i;
	/* if already invalidated, terminate */
	if (!port->_value_range.uptodate) return;
	port->_value_range.uptodate = false;
	JIVE_ITERATE_OUTPUTS(i, (jive_node *)_node)
		if (i->origin.port == (jive_value *)port) jive_node_invalidate(i->target.node);
}

static void
jive_memread_revalidate(void * _node)
{
	jive_value_bits * port = &((jive_memread_node *)_node)->output;
	jive_bitstring_value_range * output = &port->_value_range;
	
	memset(output->bits, 'D', output->nbits);
	jive_bitstring_value_range_numeric(output);
	output->uptodate = true;
}

const jive_node_class JIVE_MEMREAD = {
	&JIVE_MEMACCESS, "MEMREAD", sizeof(jive_memread_node), 0,
	
	.equiv = 0,
	.repr = 0,
	.invalidate_inputs = &jive_memread_invalidate,
	.revalidate_outputs = &jive_memread_revalidate
};

jive_node *
jive_memread_rawcreate(jive_value_bits * input, size_t size)
{
	/* FIXME: to be removed */
	jive_node * _node = jive_node_alloc(input->node->graph, &JIVE_MEMREAD, 1, (jive_value **)&input);
	jive_memread_node * node = (jive_memread_node *) _node;
	jive_value_bits_init(&node->output, _node, size*8);
	
	return _node;
}

jive_value_bits *
jive_memread(jive_value_bits * input, size_t size)
{
	jive_node * node = jive_memread_rawcreate(input, size);
	return &((jive_memread_node *)node)->output;
}

const jive_node_class JIVE_MEMWRITE = {
	&JIVE_MEMACCESS, "MEMWRITE", sizeof(jive_memaccess_node), 0,
	
	.equiv = 0,
	.repr = 0
};

jive_node *
jive_memwrite(jive_value_bits * address, jive_value_bits * data)
{
	jive_value * inputs[] = {(jive_value *)address, (jive_value *)data};
	/* FIXME: to be removed */
	return jive_node_alloc(address->node->graph, &JIVE_MEMWRITE, 2, inputs);
}

/* machine-word memory access functions */

const jive_node_class JIVE_LOAD = {
	&JIVE_MEMACCESS, "LOAD", sizeof(jive_load_node), 0,
	
	.equiv = 0,
	.repr = 0,
	.invalidate_inputs = &jive_memread_invalidate,
	.revalidate_outputs = &jive_memread_revalidate
};

jive_node *
jive_load_rawcreate(jive_value_bits * address, size_t size, size_t ext_size, bool sign_extend)
{
	/* FIXME: to be removed */
	jive_node * _node = jive_node_alloc(address->node->graph, &JIVE_LOAD, 1, (jive_value **)&address);
	jive_load_node * node = (jive_load_node *) _node;
	node->sign_extend = sign_extend;
	jive_value_bits_init(&node->output, _node, size*8);
	
	return _node;
}

jive_value_bits *
jive_load(jive_value_bits * address, size_t size, size_t ext_size, bool sign_extend)
{
	jive_load_node * node = (jive_load_node *) jive_load_rawcreate(address, size, ext_size, sign_extend);
	return &node->output;
}

const jive_node_class JIVE_STORE = {
	&JIVE_MEMACCESS, "STORE", sizeof(jive_memaccess_node), 0,
	
	.equiv = 0,
	.repr = 0
};

jive_node *
jive_store_rawcreate(jive_value_bits * address, jive_value_bits * data)
{
	jive_value * inputs[] = {(jive_value *)address, (jive_value *)&data};
	/* FIXME: to be removed */
	return jive_node_alloc(address->node->graph, &JIVE_STORE, 2, inputs);
}

