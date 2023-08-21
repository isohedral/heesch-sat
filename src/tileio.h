#pragma once

#include <iostream>
#include <vector>

#include "geom.h"
#include "grid.h"
#include "shape.h"
#include "heesch.h"

// Just in case you want to, e.g., collect a bunch of heterogeneous records
// together.
class GenericTileInfo
{
	virtual GridType getGridType() const
	{
		return NOGRID;
	}
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
		, hc_patch_ {}
		, hh_patch_ {}
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

	size_t getHeeschConnected() const 
	{
		return hc_;
	}

	const patch_t& getHeeschConnectedPatch() const 
	{
		return hc_patch_;
	}

	size_t getHeeschHoles() const
	{
		return hh_;
	}

	const patch_t& getHeeschHolesPatch() const 
	{
		return hh_patch_;
	}

	size_t getTransitivity() const
	{
		return transitivity_;
	}

	void setNonTiler( 
		size_t hc, const patch_t* hc_patch, size_t hh, const patch_t* hh_patch )
	{
		record_type_ = NONTILER;

		// Patches can be implicit if Heesch number is zero.

		hc_ = hc;
		if( (hc > 0) && hc_patch ) {
			hc_patch_ = *hc_patch;
		} else {
			hc_patch_.clear();
		}
		hh_ = hh;
		if( (hh_ > hc_) && hh_patch ) {
			hh_patch_ = *hh_patch;
		} else {
			hh_patch_.clear();
		}
	}	

	void setPeriodic( size_t transitivity = 1 ) 
	{
		hc_patch_.clear();
		hh_patch_.clear();
		transitivity_ = transitivity;

		record_type_ = (transitivity > 1) ? ANISOHEDRAL : ISOHEDRAL;
	}

	void write( std::ostream& os ) const;

private:
	RecordType record_type_;
	Shape<grid> shape_;

	size_t hc_;
	size_t hh_;

	// For non-tiles, possible patches exhibiting the shape's Heesch number
	patch_t hc_patch_;
	patch_t hh_patch_;
	
	// For periodic, number of transitivity classes
	size_t transitivity_;
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

	if( record_type_ == NONTILER ) {
		bool patch = false;

		auto i = IntReader<size_t> { buf + 1 };
		hc_ = *i++;
		hh_ = *i++;

		if( strchr( buf, 'P' ) ) {
			patch = true;
		}

		if( patch ) {
			if( hc_ > 0 ) {
				is.getline( buf, 1000 );
				size_t sz = atoi( buf );
				for( size_t idx = 0; idx < sz; ++idx ) {
					is.getline( buf, 1000 );
					IntReader<coord_t> i { buf };
					hc_patch_.emplace_back( *i++, 
						xform_t { *i++, *i++, *i++, *i++, *i++, *i++ } );
				}
			} else {
				hc_patch_.emplace_back( 0, xform_t {} );
			}

			if( hh_ != hc_ ) {
				is.getline( buf, 1000 );
				size_t sz = atoi( buf );
				for( size_t idx = 0; idx < sz; ++idx ) {
					is.getline( buf, 1000 );
					IntReader<coord_t> i { buf };
					hh_patch_.emplace_back( *i++,
						xform_t { *i++, *i++, *i++, *i++, *i++, *i++ } );
				}

			}
		}
	} else if( record_type_ == ISOHEDRAL || record_type_ == ANISOHEDRAL ) {
		auto i = IntReader<size_t> { buf + 1 };
		transitivity_ = *i;
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
	os << std::endl;

	if( record_type_ == UNKNOWN ) {
		return;
	}

	switch( record_type_ ) {
		case UNKNOWN: 
			os << '?' << std::endl;
			return;
		case HOLE:
			os << 'O' << std::endl;
			return;
		case INCONCLUSIVE:
			os << '!' << std::endl;
			return;
		case NONTILER:
			os << "~ " << hc_ << ' ' << hh_;
			if( (hc_patch_.size() > 0) || (hh_patch_.size() > 0) ) {
				os << " P";
			}
			os << std::endl;
			break;
		case ISOHEDRAL:
			os << "I " << transitivity_ << std::endl;
			break;
		case ANISOHEDRAL:
			os << "# " << transitivity_ << std::endl;
			break;
		case APERIODIC:
			os << "$" << std::endl;
			break;
	}

	if( record_type_ == NONTILER ) {
		if( hc_patch_.size() > 0 ) {
			os << hc_patch_.size() << std::endl;
			for( const auto& p : hc_patch_ ) {
				os << p.first << ' ' << p.second << std::endl;
			}
		}

		if( hh_patch_.size() > 0 ) { 
			os << hh_patch_.size() << std::endl;
			for( const auto& p : hh_patch_ ) {
				os << p.first << ' ' << p.second << std::endl;
			}
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
