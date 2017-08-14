def give(x, n): return (100 / 99 - (100 * 1) / (99 * x)) * ((1 << n) - 1)

def print_it(bitdepth, mi, ma):
	print("[bitdepth: %i]" % bitdepth)
	for i in range(mi, ma):
		print("\t[%i] give(%i, %i) = %i" % (i, i, bitdepth, give(i, bitdepth)))


o = 80 
p = 100

print_it(8, o, p)
print_it(16, o, p)
print_it(24, o, p)
