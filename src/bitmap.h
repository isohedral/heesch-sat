#pragma once

#include "geom.h"

#define DEBUG_BITMAP

// Store a grid of two-bit values.
class CellBitmap
{
public:
	CellBitmap()
	{
		clear();
	}

	uint8_t get( int x, int y ) const
	{
#ifdef DEBUG_BITMAP
		if( (x <= -127) || (x >= 127) || (y <= -127) || (y >= 127) ) {
			std::cerr << "Coordinate out of range in bitmap" << std::endl;
			exit( -1 );
		}
#endif
		size_t raw_index = (y+128)*256 + (x+128);
		uint64_t ival = data[raw_index / 32];
		size_t offs = (raw_index % 32) * 2;

		return (ival >> offs) & 3;
	}

	void set( int x, int y, uint8_t val )
	{
#ifdef DEBUG_BITMAP
		if( (x <= -127) || (x >= 127) || (y <= -127) || (y >= 127) ) {
			std::cerr << "Coordinate out of range in bitmap" << std::endl;
			exit( -1 );
		}
#endif
		size_t raw_index = (y+128)*256 + (x+128);
		uint64_t& ival = data[raw_index / 32];
		size_t offs = (raw_index % 32) * 2;
		ival = (ival & (~((uint64_t)3<<offs))) | ((uint64_t)val << offs);
		/*
		std::cerr << "SET " << x << ", " << y << " -> " << raw_index << "/" << offs << std::endl;
		std::cerr << "  ival = " << ival << std::endl;
		*/
	}

	// Set, but return previous contents.
	uint8_t getAndSet( int x, int y, uint8_t val )
	{
#ifdef DEBUG_BITMAP
		if( (x <= -127) || (x >= 127) || (y <= -127) || (y >= 127) ) {
			std::cerr << "Coordinate out of range in bitmap" << std::endl;
			exit( -1 );
		}
#endif
		size_t raw_index = (y+128)*256 + (x+128);
		uint64_t& ival = data[raw_index / 32];
		size_t offs = (raw_index % 32) * 2;
		uint8_t ret = (ival >> offs) % 4;
		ival = (ival & (~((uint64_t)3<<offs))) | ((uint64_t)val << offs);
		return ret;
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
		memset( (void*)data, 0, 2048 * 8 );
	}

	void debug()
	{
		for( int y = -125; y < 125; ++y ) {
			for( int x = -125; x < 125; ++x ) {
				uint8_t v = get( x, y );
				if( v != 0 ) {
					std::cerr << "  grid[" << x << ", " << y << "] = " << (int)v << std::endl;
				}
			}
		}
	}

private:
	uint64_t data[2048];
};
