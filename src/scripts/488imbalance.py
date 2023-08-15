import sys

for l in sys.stdin:
	ns = [int(x) for x in l.split()]
	evens = 0
	odds = 0

	for i in range( 0, len(ns), 2 ):
		if (ns[i]+ns[i+1]) % 2 == 0:
			evens = evens + 1
		else:
			odds = odds + 1

	if abs( evens - odds ) == 1:
		sys.stdout.write( l )
