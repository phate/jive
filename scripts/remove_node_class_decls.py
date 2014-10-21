import os
import re
import sys

r_node = re.compile('''(.*)
extern\s+const(\s+struct)?\s+(jive_node_class)\s+([A-Z_]+);
?(.*)''', re.DOTALL)

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
