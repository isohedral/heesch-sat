## A quick script to generate the tables of edge and all neighbours
## for the kite grid.  Define arrays for one canonical tile (east of
## the origin) and rotate it around six times.

east_edge_neighbours = [ (1,1), (2,-1), (-1,1), (0,-1) ]
east_vertex_neighbours = [ (0,2), (2,-2), (-2,0), (-2,1), (-1,-1) ]
dirs = ['east', 'northeast', 'northwest', 'west', 'southwest', 'southeast']

rot = [0,-1,0, 1,1,0]

def add( q, p ):
	return (q[0]+p[0], q[1]+p[1])

def sub( q, p ):
	return (q[0]-p[0], q[1]-p[1])

def mul( T, p ):
	return (T[0]*p[0]+T[1]*p[1]+T[2], T[3]*p[0]+T[4]*p[1]+T[5])

print( '''
template<typename coord>
const point<int8_t> KiteGrid<coord>::edge_neighbours[6][4] = {''' )

v = (1,0)
ns = [add(n,v) for n in east_edge_neighbours]
d = 0

for i in range( 6 ):
	print( '    {{ // {0}'.format( dirs[d] ) )
	for n in ns:
		w = sub( n, v )
		print( '        {{ {0}, {1} }},'.format( w[0], w[1] ) ) 
	print( '    },' )

	v = mul( rot, v )
	ns = [mul(rot,v) for v in ns]
	d = d + 1

print( '};' )

print( '''
template<typename coord>
const point<int8_t> KiteGrid<coord>::all_neighbours[6][9] = {''' )

v = (1,0)
ns = [add(n,v) for n in east_edge_neighbours + east_vertex_neighbours ]
d = 0

for i in range( 6 ):
	print( '    {{ // {0}'.format( dirs[d] ) )
	for n in ns:
		w = sub( n, v )
		print( '        {{ {0}, {1} }},'.format( w[0], w[1] ) ) 
	print( '    },' )

	v = mul( rot, v )
	ns = [mul(rot,v) for v in ns]
	d = d + 1

print( '};' )
