class SymbolTable:
	def __init__(self):
		self._global 	= dict()
		self._local		= dict()

		self._global_static_index = 0
		self._global_field_index = 0
		self._local_arg_index = 0
		self._local_var_index = 0

	def startSubroutine(self):
		self._local.clear()
		self._local_arg_index = 0
		self._local_var_index = 0

	def define(self, name, jackType, kind):
		kind = kind.upper()
		if kind == "STATIC":
			self._global[name] = (jackType, kind, self._global_static_index)
			self._global_static_index += 1
		elif kind == "FIELD":
			self._global[name] = (jackType, kind, self._global_field_index)
			self._global_field_index += 1
		elif kind == "ARG":
			self._local[name] = (jackType, kind, self._local_arg_index)
			self._local_arg_index += 1
		elif kind == "LOCAL":
			self._local[name] = (jackType, kind, self._local_var_index)
			self._local_var_index += 1

	def varCount(self, kind):
		kind = kind.upper()
		res = 0
		for (symbol, info) in self._local.items():
			if info[1] == kind:
				res += 1
		for (symbol, info) in self._global.items():
			if info[1] == kind:
				res += 1

		return res

	def typeOf(self, name):
		if name in self._local:
			return self._local[name][0]
		if name in self._global:
			return self._global[name][0]

	def kindOf(self, name):
		if name in self._local:
			return self._local[name][1]
		if name in self._global:
			return self._global[name][1]
		return "NONE"

	def indexOf(self, name):
		if name in self._local:
			return self._local[name][2]
		if name in self._global:
			return self._global[name][2]