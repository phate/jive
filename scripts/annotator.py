#!/usr/bin/env python

import itertools
import fnmatch
import os
import re

class AnnotatedFile(object):
	__slots__ = (
		"_name",
		"_lines",
		"_annotations",
	)
	
	def __init__(self, name, lines, annotations = {}):
		self._name = name
		self._lines = lines
		self._annotations = annotations.copy()
	
	def annotate(self, row, col, msg):
		msgs = self._annotations.get((row, col), [])
		msgs.append(msg)
		self._annotations[(row, col)] = msgs
	
	def output(self):
		sorted_annotations = sorted(self._annotations.items())
		
		return [
			"%s:%d:%d: %s" % (
				self._name, row + 1, col + 1, ", ".join(msg)
			)
			for ((row,col), msg) in sorted_annotations
		]
	
	lines = property(lambda self: self._lines)
	name = property(lambda self: self._name)

class Annotator(object):
	__slots__ = ()
	def process(self, annotated_file): abstract

class CompoundAnnotator(Annotator):
	"""Apply multiple annotators in sequence to a file."""
	__slots__ = ("parts",)
	PartClasses = ()
	def __init__(self):
		self.parts = tuple(C() for C in self.PartClasses)
	def process(self, annotated_file):
		for p in self.parts:
			p.process(annotated_file)

class PatternedFileAnnotator(Annotator):
	__slots__ = (
		"_handler_list",
	)
	
	file_pattern_list = ()
	
	def __init__(self):
		Annotator.__init__(self)
		handler_list = []
		for pat, factory in self.file_pattern_list:
			handler_list.append((pat, factory()))
		self._handler_list = tuple(handler_list)
	def process(self, annotated_file):
		name = os.path.basename(annotated_file.name)
		for pattern, annotator in self._handler_list:
			if fnmatch.fnmatch(name, pattern):
				annotator.process(annotated_file)

class WhitespaceAnnotator(Annotator):
	__slots__ = ()
	
	def process(self, annotated_file):
		for row, line in itertools.izip(itertools.count(), annotated_file.lines):
			self.process_line(row, line, annotated_file)
	
	def process_line(self, row, line, annotated_file):
		if not line.strip():
			return
		tmp = line.rstrip()
		if tmp != line:
			annotated_file.annotate(row, len(tmp), "trailing whitespace")

class WhitespaceAnnotator(Annotator):
	__slots__ = ()
	
	def process(self, annotated_file):
		for row, line in itertools.izip(itertools.count(), annotated_file.lines):
			self.process_line(row, line, annotated_file)
	
	def process_line(self, row, line, annotated_file):
		if not line.strip():
			return
		tmp = line.rstrip()
		if tmp != line:
			annotated_file.annotate(row, len(tmp), "trailing whitespace")

class CopyrightAnnotator(Annotator):
	__slots__ = (
		"_copyright_re",
		"_log_re",
	)
	
	def __init__(self):
		self._copyright_re = re.compile("^.*Copyright ([0-9 ]+) (.*)$")
		self._log_re = re.compile("^([a-f0-9]+) ([^<]* <[^>]*>) (....).*$")
	
	def get_log_copyrights(self, annotated_file):
		logs = os.popen("git log --date=iso --pretty=format:\"%%H %%aN <%%aE> %%ad\" %s" % annotated_file.name)
		
		holders = {}
		
		for line in logs.readlines():
			m = self._log_re.match(line.strip())
			if m:
				commit_id, author, year = m.groups()
				if commit_id == "e0508430eb865d50e70f1562a24c42ecad2a33c5":
					# this commit mechanically touched every file
					continue
				s = holders.get(author, set())
				s.add(int(year))
				holders[author] = s
		return holders
	
	def process(self, annotated_file):
		holders = {}
		for line in annotated_file.lines:
			m = self._copyright_re.match(line)
			if not m: continue
			years = set(m.groups()[0].split(" "))
			holders[m.groups()[1]] = tuple(int(x) for x in years)
		holders_expect = self.get_log_copyrights(annotated_file)
		for holder, expect_years in holders_expect.items():
			years = holders.get(holder, set())
			d = expect_years.difference(years)
			for year in d:
				annotated_file.annotate(0, 0,
					"missing copyright: %s %d" % (holder, year))

class SortedLinesAnnotator(Annotator):
	__slots__ = ("_regex",)
	pattern = ""
	not_sorted_msg = ""
	
	def __init__(self):
		self._regex = re.compile(self.pattern)
	
	def process(self, annotated_file):
		prev = None
		for row, line in itertools.izip(itertools.count(), annotated_file.lines):
			m = self._regex.match(line)
			if m:
				current = m.groups()[0]
				if prev is not None and prev > current:
					annotated_file.annotate(row, 0, self.not_sorted_msg)
				prev = current
			else:
				prev = None

class IncludeGuardAnnotator(Annotator):
	__slots__ = ()
	
	def process(self, annotated_file):
		name = annotated_file.name.\
			replace("-", "_").\
			replace(".", "_").\
			replace("/", "_").\
			upper()
		if not name.startswith("INCLUDE_"):
			return
		name = name[8:]
		i = iter(itertools.izip(itertools.count(), annotated_file.lines))
		
		# find first ifdef
		while True:
			try:
				row, line = i.next()
			except StopIteration:
				annotated_file.annotate(0, 0, "missing ifndef of include guard: %s" % name)
				break
			if line.startswith("#ifndef "):
				found_name = line.strip()[8:]
				if found_name != name:
					annotated_file.annotate(row, 8, "wrong include guard, expected: %s" % name)
				break
		# find first define
		while True:
			try:
				row, line = i.next()
			except StopIteration:
				annotated_file.annotate(0, 0, "missing define of include guard: %s" % name)
				break
			if line.startswith("#define "):
				found_name = line.strip()[8:]
				if found_name != name:
					annotated_file.annotate(row, 8, "wrong include guard, expected: %s" % name)
				break
		# find terminating endif
		while True:
			try:
				row, line = i.next()
			except StopIteration:
				annotated_file.annotate(0, 0, "missing endif for include guard: %s" % name)
				break
			if line.startswith("#endif"):
				break

class LineLengthAnnotator(Annotator):
	__slots__ = ()
	def process(self, annotated_file):
		prev = None
		for row, line in itertools.izip(itertools.count(), annotated_file.lines):
			if len(line) > 100:
				annotated_file.annotate(row, 100, "line exceeds 100 characters")

class SortedIncludesAnnotator(SortedLinesAnnotator):
	__slots__ = ()
	pattern = "^#include <([^>]*)>$"
	not_sorted_msg = "include line block not sorted lexicographically"

class SortedStructDeclAnnotator(SortedLinesAnnotator):
	__slots__ = ()
	pattern = "^struct ([a-zA-Z0-9_]+);$"
	not_sorted_msg = "struct declaration line block not sorted lexicographically"

class SortedTypedefAnnotator(SortedLinesAnnotator):
	__slots__ = ()
	pattern = "^typedef struct [a-zA-Z_0-9]+ ([a-zA-Z0-9_]+);$"
	not_sorted_msg = "typedef line block not sorted lexicographically"

class SortedTargetsAnnotator(SortedLinesAnnotator):
	__slots__ = ()
	pattern = "^[\t ]*([^ ]*) *\\\\$"
	not_sorted_msg = "build targets not sorted lexicographically"

class CAnnotator(CompoundAnnotator):
	__slots__ = ()
	PartClasses = (
		WhitespaceAnnotator,
		LineLengthAnnotator,
		SortedIncludesAnnotator,
		SortedStructDeclAnnotator,
		SortedTypedefAnnotator,
		CopyrightAnnotator,
	)

class CHeaderAnnotator(CompoundAnnotator):
	__slots__ = ()
	PartClasses = (
		IncludeGuardAnnotator,
		CAnnotator,
	)

class CSourceAnnotator(CompoundAnnotator):
	__slots__ = ()
	PartClasses = (
		CAnnotator,
	)

class MakefileAnnotator(CompoundAnnotator):
	__slots__ = ()
	PartClasses = (SortedTargetsAnnotator,)

class AutoAnnotator(PatternedFileAnnotator):
	__slots__ = ()
	
	file_pattern_list = (
		("*.c", CSourceAnnotator),
		("*.h", CHeaderAnnotator),
		("Makefile", MakefileAnnotator),
		("Makefile.sub", MakefileAnnotator),
	)
