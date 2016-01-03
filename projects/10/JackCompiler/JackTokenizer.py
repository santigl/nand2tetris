#!/usr/bin/env python3
import sys

# String constants:
TOKEN_TYPES = ["KEYWORD", "SYMBOL", "IDENTIFIER", "INT_CONST", "STRING_CONST"]

TOKEN_KEYWORDS = ["CLASS", "METHOD", "FUNCTION", "CONSTRUCTOR", "INT", \
					"BOOLEAN", "CHAR", "VOID", "VAR", "STATIC", "FIELD", \
					"LET", "DO", "IF", "ELSE", "WHILE", "RETURN", "TRUE", \
					"FALSE", "NULL", "THIS"]

SYMBOLS = ['(', ')', '[', ']', '{', '}', ',', ';', '=', '.', '+', '-', '*', \
		 '/', '&', '|', '~', '<', '>']

WHITE_SPACE = [' ', '\n', '\t']


class JackTokenizer:
	_tokenTypes         = frozenset(TOKEN_TYPES)
	_keywords           = frozenset(TOKEN_KEYWORDS)
	_symbols            = frozenset(SYMBOLS)
	_whiteSpace         = frozenset(WHITE_SPACE)

	def __init__(self, filename):
		self.file = open(filename, 'r')
		self.currentToken = ""
		if self.hasMoreTokens():
			self.advance()

	def __del__(self):
		self.file.close()

	def hasMoreTokens(self):
		while True:
			if self._peek() == "":
				return False

			# Skipping spaces and newlines:
			while self._peek() in self._whiteSpace:
				self._pop()
			# Skipping comments:
			cs = self._peek(2)
			while cs in ["//", "/*"]:
				if self._peek(2) == "//":
					self._skipLine()
				else:
					self._skipComment()

				cs = self._peek(2)

			if self._peek() not in self._whiteSpace:
				# It could be a token: it is not a comment or whitespace or EOF
				return True

	def advance(self):
		if self.hasMoreTokens():
			if self._peek() != '"':
				self.currentToken = self._peekWord()
				self._pop(len(self.currentToken))
			else:
				string_const = self._pop() # Opening "
				while self._peek() != '"':
					string_const = string_const + self._pop()
				string_const = string_const + self._pop() # Closing "

				self.currentToken = string_const
			
	def tokenType(self):
		token = self.currentToken.upper()
		if token in self._symbols:
			return "SYMBOL"
		if token in self._keywords:
			return "KEYWORD"
		if '"' in token:
			return "STRING_CONST"
		if token.isdigit():
			return "INT_CONST"

		return "IDENTIFIER"

	def keyWord(self):
		return self.currentToken.upper()

	def symbol(self):
		return self.currentToken[0]

	def identifier(self):
		return self.currentToken

	def intVal(self):
		return int(self.currentToken)

	def stringVal(self):
		return self.currentToken.replace('"', '')

	# -- Private --
	def _rawToken(self):
		''' For debugging purposes. '''
		return self.currentToken

	def _peek(self, charCount=1):
		'''Reads charCount characters and returns the iterator back to where it was.'''
		originalPos = self.file.tell()
		c = self.file.read(charCount)
		self.file.seek(originalPos)
		return c

	def _pop(self, charCount=1):
		return self.file.read(charCount)

	def _skipLine(self):
		self.file.readline()

	def _skipComment(self):
		''' Drop characters until finding the sequence '*/'. 
		Also supports the sequence "/\n*", in case the line breaks mid-comment. '''
		star = False # Did we find a possible end?
		while True:
			char = self._pop()
			if char == '*':
				# We have found a candidate for the comment ending.
				star = True
			else:
				# If the character is a '/', and we have found a '*' in the
				# previous iteration, we are done.
				if star == True and char == '/':
					return
				# But if not, we check for a line break. 
				# (Just in case the '/' got broken into the next line.)
				if char != '\n':
					star = False

	def _peekWord(self):
		''' Read until finding a whitespace character or a symbol. 
		(Restores the pointer.) '''
		res = ""
		originalPos = self.file.tell()
		currentChar = self._pop()
		if currentChar in self._symbols:
			res = currentChar
		else:
			while (currentChar != "") and (currentChar not in self._whiteSpace):
				if currentChar in self._symbols:
					break
				res = res + currentChar
				currentChar = self._pop()
		self.file.seek(originalPos)
		return res

def main():
	if len(sys.argv) < 2:
		print("Error. Missing argument [filename]")
		return -1

	filename = sys.argv[1]

	jt = JackTokenizer(filename)
	while jt.hasMoreTokens():
		print(jt._rawToken(), jt.tokenType(), sep='\t')
		jt.advance()

	print(jt._rawToken(), jt.tokenType(), sep='\t') 

if __name__ == '__main__':
	main()