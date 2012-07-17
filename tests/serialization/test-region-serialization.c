#include <stdio.h>
#include <locale.h>

#include <jive/view.h>
#include <jive/vsdg.h>
#include <jive/vsdg/equivalence.h>
#include <jive/types/bitstring.h>
#include <jive/serialization/driver.h>
#include <jive/serialization/token-stream.h>
#include <jive/util/buffer.h>

static void
my_error(jive_serialization_driver * drv, const char msg[])
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

int main()
{
	setlocale(LC_ALL, "");
	jive_context * ctx = jive_context_create();
	
	jive_buffer buf;
	jive_buffer_init(&buf, ctx);
	
	jive_graph * gr1 = jive_graph_create(ctx);
	jive_node_normal_form_set_mutable(
		jive_graph_get_nodeclass_form(gr1, &JIVE_NODE),
		false);
	
	jive_output * a = jive_bitconstant(gr1, 8, "01010101");
	jive_output * b = jive_bitconstant(gr1, 8, "10101010");
	jive_output * true_out = jive_control_true(gr1);
	jive_output * c;
	jive_gamma(true_out, 1, (const jive_type*[]){jive_output_get_type(a)}, &a, &b, &c);
	
	jive_node * theta = jive_theta_create(gr1->root_region,
		1, (const jive_type *[]){jive_output_get_type(c)}, &c);
	jive_output * d = theta->outputs[0];
	
	jive_output * e = jive_bitnot(d);
	
	jive_node * orig_node = e->node;
	
	jive_serialization_driver drv;
	jive_serialization_driver_init(&drv, ctx);
	jive_serialization_symtab_insert_nodesym(&drv.symtab,
		orig_node, jive_serialization_symtab_strdup(&drv.symtab, "TARGET"));
	jive_token_ostream * os = jive_token_ostream_simple_create(&buf);
	jive_serialize_graph(&drv, gr1, os);
	jive_token_ostream_destroy(os);
	jive_serialization_driver_fini(&drv);
	fwrite(buf.data, 1, buf.size, stderr);
	
	jive_view(gr1, stdout);
	jive_graph * gr2 = jive_graph_create(ctx);
	jive_token_istream * is = jive_token_istream_simple_create(
		ctx, (char *)buf.data, buf.size + (char *) buf.data);
	jive_serialization_driver_init(&drv, ctx);
	drv.error = my_error;
	jive_deserialize_graph(&drv, is, gr2);
	jive_node * repl_node = jive_serialization_symtab_name_to_node(&drv.symtab, "TARGET")->node;
	assert(repl_node);
	jive_serialization_driver_fini(&drv);
	jive_token_istream_destroy(is);
	
	jive_view(gr2, stdout);
	
	jive_view(gr2, stdout);
	
	assert (jive_graphs_equivalent(gr1, gr2,
		1, &orig_node, &repl_node, 0, NULL, NULL));
	
	jive_graph_destroy(gr1);
	jive_graph_destroy(gr2);
	
	jive_buffer_fini(&buf);
	
	assert(jive_context_is_empty(ctx));
	jive_context_destroy(ctx);
	
	return 0;
}