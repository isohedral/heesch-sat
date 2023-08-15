import sys

o = (-1,-1)
v = (3,3)
w = (-3,6)
ds = [(1,1), (-1,2), (-2,1), (-1,-1), (1,-2), (2,-1)]

black_ns = [(1,1), (-2,1), (1,-2)]
grey_ns = [(-1,2), (-1,-1), (2,-1)]

def getNeighbours( p ):
	if p[0] % 3 == 0:
		return [add( p, n ) for n in black_ns]
	else:
		return [add( p, n ) for n in grey_ns]

def add( a, b ):
	return (a[0]+b[0], a[1]+b[1])

def sc( v, a ):
	return (v[0]*a, v[1]*a)

def parse( l ):
	ns = [int(x) for x in l.split()]
	res = []

	for i in range(0,len(ns),2):
		p = (ns[i],ns[i+1])
		res.append( p )

	return res

def proc( ps ):
	res = []

	for p in ps:
		no = add( add( o, sc( v, p[0] ) ), sc( w, p[1] ) )
		for d in ds:
			nd = add( no, d )
			res.append( nd )

	return res

def enhance( ps ):
	res = []
	s = set( ps )

	for p in ps:
		for n in getNeighbours( p ):
			if n not in s:
				s.add( n )
				res.append( ps + [n] )

	return res

for l in sys.stdin:
	hexes = parse( l )
	tris = proc( hexes )

	for etris in enhance( tris ):
		print( ' '.join( ['{0} {1}'.format( *p ) for p in etris] ) )
