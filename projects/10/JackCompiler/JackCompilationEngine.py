import JackTokenizer
import SymbolTable
import VMWriter

class JackCompilationEngine():
	def __init__(self, in_filename, out_filename):
		self._tokenizer = JackTokenizer.JackTokenizer(in_filename)

		# Initializing structures:
		self._symbol_table = SymbolTable.SymbolTable()
		self._VM_out = VMWriter.VMWriter(out_filename)

		# Initializing context variables:
		self._label_index = 0
		self._if_index = 0
		self._while_index = 0

		# Start compilation:
		self.compileClass()

	def compileClass(self):
		self._getKeyword()    # "Class"
		self._class_name = self._getIdentifier() #  className
		self._getSymbol()     # '{'

		# Variable declarations:
		if self._tokenizer.keyWord() in ["STATIC", "FIELD"]:
			self.compileClassVarDec()

		# Class' subroutines declarations:
		while (self._tokenizer.keyWord() in ["CONSTRUCTOR", "FUNCTION",
											"METHOD", "VOID"]):
			self.compileSubroutine()

		self._getSymbol()     # '}'

	def compileClassVarDec(self):
		# While there are lines declaring variables... (There could be 0.)
		while (self._tokenMatchesKeyword("STATIC") or
			   self._tokenMatchesKeyword("FIELD")):
			var_kind = self._getKeyword()        # "static"/"field"
			var_type = self._getType()           # Var. type
			var_name = self._getIdentifier()     # Var. name

			self._defineVar(var_name, var_type, var_kind)

			# Are there more variables in the same line?
			while self._tokenizer.symbol() == ',':
				self._getSymbol()
				var_name = self._getIdentifier() # Var. name
				self._defineVar(var_name, var_type, var_kind)


			self._getSymbol()         # ';' symbol at the end of the line.

	def compileSubroutine(self):
		self._symbol_table.startSubroutine()

		subroutine_type = self._getKeyword()	# Constructor/Function/Method
		return_type = self._getType()           # Return type
		subroutine_name = self._getIdentifier()	# Subroutine name

		# A method will receive a pointer as the first argument,
		# we reserve index 0 for it.
		if subroutine_type == "METHOD":
			self._defineVar("this_ptr", "INT", "ARG")

		self.compileParameterList()				# Parameters (may be empty)


		# Subroutine body:
		self._getSymbol()         # '{'

		local_vars = 0
		if self._tokenizer.keyWord() == "VAR":
			local_vars = self.compileVarDec()

		call_name = self._class_name + "." + subroutine_name
		self._VM_out.writeFunction(call_name, local_vars)

		if subroutine_type == "METHOD":
			# Argument 0 of a constructor is the this pointer.
			# The first thing the function does is move it to the pointer register.
			self._VM_out.writePush("ARGUMENT", 0)
			self._VM_out.writePop("POINTER", 0)

		elif subroutine_type == "CONSTRUCTOR":
			# If it is a method, invoke the OS functions to allocate space.
			self._VM_out.writeAlloc(self._symbol_table.varCount("FIELD"))
			# Then we set the this pointer to the assigned space.
			self._VM_out.writePop("POINTER", 0)


		if not self._tokenMatchesSymbol('}'):
			self.compileStatements()
		self._getSymbol()         # '}'
		# End of subroutine body.


		# If the return type is void, we need to push some value.
		# That way the caller can always pop at least one value.
		if return_type == "VOID":
			self._VM_out.writePush("constant", 0)
		self._VM_out.writeReturn()

	def compileParameterList(self):
		self._getSymbol()	# '('

		while self._tokenizer.symbol() != ')':
			var_type = self._getKeyword()            # Type
			var_name = self._getIdentifier()         # Name
			self._defineVar(var_name, var_type, "ARG")

			# More parameters?
			if self._tokenizer.symbol() == ',':
				self._getSymbol()

		self._getSymbol()	# ')'


	def compileVarDec(self):
		vars_declared = 0
		# While there are lines declaring variables... (There could be 0.)
		while self._tokenMatchesKeyword("VAR"):
			self._getKeyword()    				# "Var"
			var_type = self._getType()			# Var. type
			var_name = self._getIdentifier()	# Var. name

			self._defineVar(var_name, var_type, "LOCAL")
			vars_declared +=1

			# Are there more variables in the same line?
			while self._tokenizer.symbol() == ',':
				self._getSymbol()
				var_name = self._getIdentifier()
				self._defineVar(var_name, var_type, "LOCAL")
				vars_declared +=1

			self._getSymbol() # ';' symbol at the end of the line.
		return vars_declared


	def compileStatements(self):
		while not self._tokenMatchesSymbol('}'):
			if self._tokenizer.keyWord() == "LET":
				self.compileLet()

			elif self._tokenizer.keyWord() == "RETURN":
				self.compileReturn()

			elif self._tokenizer.keyWord() == "DO":
				self.compileDo()

			elif self._tokenizer.keyWord() == "IF":
				self.compileIf()

			elif self._tokenizer.keyWord() == "WHILE":
				self.compileWhile()

	def compileDo(self):
		self._getKeyword()        			# "Do"
		name = self._getIdentifier()    	# Subroutine name/(class/var)
		self._writeSubroutineCall(name, returns_void = True)
		self._getSymbol()         # ';'

	def compileLet(self):
		self._getKeyword() # "Let"
		var_name = self._getIdentifier() # Variable name

		array = False
		if self._tokenizer.symbol() == '[':
			array = True

			self._writeVarPush(var_name)
			self._getSymbol()         	#'['
			self.compileExpression()    # expr
			self._getSymbol()         	#']'
			self._VM_out.writeArithmetic('+')
			self._VM_out.writePop("POINTER", 1) # Push to the THAT pointer.

		self._getSymbol()             	# '='
		self.compileExpression()

		if array:
			self._VM_out.writePop("THAT", 0)
		else:
			self._writeVarPop(var_name)

		self._getSymbol() #';'

	def compileWhile(self):
		self._while_index += 1

		while_begin_label = "W%d" % self._while_index
		while_end_label = "Wend%d" % self._while_index

		self._VM_out.writeLabel(while_begin_label)

		self._getKeyword()        	# "While"
		self._getSymbol()         	# '('
		self.compileExpression()	# condition
		self._getSymbol()         	# ')'

		# While guard. Negating it and making a goto in case its false:
		self._VM_out.writeArithmetic("~")
		self._VM_out.writeIf(while_end_label)

		self._getSymbol()         # '{'
		self.compileStatements()
		self._getSymbol()         # '}'
		self._VM_out.writeGoto(while_begin_label)

		self._VM_out.writeLabel(while_end_label)

	def compileReturn(self):
		self._getKeyword() # "Return"

		if not self._tokenMatchesSymbol(';'):
			self.compileExpression() # If there is an expression, compile it.

		self._getSymbol() #';'


	def compileIf(self):
		self._if_index += 1

		self._getKeyword()            	# "If"
		self._getSymbol()             	# '('
		self.compileExpression()		# condition
		self._getSymbol()             	# ')'
		self._VM_out.writeArithmetic("~")

		false_label 	= "ifF%d" % self._if_index
		true_label 		= "ifT%d" % self._if_index
		end_if_label 	= "ifEnd%d" % self._if_index

		self._VM_out.writeIf(false_label)

		self._getSymbol()             	# '{'
		self._VM_out.writeLabel(true_label)
		self.compileStatements()        # (...)
		self._VM_out.writeGoto(end_if_label)
		self._getSymbol()             	# '}'

		self._VM_out.writeLabel(false_label)
		if self._tokenMatchesKeyword("ELSE"):
			self._getKeyword()        	# "Else"
			self._getSymbol()         	# '{'
			self.compileStatements()	# (...)r
			self._getSymbol()         	# '}'

		self._VM_out.writeLabel(end_if_label)

	def compileExpression(self):
		self.compileTerm()

		while self._tokenIsOperator():
			command = self._getSymbol()
			self.compileTerm()
			self._VM_out.writeArithmetic(command)

	def compileTerm(self):
	# integerConstant | stringConstant | keywordConstant |
	# varName | varName '[' expression ']' | subroutineCall |
	# '(' expression ')' | unaryOp term
		tokenType = self._tokenizer.tokenType()

		if self._tokenIsKeywordConstant():  # keywordConstant
			kw = self._getKeyword()
			if kw == "FALSE" or kw == "NULL":
				self._VM_out.writePush("constant", 0)
			elif kw == "TRUE":	# -1 in two's complement.
				self._VM_out.writePush("constant", 0)
				self._VM_out.writeArithmetic("~")
			elif kw == "THIS":
				self._VM_out.writePush("pointer", 0)

		elif tokenType == "INT_CONST":      # integerConstant
			self._VM_out.writePush("constant", self._getIntVal())


		elif tokenType == "STRING_CONST":   # stringConstant
			string = self._getStringVal()
			self._VM_out.writeString(string)


		elif tokenType == "IDENTIFIER":     # varName
			var_name = self._getIdentifier()
			var_kind = self._symbol_table.kindOf(var_name)

			# Is it an array?
			self._writeVarPush(var_name)
			if self._tokenMatchesSymbol('['):   # '[' expression']'
				self._getSymbol() # '['
				self._VM_out.writePush("POINTER", 1) # Save the current THAT pointer
				self._VM_out.writePop("TEMP", 0)	# into TEMP0.
				self.compileExpression()
				# Adding the expression to the previously loaded base pointer:
				self._VM_out.writeArithmetic('+')
				self._VM_out.writePop("POINTER", 1)
				self._VM_out.writePush("THAT", 0) # Deferefencing into TEMP1.
				self._VM_out.writePop("TEMP", 1)
				# Re-establish THAT pointer.
				self._VM_out.writePush("TEMP", 0)
				self._VM_out.writePop("POINTER", 1)

				self._VM_out.writePush("TEMP", 1) # Push result.
				self._getSymbol() # ']'

			# Is it a subroutine call?
			elif self._tokenMatchesSymbol('('): # '(' expression ')'
				self._writeSubroutineCall(var_name)
			# Is it a method call?
			elif self._tokenMatchesSymbol('.'):
				self._writeSubroutineCall(var_name)

			#else:
				#self._writeVarPush(var_name)

		elif self._tokenMatchesSymbol('('): # '(' Expression ')'
			self._getSymbol() # '('
			exp = self.compileExpression()
			self._getSymbol() # ')'

		elif self._tokenIsUnaryOperator():  # unaryOp term
			symbol = self._getSymbol()
			self.compileTerm()
			if symbol == '-':
				symbol = 'neg'
			self._VM_out.writeArithmetic(symbol)

	def compileExpressionList(self):
		self._getSymbol() # '('

		number_of_expressions = 0
		# While there are expressions...
		while not self._tokenMatchesSymbol(')'):
			exp = self.compileExpression()
			number_of_expressions += 1

			if self._tokenMatchesSymbol(','):
				self._getSymbol()

		self._getSymbol() # ')'
		return number_of_expressions

	# --- PRIVATE functions --- #
	def _getKeyword(self):
		keyword = self._tokenizer.keyWord()
		self._tokenizer.advance()
		return keyword

	def _getIdentifier(self):
		identifier = self._tokenizer.identifier()
		self._tokenizer.advance()
		return identifier

	def _getSymbol(self):
		symbol = self._tokenizer.symbol()
		self._tokenizer.advance()
		return symbol

	def _getIntVal(self):
		int_val = self._tokenizer.intVal()
		self._tokenizer.advance()
		return int_val

	def _getStringVal(self):
		string_val = self._tokenizer.stringVal()
		self._tokenizer.advance()
		return string_val

	def _getType(self):
		if self._tokenIsPrimitiveType():
			return self._getKeyword()
		else:
			return self._getIdentifier()

	def _tokenIsKeyword(self):
		return self._tokenizer.tokenType() == "KEYWORD"

	def _tokenIsSymbol(self):
		return self._tokenizer.tokenType() == "SYMBOL"

	def _tokenMatchesSymbol(self, symbol):
		return self._tokenIsSymbol() and self._tokenizer.symbol() == symbol

	def _tokenMatchesKeyword(self, kw):
		return self._tokenIsKeyword() and self._tokenizer.keyWord() == kw

	def _tokenIsOperator(self):
		return(self._tokenIsSymbol() and
			   self._tokenizer.symbol() in ['+', '-', '*', '/', '&', '|',
											'<', '>', '='])
	def _tokenIsUnaryOperator(self):
		return(self._tokenIsSymbol() and
			   self._tokenizer.symbol() in ['-', '~'])

	def _tokenIsPrimitiveType(self):
		return(self._tokenIsKeyword() and
			   self._tokenizer.keyWord() in ["INT", "CHAR", "BOOLEAN", "VOID"])

	def _tokenIsKeywordConstant(self):
		return(self._tokenIsKeyword() and
			   self._tokenizer.keyWord() in ["TRUE", "FALSE", "NULL", "THIS"])

	def _defineVar(self, name, varType, kind):
		if self._symbol_table.kindOf(name) == "NONE":
			self._symbol_table.define(name, varType, kind)

	def _writeVarPush(self, var_name):
		''' Writes the operations to push a variable depending on its kind. '''
		var_kind = self._symbol_table.kindOf(var_name)
		if var_kind == "NONE":
			return

		var_index = self._symbol_table.indexOf(var_name)
		if var_kind == "FIELD":
			self._VM_out.writePush("THIS", var_index)
		elif var_kind == "STATIC":
			self._VM_out.writePush("STATIC", var_index)
		elif var_kind == "LOCAL":
			self._VM_out.writePush("LOCAL", var_index)
		elif var_kind == "ARG":
			self._VM_out.writePush("ARGUMENT", var_index)

	def _writeVarPop(self, var_name):
		''' Writes the operations to pop a variable depending on its kind. '''
		var_kind = self._symbol_table.kindOf(var_name)
		if var_kind == "NONE":
			return

		var_index = self._symbol_table.indexOf(var_name)
		if var_kind == "FIELD":
			self._VM_out.writePop("THIS", var_index)
		elif var_kind == "STATIC":
			self._VM_out.writePop("STATIC", var_index)
		elif var_kind == "LOCAL":
			self._VM_out.writePop("LOCAL", var_index)
		elif var_kind == "ARG":
			self._VM_out.writePop("ARGUMENT", var_index)

	def _writeSubroutineCall(self, name, returns_void = False):
		call_name = ""
		method_name = ""
		push_pointer = False

		if self._tokenMatchesSymbol('.'): 	# Method call.
			self._getSymbol()     # '.'
			method_name = self._getIdentifier()

		if method_name == "":
			# Implicit class, equivalent to "self.method()".
			# Appending the current/local class name to the function,
			# and pushing the "this" pointer.
			push_pointer = True
			self._VM_out.writePush("POINTER", 0)
			call_name = "%s.%s" % (self._class_name, name)
		else:
			kind = self._symbol_table.kindOf(name)
			if kind == "NONE": # "name" is a class: call it directly.
				call_name = "%s.%s" % (name, method_name)
			else:
				t = self._symbol_table.typeOf(name) # Get the variable's class.
				call_name = "%s.%s" % (t, method_name)
				push_pointer = True # Push the location to which the variable points.
				self._writeVarPush(name)

		number_of_parameters = self.compileExpressionList()

		if push_pointer:
			number_of_parameters +=1

		self._VM_out.writeCall(call_name, number_of_parameters)

		if returns_void: # Void functions return 0. We ignore that value.
			self._VM_out.writePop("TEMP", 0)