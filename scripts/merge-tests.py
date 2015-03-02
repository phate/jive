import sys

c_includes = set()
cxx_includes = set()
jive_includes = set()
local_includes = set()

code_blocks = []

def mangle(fname):
	name = fname[6:-2]
	name = name.replace('/', '_')
	name = name.replace('-', '_')
	return name

for fname in sys.argv[1:]:
	seen_includes = False
	code_lines = []
	name = mangle(fname)
	for line in file(fname).readlines():
		line = line[:-1]
		if line[:9] == "#include ":
			include = line[9:]
			if include[:6] == "<jive/":
				jive_includes.add(include)
			elif include[-3:] == ".h>":
				c_includes.add(include)
			elif include[0] == '"':
				local_includes.add(include)
			else:
				cxx_includes.add(include)
			seen_includes = True
			continue
		if not seen_includes: continue
		line = line + '\n'
		if line == '\n' and code_lines and code_lines[-1] == '\n':
			continue
		line = line.replace('test_main', name)
		code_lines.append(line)
	code_blocks.append(''.join(code_lines))

out = sys.stdout

if local_includes:
	for i in sorted(local_includes): out.write('#include %s\n' % i)
	out.write('\n')

if c_includes:
	for i in sorted(c_includes): out.write('#include %s\n' % i)
	out.write('\n')

if cxx_includes:
	for i in sorted(cxx_includes): out.write('#include %s\n' % i)
	out.write('\n')

if jive_includes:
	for i in sorted(jive_includes): out.write('#include %s\n' % i)
	out.write('\n')

for c in code_blocks: out.write(c)
