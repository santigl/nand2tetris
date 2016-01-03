import JackTokenizer

class JackCompilationEngineXML():
	def __init__(self, inFilename, outFilename):
		self.tokenizer = JackTokenizer.JackTokenizer(inFilename)
		self.output = open(outFilename, 'w')
		self.indentLevel = 0
		self.compileClass()
		self.output.close()

	def compileClass(self):
		self._startSection("class")

		self._writeKeyword()    # "Class"
		self._writeIdentifier() #  className
		self._writeSymbol()     # '{'

		# Variable declarations:
		if self.tokenizer.keyWord() in ["STATIC", "FIELD"]:
			self.compileClassVarDec()

		# Class' subroutines declarations:
		while (self.tokenizer.keyWord() in ["CONSTRUCTOR", "FUNCTION",
											"METHOD", "VOID"]):
			self.compileSubroutine()

		self._writeSymbol()     # '}'
		self._endSection("class")   

	def compileClassVarDec(self):
		# While there are lines declaring variables... (There could be 0.)
		while (self._tokenMatchesKeyword("STATIC") or
			   self._tokenMatchesKeyword("FIELD")):
			self._startSection("classVarDec")
			self._writeKeyword()        # "static"/"field"
			self._writeType()           # Var. type
			self._writeIdentifier()     # Var. name

			# Are there more variables in the same line?
			while self.tokenizer.symbol() == ',':
				self._writeSymbol()
				self._writeIdentifier() # Var. name

			self._writeSymbol()         # ';' symbol at the end of the line.
			self._endSection("classVarDec")


	def compileSubroutine(self):
		self._startSection("subroutineDec")
		self._writeKeyword()        # Constructor/Function/Method

		self._writeType()           # Return type

		self._writeIdentifier()     # Subroutine name
		self.compileParameterList() # Parameters (may be empty)

		# Body:
		self._startSection("subroutineBody")
		self._writeSymbol()         # '{'

		if self.tokenizer.keyWord() == "VAR":
			self.compileVarDec() 

		if not self._tokenMatchesSymbol('}'):
			self.compileStatements()
		self._writeSymbol()         # '}' (end of subroutine body.)
		self._endSection("subroutineBody")

		self._endSection("subroutineDec")


	def compileParameterList(self):
		self._writeSymbol()                 # '('
		self._startSection("parameterList")

		while self.tokenizer.symbol() != ')':
			self._writeKeyword()            # Type
			self._writeIdentifier()         # Name
			# More parameters?
			if self.tokenizer.symbol() == ',':
				self._writeSymbol()

		self._endSection("parameterList")   # ')'
		self._writeSymbol()


	def compileVarDec(self):
		# While there are lines declaring variables... (There could be 0.)
		while self._tokenMatchesKeyword("VAR"):
			self._startSection("varDec")
			self._writeKeyword()    # "Var"
			self._writeType()       # Var. type
			self._writeIdentifier() # Var name

			# Are there more variables in the same line?
			while self.tokenizer.symbol() == ',':
				self._writeSymbol()
				self._writeIdentifier() # Variable name.

			self._writeSymbol() # ';' symbol at the end of the line.
			self._endSection("varDec")
		

	def compileStatements(self):
		self._startSection("statements")

		while not self._tokenMatchesSymbol('}'):
			if self.tokenizer.keyWord() == "LET":
				self.compileLet()
		
			elif self.tokenizer.keyWord() == "RETURN":
				self.compileReturn()

			elif self.tokenizer.keyWord() == "DO":
				self.compileDo()

			elif self.tokenizer.keyWord() == "IF":
				self.compileIf()

			elif self.tokenizer.keyWord() == "WHILE":
				self.compileWhile()

		self._endSection("statements")


	def compileDo(self):
		self._startSection("doStatement")
		self._incIndentLevel()

		self._writeKeyword()        # "Do"
		self._writeIdentifier()     # Subroutine name/(class/var):

		if self._tokenMatchesSymbol('.'):
			self._writeSymbol()     # '.'
			self._writeIdentifier() # Method name

		self.compileExpressionList()

		self._writeSymbol()         # ';'

		self._decIndentLevel()
		self._endSection("doStatement")


	def compileLet(self):
		self._startSection("letStatement")

		# "Let":
		self._writeKeyword()
		# Variable name:
		self._writeIdentifier()

		if self.tokenizer.symbol() == '[':
			self._writeSymbol()         #'['
			self.compileExpression()    # expr
			self._writeSymbol()         #']'

		self._writeSymbol()             # '='
		self.compileExpression()

		self._writeSymbol() #';'
		self._endSection("letStatement")

	def compileWhile(self):
		self._startSection("whileStatement")

		self._writeKeyword()        # "While"
		self._writeSymbol()         # '('
		self.compileExpression()    # condition
		self._writeSymbol()         # ')'

		self._writeSymbol()         # '{'
		self.compileStatements()
		self._writeSymbol()         # '}'

		self._endSection("whileStatement")

	def compileReturn(self):
		self._startSection("returnStatement")

		# "Return"
		self._writeKeyword()

		if not self._tokenMatchesSymbol(';'):
			# If there is an expression, compile it.
			self.compileExpression()

		#';'
		self._writeSymbol()
		self._endSection("returnStatement")


	def compileIf(self):
		self._startSection("ifStatement")
		self._writeKeyword()            # "If"
		self._writeSymbol()             # '('
		self.compileExpression()        # condition
		self._writeSymbol()             # ')'

		self._writeSymbol()             # '{'
		self.compileStatements()        # (...)
		self._writeSymbol()             # '}'

		if self._tokenMatchesKeyword("ELSE"):
			self._writeKeyword()        # "Else"
			self._writeSymbol()         # '{'
			self.compileStatements()    # (...)
			self._writeSymbol()         # '}'

		self._endSection("ifStatement")

	def compileExpression(self):
		self._startSection("expression")
		self.compileTerm()

		while self._tokenIsOperator():
			self._writeSymbol()
			self.compileTerm()

		self._endSection("expression")

	def compileTerm(self):
	# integerConstant | stringConstant | keywordConstant |
	# varName | varName '[' expression ']' | subroutineCall |
	# '(' expression ')' | unaryOp term
		self._startSection("term")
		tokenType = self.tokenizer.tokenType()

		if self._tokenIsKeywordConstant():  # keywordConstant
			self._writeKeyword()

		elif tokenType == "INT_CONST":      # integerConstant
			self._writeIntVal()

		elif tokenType == "STRING_CONST":   # stringConstant
			self._writeStringVal()

		elif tokenType == "IDENTIFIER":     # varName
			self._writeIdentifier()
			# Is it an array?
			if self._tokenMatchesSymbol('['):   # '[' expression']'
				self._writeSymbol() # '['
				self.compileExpression()
				self._writeSymbol() # ']'
			# Is it a subroutine call?
			elif self._tokenMatchesSymbol('('): # '(' expression ')' 
				self._writeSymbol()
				self.compileExpressionList()
				self._writeSymbol()
			# Is it a method call?
			elif self._tokenMatchesSymbol('.'):
				self._writeSymbol()
				self._writeIdentifier()
				self.compileExpressionList()

		elif self._tokenMatchesSymbol('('): # '(' Expression ')'
			self._writeSymbol() # '('
			self.compileExpression()
			self._writeSymbol() # ')'

		elif self._tokenIsUnaryOperator():  # unaryOp term
			self._writeSymbol()
			self.compileTerm()

		self._endSection("term")


	def compileExpressionList(self):
		self._writeSymbol() # '('
		self._startSection("expressionList")

		# While there are expressions...
		while not self._tokenMatchesSymbol(')'):
			self.compileExpression()
			if self._tokenMatchesSymbol(','):
				self._writeSymbol()

		self._endSection("expressionList")
		self._writeSymbol() # ')'

	# --- PRIVATE functions --- #
	def _writeLine(self, line):
		prefix = ' ' * 2 * self.indentLevel
		self.output.write(prefix + line + '\n')

	def _writeKeyword(self):
		self._writeLine("<keyword> "
						+ self.tokenizer.keyWord().lower() 
						+ " </keyword>")
		self.tokenizer.advance()

	def _writeIdentifier(self):
		self._writeLine("<identifier> " 
						+ self.tokenizer.identifier() 
						+ " </identifier>")
		self.tokenizer.advance()        

	def _writeSymbol(self):
		symbol = self.tokenizer.symbol()
		if symbol == '"':
			symbol = "&quot;"
		if symbol == '&':
			symbol = "&amp;"
		elif symbol == '<':
			symbol = "&lt;"
		elif symbol == '>':
			symbol = "&gt;"

		self._writeLine("<symbol> " + symbol + " </symbol>")
		self.tokenizer.advance()

	def _writeIntVal(self):
		self._writeLine("<integerConstant> "
						+ str(self.tokenizer.intVal())
						+ " </integerConstant>")
		self.tokenizer.advance()        

	def _writeStringVal(self):
		self._writeLine("<stringConstant> "
						+ self.tokenizer.stringVal()
						+ " </stringConstant>")
		self.tokenizer.advance()

	def _writeType(self):
		if self._tokenIsPrimitiveType():
			self._writeKeyword()
		else:
			self._writeIdentifier()

	def _incIndentLevel(self):
		self.indentLevel = self.indentLevel + 1

	def _decIndentLevel(self):
		if self.indentLevel > 0:
			self.indentLevel = self.indentLevel - 1

	def _tokenIsKeyword(self):
		return self.tokenizer.tokenType() == "KEYWORD"

	def _tokenIsSymbol(self):
		return self.tokenizer.tokenType() == "SYMBOL"

	def _tokenMatchesSymbol(self, symbol):
		return self._tokenIsSymbol() and self.tokenizer.symbol() == symbol

	def _tokenMatchesKeyword(self, kw):
		return self._tokenIsKeyword() and self.tokenizer.keyWord() == kw    

	def _tokenIsOperator(self):
		return(self._tokenIsSymbol() and
			   self.tokenizer.symbol() in ['+', '-', '*', '/', '&', '|',
											'<', '>', '='])
	def _tokenIsUnaryOperator(self):
		return(self._tokenIsSymbol() and
			   self.tokenizer.symbol() in ['-', '~'])

	def _tokenIsPrimitiveType(self):
		return(self._tokenIsKeyword() and
			   self.tokenizer.keyWord() in ["INT", "CHAR", "BOOLEAN", "VOID"])

	def _tokenIsKeywordConstant(self):
		return(self._tokenIsKeyword() and
			   self.tokenizer.keyWord() in ["TRUE", "FALSE", "NULL", "THIS"])

	def _startSection(self, section):
		self._writeLine("<" + section + ">")
		self._incIndentLevel()

	def _endSection(self, section):
		self._decIndentLevel()
		self._writeLine("</" + section + ">")