import re
import sys

r = re.compile('([a-zA-Z0-9_*_ ]*\\[\\]) *')
r = re.compile('\\(([a-zA-Z0-9_*_ ]*)\\[\\]\\)')

def scan_stmt_starts(text):
	starts = []
	next_line_stmt = False
	beginning_decl = False
	nested_decls = set()
	depth = 0
	for n in range(len(text)):
		c = text[n]
		if c == '{':
			if beginning_decl:
				nested_decls.add(depth)
			depth += 1
		elif c == '}':
			depth -= 1
			if depth in nested_decls:
				nested_decls.remove(depth)
		elif c == '=':
			beginning_decl = True
		if c not in '= \t\n':
			beginning_decl = False
		
		if c == ';' or c == '{' or c == '}':
			next_line_stmt = not bool(nested_decls)
		elif c == '\n' and next_line_stmt:
			starts.append((n + 1, get_indent(text, n + 1)))
			next_line_stmt = False
		else:
			next_line_stmt = False
	assert depth == 0
	return starts

def get_indent(text, pos):
	indent = ''
	while pos < len(text) and text[pos] == '\t':
		indent += text[pos]
		pos += 1
	return indent

#for start, indent in scan_stmt_starts(text):
	#print '---'
	#print indent, start
	#print text[start:text.find('\n', start)]

def find_stmt_start(text, pos):
	last_start, last_indent = 0, ''
	for start, indent in scan_stmt_starts(text):
		if start > pos:
			return last_start, last_indent
		last_start = start
		last_indent = indent
	return last_start, last_indent

def find_closing_brace(text, pos):
	depth = 1
	while depth > 0 and pos < len(text):
		if text[pos] in '([{':
			depth += 1
		elif text[pos] in ')]}':
			depth -= 1
		pos += 1
	return pos

def is_macro_def(text, pos):
	while pos > 0:
		if text[pos] == '#':
			return True
		elif text[pos] == '\n':
			return False
		else:
			pos -= 1
	return False

def convert_single(text, counter):
	for m in r.finditer(text):
		start, end = m.start(), m.end()
		
		if is_macro_def(text, start):
			continue
		
		arraytype = m.group(1)
		
		stmt_start, stmt_indent = find_stmt_start(text, start)
		
		values_start = text.find('{', end)
		values_end = find_closing_brace(text, values_start + 1)
		
		values = text[values_start:values_end]
		
		before_stmt = text[:stmt_start]
		
		var = 'tmparray%d' % counter
		
		inserted_tmp = stmt_indent + arraytype + ' ' + var +'[] = ' + values + ';\n';
		
		remainder = text[stmt_start:start] + var + text[values_end:]
		
		return text[:stmt_start] + inserted_tmp + remainder
	
	return None

def convert_all(text):
	counter = 0
	while True:
		new_text = convert_single(text, counter)
		if not new_text: return text
		text = new_text
		counter += 1

filename = sys.argv[1]
text = file(filename).read()

new_text = convert_all(text)

if new_text != text:
	sys.stderr.write(filename + '\n')
	file(filename, 'w').write(new_text)
