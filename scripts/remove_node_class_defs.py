import os
import re
import sys

r_node = re.compile('''(.*)
const\s+(jive_node_class)\s+([A-Z_]+)\s*=\s*{
\s*parent\s*:\s*&([A-Z_]+)\s*,
\s*name\s*:\s*"([A-Z_]+)"\s*,
\s*fini\s*:\s*([a-z_]+),\s*[a-z\s*/]*
\s*get_default_normal_form\s*:\s*([a-z_]+),\s*[a-z\s*/]*
\s*get_label\s*:\s*([a-z_]+),\s*[a-z\s*/]*
\s*match_attrs\s*:\s*([a-z_]+),\s*[a-z\s*/]*
\s*check_operands\s*:\s*([a-z_]+),\s*[a-z\s*/]*
\s*create\s*:\s*([a-z_]+),?[a-z\s*/]*
};
(.*)''', re.DOTALL)

def remove_node_defs(text):
	m = r_node.match(text)
	if not m: return text
	else: return  remove_node_defs(m.groups()[0]) +  remove_node_defs(m.groups()[-1])

files = [f for f in os.popen('git ls-files').read().split('\n') if f]
#files = files[:320]
#files = ['src/vsdg/anchor.c']
classes = {}
methods = {}

for filename in files:
	#sys.stderr.write(filename + '\n')
	text = file(filename).read()
	new_text = remove_node_defs(text)
	if new_text != text:
		file(filename, 'w').write(new_text)
