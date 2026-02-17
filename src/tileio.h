#pragma once

#include <iostream>
#include <vector>

#include "geom.h"
#include "grid.h"
#include "shape.h"

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
	using patch_t = LabelledPatch<coord_t>;

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

	void setInconclusive()
	{
		record_type_ = INCONCLUSIVE;
		patches_.clear();
	}

	void setInconclusive(const patch_t& patch)
	{
		record_type_ = INCONCLUSIVE;
		patches_.clear();
		patches_.push_back(patch);
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

	void setPeriodic(size_t transitivity = 1, const patch_t *patch = nullptr) 
	{
		patches_.clear();
		transitivity_ = transitivity;
		record_type_ = (transitivity > 1) ? ANISOHEDRAL : ISOHEDRAL;

		 if (patch) {
		 	patches_.push_back(*patch);
		 }
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

// Similar to dispatchGridTypeImpl in grid.h, read an input stream
// line by line and dispatch each tile shape found to the callable
// F, bootstrapped into the correct grid type.

template<auto F>
void processInputStreamImpl(std::istream& is, GridType default_gt = OMINO)
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
		
		// Chain to existing dispatch code (glad this can be done in
		// a single line!)
		dispatchGridTypeImpl<F>(gt, is);

		if( !mo ) {
			break;
		}
	}
}

// And as with grid.h, wrap the bootstrapping process in a little
// macro.  We can't use the same macro, because we need to construct
// a templated TileInfo object on the way in.  (It's theoretically
// possible to abstract over this, but it isn't worth the coding
// effort.)

// FIXME A bit awkward.  The function takes a TileInfo reference,
// So we need to declare a local info variable in order to have an
// lvalue to pass in.
#define processInputStream(is, f) \
    processInputStreamImpl<[]<typename Grid>(std::istream& my_is) { \
		TileInfo<Grid> info {my_is}; \
		return f<Grid>(info);}>(is);
