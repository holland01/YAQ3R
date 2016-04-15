import fileinput
import os

fpath = os.path.join('..', 'worker', 'file_traverse.js')

terms = ['\'', '\"']
terms_len = len(terms)

with open(fpath, 'rb') as f:
	origin = f.read().decode('utf-8').splitlines()
	count = 0
	found = False
	skip = False

	def report(type, column):
		global found
		print('Invalid ' + type + ' index break at line:column -> %i:%i' % \
			(count, column))
		found = True

	comment = ''

	def find_end_block(line, same):
		global skip
		global comment
		if skip:
			if '*/' in line:
				k = line.find('*/')
				v = 'line' if same else 'block'
				print('end comment ' + v + ' at %i:%i ->\n %s' % (count, k, comment))
				comment = ''
				skip = False
				return (True, True, k + 2)
			else:
				return (True, False, -1)
		return (False, False, -1)

	double_indices = []
	single_indices = []

	for line in origin:
		if skip:

		find_end_block(line, False)
		if skip:
			continue
		double_indices.clear()
		single_indices.clear()
		count += 1
		llen = len(line)
		i = 0
		while i < llen:
			# Ignore comments
			if i < llen - 1:
				if line[i] == '/' and line[i + 1] == '/':
					break
				if line[i] == '/' and line[i + 1] == '*':
					x = i + 2
					skip = True
					print('begin comment block at: %i:%i' % (count, i))
					find_end_block(line, True)
					if skip:
						comment += line[(i-1):]
						break

			if line[i] == '\'':
				single_indices.append(i)
			elif line[i] == '\"':
				double_indices.append(i)
			i += 1
		if skip:
			continue

		si_len = len(single_indices)
		di_len = len(double_indices)
		if si_len % 2 != 0:
			if di_len % 2 == 0:
				is_valid = False
				for u in single_indices:
					v = 0
					while v < di_len:
						if u in range(double_indices[v],\
							double_indices[v + 1]):
							is_valid = True
							break
						v += 2
				if not is_valid:
					report('single', single_indices[si_len - 1])
					break
			else:
				report('single', single_indices[si_len - 1])
				break

		if di_len % 2 != 0:
			report('double', double_indices[di_len - 1])
			break
