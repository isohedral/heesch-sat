#pragma once

#include <iostream>
#include <vector>

#include "geom.h"
#include "grid.h"
#include "shape.h"
#include "heesch.h"

// Handle text-based input and output of information about polyforms.
// It would be natural to use a standard format like JSON here, but 
// because of the sheer volume of data we'll be processing, there's 
// value in trying to be as compact as possible.  (A binary format
// would potentially be even better, but there's some virtue to having
// files be human-readable, particularly for debugging.)

// Just in case you want to, e.g., collect a bunch of heterogeneous records
// together.
class GenericTileInfo
{
	virtual GridType getGridType() const
	{
		return NOGRID;
	}
};

template<typename num = int>
struct IntReader
{
public:
	IntReader( char *buf ) : buf_ { buf } { advance(); }
	num operator *() { return (num)atoi( buf_ ); }
	bool operator ==( const IntReader<num>& other ) const
	{ return buf_ == other.buf_; }
	bool operator !=( const IntReader<num>& other ) const
	{ return buf_ != other.buf_; }
	IntReader<num>& operator++()
	{ hop(); advance(); return *this; }
	IntReader<num> operator++( int )
	{ IntReader<num> ret { buf_ }; hop(); advance(); return ret; }

	void hop()
	{
		while( *buf_ != '\0' ) {
			if( std::isdigit( *buf_ ) || (*buf_ == '-') ) {
				++buf_;
			} else {
				break;
			}
		}
	}

	void advance()
	{
		while( *buf_ != '\0' ) {
			if( std::isdigit( *buf_ ) || (*buf_ == '-') ) {
				break;
			} else {
				++buf_;
			}
		}
	}

	char *buf_;
};

template<typename grid>
class TileInfo
	: GenericTileInfo
{
	using coord_t = typename grid::coord_t;
	using xform_t = typename grid::xform_t;
	using patch_t = Solution<coord_t>;

public:
	enum RecordType
	{
		UNKNOWN,
		HOLE,
		INCONCLUSIVE,

		NONTILER,
		ISOHEDRAL,
		ANISOHEDRAL, // Not supported
		APERIODIC 	 // Not supported
	};

public:
	TileInfo()
		: record_type_ { UNKNOWN }
		, shape_ {}
		, hc_ { 0 }
		, hh_ { 0 }
		, patches_ {}
		, transitivity_ { 0 }
	{}

	TileInfo( std::istream& is );

	virtual GridType getGridType() const 
	{ 
		return grid::grid_type; 
	}

	const Shape<grid>& getShape() const
	{
		return shape_;
	}

	const size_t getRecordType() const
	{
		return record_type_;
	}

	void setShape( const Shape<grid>& shape )
	{
		shape_ = shape;
	}

	void setRecordType( RecordType record_type )
	{
		record_type_ = record_type;
	}

	size_t numPatches() const
	{
		return patches_.size();
	}

	const patch_t& getPatch( size_t idx ) const
	{
		return patches_[idx];
	}

	size_t getHeeschConnected() const
	{
		return hc_;
	}

	size_t getHeeschHoles() const
	{
		return hh_;
	}

	size_t getTransitivity() const
	{
		return transitivity_;
	}

	void setInconclusive( const patch_t* patch = nullptr )
	{
		record_type_ = INCONCLUSIVE;
		patches_.clear();

		if( patch ) {
			patches_.push_back( *patch );
		}
	}

	void setNonTiler( 
		size_t hc, const patch_t* hc_patch, size_t hh, const patch_t* hh_patch )
	{
		record_type_ = NONTILER;
		patches_.clear();

		// Patches can be implicit if Heesch number is zero.

		hc_ = hc;
		if( (hc > 0) && hc_patch ) {
			patches_.push_back( *hc_patch );
		}
		hh_ = hh;
		if( (hh_ > hc_) && hh_patch ) {
			patches_.push_back( *hh_patch );
		}
	}	

	void setPeriodic( size_t transitivity = 1 ) 
	{
		patches_.clear();
		transitivity_ = transitivity;

		record_type_ = (transitivity > 1) ? ANISOHEDRAL : ISOHEDRAL;
	}

	void write( std::ostream& os ) const;

private:
	patch_t readPatch( std::istream& is, char *buf )
	{
		patch_t patch;
		is.getline( buf, 1000 );
		size_t sz = atoi( buf );
		for( size_t idx = 0; idx < sz; ++idx ) {
			is.getline( buf, 1000 );
			IntReader<coord_t> i { buf };
			patch.emplace_back( *i++, 
				xform_t { *i++, *i++, *i++, *i++, *i++, *i++ } );
		}

		// Move semantics.
		return patch;
	}

	RecordType record_type_;
	Shape<grid> shape_;

	size_t hc_;
	size_t hh_;

	std::vector<patch_t> patches_;
	
	// For periodic, number of transitivity classes
	size_t transitivity_;
};

template<typename grid>
TileInfo<grid>::TileInfo( std::istream& is )
	: TileInfo {}
{
	// Assume that the character code giving us the grid type has already
	// been consumed (which is how we resolved the binding on the grid.
	// So start with coordinates.
	char buf[1000];
	is.getline( buf, 1000 );
	// Special syntax to declare UNKNOWN and skip any other specifications.
	bool naked = (buf[0] == '?');

	auto iend = IntReader<coord_t> { buf + is.gcount() - 1 };
	for( auto i = IntReader<coord_t> { buf }; i != iend; ) {
		shape_.add( *i++, *i++ );
	}
	shape_.complete();

	if( naked ) {
		record_type_ = UNKNOWN;
		return;
	}

	is.getline( buf, 1000 );
	switch( buf[0] ) {
		case '?':
			record_type_ = UNKNOWN;
			break;
		case 'O':
			record_type_ = HOLE;
			break;
		case '!':
			record_type_ = INCONCLUSIVE;
			break;
		case '~':
			record_type_ = NONTILER;
			break;
		case 'I':
			record_type_ = ISOHEDRAL;
			break;
		case '#':
			record_type_ = ANISOHEDRAL;
			break;
		case '$':
			record_type_ = APERIODIC;
			break;
	}

	auto i = IntReader<size_t> { buf + 1 };

	if( record_type_ == NONTILER ) {
		hc_ = *i++;
		hh_ = *i++;
	} else if( record_type_ == ISOHEDRAL || record_type_ == ANISOHEDRAL ) {
		transitivity_ = *i++;
	}

	size_t num_patches = *i;

	for( size_t idx = 0; idx < num_patches; ++idx ) {
		patches_.push_back( std::move( readPatch( is, buf ) ) );
	}
}

template<typename grid>
void TileInfo<grid>::write( std::ostream& os ) const
{
	os << gridTypeAbbreviation( grid::grid_type );
	if( record_type_ == UNKNOWN ) {
		// Make a naked record
		os << '?';
	}

	for( const auto& p : shape_ ) {
		os << ' ' << p.x_ << ' ' << p.y_;
	}	
	// os << std::endl;
	os << '\n';

	if( record_type_ == UNKNOWN ) {
		return;
	}

	switch( record_type_ ) {
		case UNKNOWN: 
			os << '?';
			break;
		case HOLE:
			os << 'O';
			break;
		case INCONCLUSIVE:
			os << '!';
			break;
		case NONTILER:
			os << "~ " << hc_ << ' ' << hh_;
			break;
		case ISOHEDRAL:
			os << "I " << transitivity_;
			break;
		case ANISOHEDRAL:
			os << "# " << transitivity_;
			break;
		case APERIODIC:
			os << "$";
			break;
	}

	os << ' ' << patches_.size() << '\n'; // std::endl;

	for( const auto& patch : patches_ ) {
		os << patch.size() << '\n'; // std::endl;
		for( const auto& p : patch ) {
			os << p.first << ' ' << p.second << '\n'; // std::endl;
		}
	}
}

template<template<typename> typename g, template<typename> class F>
bool processOne( std::istream& is )
{
	using coord = int16_t;
	using grid = g<coord>;
	using Func = F<grid>;

	return Func()( TileInfo<grid>( is ) );
}

// This is surprisingly tricky to get working.  The problem is that you need
// to dispatch to one of the templated grid classes almost immediately, but
// you don't know which class you'll need until you actually start parsing
// the input.  Of course, that's why the grid type is the first character 
// in the description of a tile.  Life might have been a bit easier if we
// had mandated that the grid type is homogeneous across all polyforms in 
// the input, but that restriction seemed annoying.

template<template<typename grid> class Func>
void processInputStream( std::istream& is, GridType default_gt = OMINO )
{
	while( true ) {
		bool mo = true;
		
		int ch = is.peek();
		if( ch == EOF ) {
			return;
		}

		GridType gt = default_gt;

		if( !(std::isdigit( ch ) || (ch == '-')) ) {
			ch = is.get();
			gt = getGridType( ch );
		}

		// FIXME This duplicates the code from dispatchToGridType.  That's
		// annoying -- it would be much nicer to have a single universal
		// dispatch mechanism that could be used here too.  But it seems
		// intractable because of the need to pass along Func as a kind of
		// lambda.  Punt on this for now.
		switch( gt ) {
			case OMINO: mo = processOne<OminoGrid,Func>( is ); break;
			case HEX: mo = processOne<HexGrid,Func>( is ); break;
			case IAMOND: mo = processOne<IamondGrid,Func>( is ); break;
			case OCTASQUARE: mo = processOne<OctaSquareGrid,Func>( is ); break;
			case TRIHEX: mo = processOne<TriHexGrid,Func>( is ); break;
			case ABOLO: mo = processOne<AboloGrid,Func>( is ); break;
			case DRAFTER: mo = processOne<DrafterGrid,Func>( is ); break;
			case KITE: mo = processOne<KiteGrid,Func>( is ); break;
			case HALFCAIRO: mo = processOne<HalfCairoGrid,Func>( is ); break;
			case BEVELHEX: mo = processOne<BevelHexGrid,Func>( is ); break;
			default:
				break;
		}

		if( !mo ) {
			break;
		}
	}
}

#define FOR_EACH_IN_STREAM( is, f ) \
	processInputStream<f##Wrapper>( is );
