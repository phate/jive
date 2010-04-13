#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jive/graphdebug.h>
#include <jive/nodeclass.h>
#include <jive/passthrough.h>
#include <jive/machine.h>

static const char dotfile_header[] =
	"digraph {\n"
	"graph [size=\"7.5,10\" ];\n"
	"edge [ arrowsize=\"0.75\", labeldistance=\"2\", labelfloat=true ];\n";

static const char node_template[] =
	"n%lx [ shape=\"record\", label=\"{{%s}|<body%ld>%p\\n%s|{%s}}\" ];\n";

static const char edge_template[] =
	"n%lx:p%lx -> n%lx:p%lx [ style=solid ];\n";

static const char stateedge_template[] =
	"n%lx -> n%lx [ style=dashed ] ;\n";

static const char dotfile_footer[] =
	"}\n";

static void
jive_node_dump(jive_node * node, int fd)
{
	char operands[1024] = "";
	char values[1024] = "";
	char tmp[1024];
	int count = 0;
	
	jive_operand * operand = jive_node_iterate_operands(node);
	while(operand) {
		if (operands[0]) strncat(operands, "|", sizeof(operands));
		
		jive_passthrough * pt = jive_operand_get_passthrough(operand);
		char * oprepr = operand->type->repr(operand);
		if (!pt)
			snprintf(tmp, sizeof(tmp), "<p%lx> %s", (long)operand, oprepr);
		else {
			if (pt->description)
				snprintf(tmp, sizeof(tmp), "<p%lx> %s:%s", (long)operand, pt->description, oprepr);
			else
				snprintf(tmp, sizeof(tmp), "<p%lx> %lx:%s", (long)operand, (long)pt, oprepr);
		}
		free(oprepr);
		
		strncat(operands, tmp, sizeof(operands));
		
		jive_cpureg_t reg = jive_value_get_cpureg(operand->value);
		if (reg) {strncat(operands, ":", sizeof(operands)); strncat(operands, reg->name, sizeof(operands));}
		operand = operand->next;
	}
	
	jive_value * value = jive_node_iterate_values(node);
	while(value) {
		if (values[0]) strncat(values, "|", sizeof(values));
		
		jive_passthrough * pt = jive_value_get_passthrough(value);
		char * valuerepr = value->type->repr(value);
		
		if (!pt)
			snprintf(tmp, sizeof(tmp), "<p%lx> %s", (long)value, valuerepr);
		else {
			if (pt->description)
				snprintf(tmp, sizeof(tmp), "<p%lx> %s:%s", (long)value, pt->description, valuerepr);
			else
				snprintf(tmp, sizeof(tmp), "<p%lx> %lx:%s", (long)value, (long)pt, valuerepr);
		}
		free(valuerepr);
		
		strncat(values, tmp, sizeof(values));
		
		jive_cpureg_t reg = jive_value_get_cpureg(value);
		if (reg) {strncat(values, ":", sizeof(values)); strncat(values, reg->name, sizeof(values));}
		
		value = value->next;
	}
	
	if (!node->type->repr) {
		count = snprintf(tmp, sizeof(tmp), node_template,
			(long)node,
			operands,
			(long)node, node, node->type->name,
			values);
	} else {
		char * content = node->type->repr(node);
		count = snprintf(tmp, sizeof(tmp), node_template,
			(long)node,
			operands,
			(long)node, node, content,
			values);
		free(content);
	}
	
	write(fd, tmp, count);
}

static void
jive_edge_dump(jive_edge * edge, int fd)
{
	char tmp[512];
	int count = 0;
	
	if (edge->origin.port) {
		count = snprintf(tmp, sizeof(tmp), edge_template,
			(long)edge->origin.node, (long)edge->origin.port,
			(long)edge->target.node, (long)edge->target.port
			);
	} else {
		count = snprintf(tmp, sizeof(tmp), stateedge_template,
			(long)edge->origin.node, (long)edge->target.node);
	}
	
	write(fd, tmp, count);
}

static void
jive_graph_dump_noheader(jive_graph * graph, int fd)
{
	jive_graph_traverser * trav = jive_graph_traverse_topdown(graph);
	
	jive_node * node = jive_graph_traverse_next(trav);
	while(node) {
		jive_node_dump(node, fd);
		
		jive_input_edge_iterator i;
		JIVE_ITERATE_INPUTS(i, node)
			jive_edge_dump(i, fd);
		
		node = jive_graph_traverse_next(trav);
	}
	
	jive_graph_traverse_finish(trav);
}

void
jive_graph_dump(jive_graph * graph, int fd)
{
	write(fd, dotfile_header, sizeof(dotfile_header) - 1);
	
	jive_graph_dump_noheader(graph, fd);
	
	write(fd, dotfile_footer, sizeof(dotfile_footer) - 1);
}

void
jive_graph_view(jive_graph * graph)
{
	int fd = open("/tmp/tmp.dot", O_CREAT|O_RDWR|O_TRUNC, 0644);
	jive_graph_dump(graph, fd);
	close(fd);
	
	system("dot -Tps </tmp/tmp.dot >/tmp/tmp.ps");
	system("gv /tmp/tmp.ps >/dev/null 2>&1");
}

#include <jive/regalloc/assign.h>
#include <jive/instruction.h>
#include <jive/internal/instruction_str.h>

static const char reg_candidate_template[] =
	"n%lx [ shape=\"oval\", label=\"%p/%p\\n%s:%s\" ];\n";

static const char interference_edge_template[] =
	"n%lx -> n%lx [ style=dashed, dir=both ];\n";

static const char instruction_template[] =
	"n%lx [ shape=\"box\", label=\"%p\" ];\n";

static const char instruction_edge_template[] =
	"n%lx -> n%lx [ style=dashed, dir=forward ] ;\n";

static const char from_reg_cand_edge[] =
	"n%lx -> n%lx:p%lx [ style=solid ] ;\n";

static const char to_reg_cand_edge[] =
	"n%lx:p%lx -> n%lx [ style=solid ] ;\n";

static const char instr_cluster[] = "subgraph cluster_instr { rank=same; \n";

static void
jive_reg_candidate_dump(jive_reg_candidate * cand, int fd)
{
	char tmp[80];
	const char * regcls_name = cand->regcls->name;
	const char * reg_name = cand->reg->name;
	size_t count = snprintf(tmp, sizeof(tmp), reg_candidate_template, (long) cand, cand, cand->value, regcls_name, reg_name);
	write(fd, tmp, count);
	
	size_t n;
	for(n=0; n<cand->interference.nitems; n++) {
		jive_reg_candidate * other = cand->interference.items[n].value;
		if (other > cand) continue;
		count = snprintf(tmp, sizeof(tmp), interference_edge_template, (long)cand, (long)other);
		write(fd, tmp, count);
	}
}

static void
jive_instr_dump(jive_interference_graph * igraph, jive_instruction * instr, int fd)
{
	char tmp[160];
	jive_node_dump((jive_node *) instr, fd);
	if (instr->prev) {
		size_t count = snprintf(tmp, sizeof(tmp), instruction_edge_template, (long)instr->prev, (long)instr);
		write(fd, tmp, count);
	}
	
	jive_input_edge_iterator i;
	JIVE_ITERATE_INPUTS(i, (jive_node *) instr) {
		if (!i->origin.port) continue;
		
		jive_reg_candidate * cand = jive_interference_graph_map_value(igraph, i->origin.port);
		size_t count = snprintf(tmp, sizeof(tmp), from_reg_cand_edge, (long)cand, (long)i->target.node, (long)i->target.port);
		write(fd, tmp, count);
	}
	
	jive_value * value;
	for(value = jive_node_iterate_values((jive_node *)instr); value; value = value->next) {
		jive_reg_candidate * cand = jive_interference_graph_map_value(igraph, value);
		size_t count = snprintf(tmp, sizeof(tmp), to_reg_cand_edge, (long)instr, (long)value, (long)cand);
		write(fd, tmp, count);
	}
}

void
jive_igraph_dump(jive_interference_graph * igraph, int fd)
{
	write(fd, dotfile_header, sizeof(dotfile_header)-1);
	size_t n;
	for(n=0; n<igraph->map.nitems; n++) {
		jive_reg_candidate * cand = igraph->map.items[n];
		jive_reg_candidate_dump(cand, fd);
	}
	jive_instruction * instr = igraph->seq.first;
	while(instr) {
		jive_instr_dump(igraph, instr, fd);
		instr = instr->next;
	}
	write(fd, instr_cluster, sizeof(instr_cluster)-1);
	instr = igraph->seq.first;
	while(instr) {
		char tmp[80];
		ssize_t count = snprintf(tmp, sizeof(tmp), "\tn%lx\n",
			(long)instr);
		write(fd, tmp, count);
		instr = instr->next;
	}
	write(fd, "}\n", 2);
	write(fd, dotfile_footer, sizeof(dotfile_footer)-1);
}

void
jive_igraph_view(jive_interference_graph * igraph)
{
	int fd = open("/tmp/itmp.dot", O_CREAT|O_RDWR|O_TRUNC, 0644);
	jive_igraph_dump(igraph, fd);
	close(fd);
	
	system("dot -Tps </tmp/itmp.dot >/tmp/itmp.ps");
	system("gv /tmp/itmp.ps >/dev/null 2>&1");
}

#include <jive/regalloc/cut.h>

static const char cut_cluster[] = "subgraph cluster%lx{rank=same;\n";

void
jive_graphcut_dump(jive_graphcut * cut, int fd)
{
	write(fd, dotfile_header, sizeof(dotfile_header) - 1);
	
	jive_graph_dump_noheader(cut->graph, fd);
	
	while(cut) {
		char tmp[80];
		ssize_t count;
		
		count = snprintf(tmp, sizeof(tmp), cut_cluster, (long)cut);
		write(fd, tmp, count);
		jive_instruction * instr = cut->first;
		while(instr) {
			count = snprintf(tmp, sizeof(tmp), "\tn%lx\n",
				(long)instr);
			write(fd, tmp, count);
			if (instr == cut->last) instr = 0;
			else instr = instr->next;
		}
		write(fd, "}\n", 2);
		cut = cut->lower;
	}
	
	write(fd, dotfile_footer, sizeof(dotfile_footer) - 1);
}

void
jive_graphcut_view(jive_graphcut * cut)
{
	int fd = open("/tmp/ctmp.dot", O_CREAT|O_RDWR|O_TRUNC, 0644);
	jive_graphcut_dump(cut, fd);
	close(fd);
	
	system("dot -Tps </tmp/ctmp.dot >/tmp/ctmp.ps");
	system("gv /tmp/ctmp.ps >/dev/null 2>&1");
}
