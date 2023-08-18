#pragma once

#include <unordered_set>
#include <unordered_map>

#include <boost/functional/hash.hpp>

#include "common.h"

template<typename coord>
class point
{
public:
	point() 
		: x_ {}, y_ {}
	{}
	template<typename ocoord>
	point( const point<ocoord>& other ) 
		: x_ { static_cast<coord>( other.x_ ) }
		, y_ { static_cast<coord>( other.y_ ) }
	{}
	point( coord x, coord y ) 
		: x_ { x }, y_ { y }
	{}

	coord getX() const { return x_; }
	coord getY() const { return y_; }

	template<typename ocoord>
	bool operator ==( const point<ocoord>& other ) const
	{
		return (x_ == other.x_) && (y_ == other.y_);
	}

	template<typename ocoord>
	bool operator !=( const point<ocoord>& other ) const
	{
		return (x_ != other.x_) || (y_ != other.y_);
	}

	template<typename ocoord>
	bool operator <( const point<ocoord>& other ) const
	{
		return (y_ < other.y_) || ((y_ == other.y_) && (x_ < other.x_));
	}

	template<typename ocoord>
	bool operator <=( const point<ocoord>& other ) const
	{
		return (y_ < other.y_) || ((y_ == other.y_) && (x_ <= other.x_));
	}

	template<typename ocoord>
	point<coord> operator +( const point<ocoord>& other ) const
	{
		return { coord( x_ + other.x_ ), coord( y_ + other.y_ ) };
	}

	template<typename ocoord>
	point<coord> operator -( const point<ocoord>& other ) const
	{
		return { coord( x_ - other.x_ ), coord( y_ - other.y_ ) };
	}

	point<coord> operator -() const
	{
		return { coord( -x_ ), coord( -y_ ) };
	}

	template<typename ocoord>
	point<coord>& operator =( const point<ocoord>& other )
	{
		x_ = other.x_;
		y_ = other.y_;
		return *this;
	}

	template<typename ocoord>
	point<coord>& operator +=( const point<ocoord>& other )
	{
		x_ += other.x_;
		y_ += other.y_;
		return *this;
	}

	template<typename ocoord>
	point<coord>& operator -=( const point<ocoord>& other ) 
	{
		x_ -= other.x_;
		y_ -= other.y_;
		return *this;
	}

	size_t hash() const 
	{
		size_t res = 0;
		boost::hash_combine( res, x_ );
		boost::hash_combine( res, y_ );
		return res;
	}

	coord x_;
	coord y_;
};

template<typename coord>
inline std::ostream& operator <<( std::ostream& os, const point<coord>& p )
{
	return os << '<' << int(p.x_) << ',' << int(p.y_) << '>';
}

template<typename coord>
class xform
{
public:
	xform()
		: a_ { 1 } , b_ { 0 } , c_ { 0 }
		, d_ { 0 } , e_ { 1 } , f_ { 0 }
	{}
	xform( coord a, coord b, coord c, coord d, coord e, coord f )
		: a_ { a } , b_ { b } , c_ { c }
		, d_ { d } , e_ { e } , f_ { f }
	{}
	template<typename ocoord>
	xform( const xform<ocoord>& other )
		: a_ { static_cast<coord>( other.a_ ) } 
		, b_ { static_cast<coord>( other.b_ ) } 
		, c_ { static_cast<coord>( other.c_ ) }
		, d_ { static_cast<coord>( other.d_ ) } 
		, e_ { static_cast<coord>( other.e_ ) } 
		, f_ { static_cast<coord>( other.f_ ) }
	{}

	template<typename ocoord>
	xform<coord>& operator =( const xform<ocoord>& other )
	{
		a_ = other.a_;
		b_ = other.b_;
		c_ = other.c_;
		d_ = other.d_;
		e_ = other.e_;
		f_ = other.f_;
		return *this;
	}

	template<typename ocoord>
	point<coord> operator *( const point<ocoord>& p ) const
	{
		return { coord( a_ * p.x_ + b_ * p.y_ + c_ ),
				 coord( d_ * p.x_ + e_ * p.y_ + f_ ) };
	}
	template<typename ocoord>
	xform<coord> operator *( const xform<ocoord>& other ) const
	{
		return {
			coord( a_*other.a_ + b_*other.d_ ),
			coord( a_*other.b_ + b_*other.e_ ),
			coord( a_*other.c_ + b_*other.f_ + c_ ),

			coord( d_*other.a_ + e_*other.d_ ),
			coord( d_*other.b_ + e_*other.e_ ),
			coord( d_*other.c_ + e_*other.f_ + f_ ) };
	}
	template<typename ocoord>
	xform<coord> translate( const point<ocoord>& pt ) const
	{
		return { a_, b_, coord( c_ + pt.x_ ), d_, e_, coord( f_ + pt.y_ ) };
	}

	xform<coord> invert() const
	{
		// Must be +-1, no need to take reciprocal
		coord det( a_ * e_ - b_ * d_ );

		return { 
			coord( e_ * det ), 
			coord( -b_ * det ),
			coord( (b_ * f_ - c_ * e_) * det ),

			coord( -d_ * det ), 
			coord( a_ * det ),
			coord( (c_ * d_ - a_ * f_) * det ) };
	}

	template<typename ocoord>
	xform<coord>& operator +=( const point<ocoord>& pt ) 
	{
		c_ += pt.x_;
		f_ += pt.y_;
		return *this;
	}

	template<typename ocoord>
	bool operator ==( const xform<ocoord>& other ) const
	{
		return (a_ == other.a_) 
			&& (b_ == other.b_) 
			&& (c_ == other.c_) 
			&& (d_ == other.d_) 
			&& (e_ == other.e_) 
			&& (f_ == other.f_);
	}
	template<typename ocoord>
	bool operator !=( const xform<ocoord>& other ) const
	{
		return !(*this == other);
	}

	bool isIdentity() const
	{
		return (a_==1) && (b_==0) && (c_==0) && (d_==0) && (e_==1) && (f_==0);
	}
	bool isTranslation() const
	{
		return (a_==1) && (b_==0) && (d_==0) && (e_==1);
	}
	bool isHalfturn() const
	{
		return (a_==-1) && (b_==0) && (d_==0) && (e_==-1);
	}
	coord det() const
	{
		return a_*e_ - b_*d_;
	}

	size_t hash() const
	{
		size_t res = 0;
		boost::hash_combine( res, a_ );
		boost::hash_combine( res, b_ );
		boost::hash_combine( res, c_ );
		boost::hash_combine( res, d_ );
		boost::hash_combine( res, e_ );
		boost::hash_combine( res, f_ );
		return res;
	}

	coord a_;
	coord b_;
	coord c_;
	coord d_;
	coord e_;
	coord f_;
};

template<typename coord>
inline std::ostream& operator <<( std::ostream& os, const xform<coord>& T )
{
	return os << '<' 
		<< int(T.a_) << ',' 
		<< int(T.b_) << ','
		<< int(T.c_) << ',' 
		<< int(T.d_) << ','
		<< int(T.e_) << ',' 
		<< int(T.f_) << '>';
}

template<typename coord>
using point_set = std::unordered_set<point<coord>,method_hash<point<coord>>>;
template<typename coord, typename T> 
using point_map = std::unordered_map<point<coord>,T,method_hash<point<coord>>>;

template<typename coord>
using xform_set = std::unordered_set<xform<coord>,method_hash<xform<coord>>>;
template<typename coord, typename T> 
using xform_map = std::unordered_map<xform<coord>,T,method_hash<xform<coord>>>;
