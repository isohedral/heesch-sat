#pragma once

template<typename T>
struct method_hash
{
public:
	size_t operator()( const T& obj ) const
	{ 
		return obj.hash();
	}
};

using tile_index = int32_t;
using cell_index = int32_t;

// Define grid types here so that individual grids can report their types
// cleanly
enum GridType {
	NOGRID = -1,

	OMINO = 0,
	HEX = 1,
	IAMOND = 2,
	OCTASQUARE = 3,
	TRIHEX = 4,
	ABOLO = 5,
	DRAFTER = 6, 
	KITE = 7, 
	HALFCAIRO = 8
};

// Get a grid type from a single-character abbreviation.  Don't use 
// digits or - as grid abbreviations.
inline GridType getGridType( int ch )
{
	switch( ch ) {
		case 'O': return OMINO;
		case 'H': return HEX;
		case 'I': return IAMOND;
		case 'o': return OCTASQUARE;
		case 'T': return TRIHEX;
		case 'A': return ABOLO;
		case 'D': return DRAFTER;
		case 'K': return KITE;
		case 'h': return HALFCAIRO;
		default: return NOGRID;
	};
}

inline char gridTypeAbbreviation( GridType gt )
{
	switch( gt ) {
		case OMINO: return 'O';
		case HEX: return 'H';
		case IAMOND: return 'I';
		case OCTASQUARE: return 'o';
		case TRIHEX: return 'T';
		case ABOLO: return 'A';
		case DRAFTER: return 'D';
		case KITE: return 'K';
		case HALFCAIRO: return 'h';
		default: return '?';
	}
}
