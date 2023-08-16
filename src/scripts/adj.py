L = '''<-1,0,-28,1,1,14>
<-1,-1,-14,1,0,28>
<1,1,-21,0,-1,21>
<1,1,0,0,-1,21>
<0,1,-21,-1,-1,21>
<-1,-1,0,1,0,21>
<1,0,0,-1,-1,14>
<0,-1,7,-1,0,7>
<1,1,-21,-1,0,14>
<0,-1,14,1,1,7>
<1,1,-14,-1,0,0>
<1,1,0,-1,0,7>
<0,-1,0,1,1,14>
<1,0,-14,0,1,7>
<-1,0,-21,0,-1,14>
<1,1,7,0,-1,7>
<0,-1,-7,1,1,14>
<1,0,14,-1,-1,7>
<-1,-1,-14,0,1,14>
<-1,0,-14,0,-1,14>
<1,0,14,0,1,-7>
<-1,0,-28,0,-1,28>
<1,1,-14,0,-1,7>
<0,1,0,-1,-1,0>
<-1,-1,7,1,0,7>
<0,-1,14,-1,0,-7>
<-1,0,-14,1,1,0>
<1,1,-7,-1,0,-7>
<-1,-1,-14,1,0,14>
<0,1,-14,-1,-1,0>
<0,1,-7,-1,-1,14>
<-1,0,7,0,-1,0>
<1,0,-14,-1,-1,21>
<1,1,0,0,-1,0>
<1,1,-14,0,-1,28>
<0,-1,-14,-1,0,14>
<0,-1,14,-1,0,-14>
<-1,-1,0,0,1,-14>
<1,1,0,0,-1,7>
<1,1,-7,0,-1,7>
<1,0,14,0,1,-14>
<-1,0,0,1,1,7>
<-1,0,-14,1,1,14>
<1,0,-7,-1,-1,7>
<-1,-1,-7,0,1,-7>
<1,0,0,0,1,7>
<-1,0,0,1,1,-7>
<1,1,0,-1,0,-14>
<0,1,-28,-1,-1,14>
<1,0,7,-1,-1,0>
<0,1,-7,1,0,0>
<0,1,-14,1,0,21>
<-1,-1,7,0,1,0>
<0,1,-28,1,0,28>
<1,0,0,0,1,-7>
<-1,-1,0,1,0,0>
<0,1,0,1,0,7>
<0,-1,-14,1,1,14>
<1,0,0,-1,-1,0>
<-1,0,-14,0,-1,21>
<-1,-1,-14,0,1,7>
<-1,0,0,0,-1,14>
<0,-1,-7,-1,0,14>
<-1,-1,-14,0,1,0>
<0,-1,7,1,1,-7>
<0,1,-21,1,0,14>
<-1,0,0,0,-1,0>
<1,0,-14,0,1,14>'''.split( '\n' )

bd = '2 1 1 2 -1 3 1 4 -1 5 -2 6 -10 8 -5 8 -3 8 -10 9 -6 9 -9 10 -8 10 -12 11 -11 12'

for a in L:
	print( '''{0}
Hc = 1 Hh = 1
2
0 ; <1,0,0,0,1,0>
1 ; {1}'''.format( bd, a ) )
