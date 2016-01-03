class VMWriter():
	def __init__(self, out_file):
		self._output = open(out_file, 'w')

	def __del__(self):
		self._output.close()

	def writePush(self, segment, index):
		self._output.write("push %s %d\n" % (segment.lower(), index))

	def writePop(self, segment, index):
		self._output.write("pop %s %d\n" % (segment.lower(), index))

	def writeArithmetic(self, command):
		if command == '+':
			self._output.write("add")
		elif command == '-':
			self._output.write("sub")
		elif command == '*':
			self._output.write("call Math.multiply 2")
		elif command == '/':
			self._output.write("call Math.divide 2")
		elif command == '&':
			self._output.write("and")
		elif command == '|':
			self._output.write("or")
		elif command == '<':
			self._output.write("lt")
		elif command == '>':
			self._output.write("gt")
		elif command == '=':
			self._output.write("eq")
		elif command == '~':
			self._output.write("not")
		elif command == 'neg': # Unary
			self._output.write("neg")
		self._output.write('\n')

	def writeLabel(self, label):
		self._output.write("label %s\n" % label)

	def writeGoto(self, label):
		self._output.write("goto %s\n" % label)

	def writeIf(self, label):
		self._output.write("if-goto %s\n" % label)

	def writeCall(self, name, nArgs):
		self._output.write("call %s %d\n" % (name, nArgs))

	def writeFunction(self, name, nLocals):
		self._output.write("function %s %d\n" % (name, nLocals))

	def writeReturn(self):
		self._output.write("return\n")

	def writeAlloc(self, size):
		self.writePush("CONSTANT", size)
		self._output.write("call Memory.alloc 1\n")

	def writeString(self, string):
		self.writePush("CONSTANT", len(string))
		self.writeCall("String.new", 1)
		for char in string:
			unicode_rep = ord(char)
			self.writePush("CONSTANT", unicode_rep)
			self.writeCall("String.appendChar", 2)