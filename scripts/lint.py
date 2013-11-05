import annotator
import getopt
import os
import sys

options_seq, filenames = getopt.getopt(sys.argv[1:], "", ("all", "summary"))
options = {}
options.update(options_seq)

if "--all" in options:
	filenames = [f for f in os.popen("git ls-files").read().split("\n") if f]
elif not filenames:
	# by default, lint files touched by the current commit
	pending = [f for f in os.popen("git status --short").read().split("\n") if f]
	filenames = []
	for f in pending:
		if f[0:2] == "??": continue
		if f[0:2] == "D ": continue
		if f[0:2] == "R ": filenames.append(f[f.find("->")+3:])
		else: filenames.append(f[3:])
else:
	# otherwise just lint files specified on the command line
	pass

files = [
	annotator.AnnotatedFile(fname, [l[:-1] for l in file(fname).readlines()])
	for fname in filenames
]

a = annotator.AutoAnnotator()

for f in files:
	a.process(f)
	if "--summary" in options:
		if f.output(): print "Errors in", f.name
	else:
		for l in f.output(): print l
