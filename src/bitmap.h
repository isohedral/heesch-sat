#pragma once

#include "geom.h"

const bool DEBUG_BITMAP = true;

// A packed array that represents N K-bit values.  We'll assume K <= 8,
// because otherwise why would you need such a complicated data structure?
template<size_t N, size_t K=1>
class multibitset
{
public:
	multibitset()
	{
		clear();
	}

	multibitset( const multibitset<N,K>& bs )
	{
		std::copy( bs.data, bs.data + num_ints, data );
	}

	uint8_t get( size_t index ) const
	{
		if( DEBUG_BITMAP ) {
			if( (index < 0) || (index >= N) ) {
				std::cerr << "Coordinate out of range in multibitset" 
					<< std::endl;
				exit( -1 );
			}
		}

		return (data[index/elements_per_int] >> (K*(index%elements_per_int)))
			& mask;
	}

	void set( size_t index, uint8_t val )
	{
		if( DEBUG_BITMAP ) {
			if( (index < 0) || (index >= N) ) {
				std::cerr << "Coordinate out of range in multibitset" 
					<< std::endl;
				exit( -1 );
			}
		}

		size_t idx = index / elements_per_int;
		size_t shft = K * (index % elements_per_int);
		data[idx] = (data[idx] & ~(mask << shft)) | (val << shft);
	}

	uint8_t getAndSet( size_t index, uint8_t val )
	{
		if( DEBUG_BITMAP ) {
			if( (index < 0) || (index >= N) ) {
				std::cerr << "Coordinate out of range in multibitset" 
					<< std::endl;
				exit( -1 );
			}
		}

		size_t idx = index / elements_per_int;
		size_t shft = K * (index % elements_per_int);
		uint8_t ret = (data[idx] >> shft) & mask;
		data[idx] = (data[idx] & ~(mask << shft)) | (val << shft);
		return ret;
	}

	void clear()
	{
		std::fill( data, data + num_ints, 0 );
	}

private:
	static constexpr size_t elements_per_int = 8 * sizeof(unsigned int) / K;
	static constexpr size_t num_ints = (N+elements_per_int-1)/elements_per_int;
	static constexpr size_t num_bytes = num_ints * sizeof(unsigned int);
	static constexpr unsigned int mask = (1<<K) - 1;

	unsigned int data[num_ints];
};

// An NxN grid of cells, where each cell stores K<=8 bits of data.
// The grid coordinates are centred: (0,0) is as close as possible
// to the centre of the grid.
template<size_t N, size_t K=1>
class bitgrid
{
public:
	bitgrid()
		: grid {}
	{}
	bitgrid( const bitgrid<N,K>& other )
		: grid { other.grid }
	{}

	uint8_t get( int x, int y ) const
	{
		if( DEBUG_BITMAP ) {
			if( (x<-offset) || (x>=offset) || (y<-offset) || (y>=offset) ) {
				std::cerr << "Coordinate out of range in bitgrid" << std::endl;
				exit( -1 );
			}
		}

		return grid.get( (y+offset)*N + (x+offset) );
	}

	void set( int x, int y, uint8_t val ) 
	{
		if( DEBUG_BITMAP ) {
			if( (x<-offset) || (x>=offset) || (y<-offset) || (y>=offset) ) {
				std::cerr << "Coordinate out of range in bitgrid" << std::endl;
				exit( -1 );
			}
		}

		grid.set( (y+offset)*N + (x+offset), val );
	}

	uint8_t getAndSet( int x, int y, uint8_t val )
	{
		if( DEBUG_BITMAP ) {
			if( (x<-offset) || (x>=offset) || (y<-offset) || (y>=offset) ) {
				std::cerr << "Coordinate out of range in bitgrid" << std::endl;
				exit( -1 );
			}
		}

		return grid.getAndSet( (y+offset)*N + (x+offset), val );
	}

	template<typename coord_t>
	uint8_t get( const point<coord_t>& pt ) const
	{
		return get( (int)pt.getX(), (int)pt.getY() );
	}

	template<typename coord_t>
	void set( const point<coord_t>& pt, uint8_t val )
	{
		set( (int)pt.getX(), (int)pt.getY(), val );
	}

	template<typename coord_t>
	uint8_t getAndSet( const point<coord_t>& pt, uint8_t val )
	{
		return getAndSet( (int)pt.getX(), (int)pt.getY(), val );
	}

	void clear()
	{
		grid.clear();
	}

private:
	static constexpr int offset = N/2;

	multibitset<N*N, K> grid;
};
