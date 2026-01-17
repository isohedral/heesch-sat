#pragma once

#include <iostream>
#include <type_traits>

#include "geom.h"

// Define some helper classes and functions that permit abstract 
// operations on boundary words.  In an ideal world, a lot of this
// can be done using C++ ranges and views, but some of the features of
// that library aren't quite ready yet, so we'll roll our own for now
// and revisit later.

// Keep things simple for now -- assume that a word is subscriptable
// like a vector.  Don't allow fancier abstractions involving iterators.
template<typename C>
concept word = requires(C seq)
{
	typename C::value_type;	
	{seq.size()} -> std::same_as<size_t>;
	{seq[0]} -> std::convertible_to<typename C::value_type>;
};

// Derived from Langerman + Winslow: longest matching prefixes, up to a
// predefined upper bound.
inline size_t longestMatch(const word auto& s1, const word auto& s2, size_t ub)
{
	ub = std::min({s1.size(), s2.size(), ub});
	size_t idx = 0;
	while (idx < ub && s1[idx] == s2[idx]) {
		++idx;
	}
	return idx;
}

template<typename C>
struct reverse_op {
	using value_type = typename C::value_type;

	reverse_op(const C& w)
		: w_ {w}
	{}

	size_t size() const 
	{return w_.size();}
	value_type operator [](size_t idx) const 
	{return w_[w_.size() - 1 - idx];}

	const C& w_;
};

template<typename C>
struct backtrack_op {
	using value_type = typename C::value_type;

	backtrack_op(const C& w)
		: w_ {w}
	{}

	size_t size() const 
	{return w_.size();}
	value_type operator [](size_t idx) const 
	{return -w_[w_.size() - 1 - idx];}

	const C& w_;
};

template<typename C>
struct transform_op {
	using value_type = typename C::value_type;

	transform_op(const C& w, const xform<int8_t>& T)
		: w_ {w}
		, T_ {T}
	{}

	size_t size() const 
	{return w_.size();}
	value_type operator [](size_t idx) const 
	{return T_ * w_[idx];}

	const C& w_;
	const xform<int8_t>& T_;
};

template<typename C>
struct slice_op {
	using value_type = typename C::value_type;

	slice_op(const C& w, size_t from, size_t to = SIZE_MAX)
		: w_ {w}
		, from_ {from}
		, size_ {std::min(to, w.size()) - from_}
	{}

	size_t size() const 
	{return size_;}
	value_type operator [](size_t idx) const 
	{return w_[from_ + idx];}

	const C& w_;
	size_t from_;
	size_t size_;
};

template<typename C1, typename C2>
struct concat_op {
	using value_type = typename C1::value_type;

	concat_op(const C1& w1, const C2& w2)
		: w1_ {w1}
		, w2_ {w2}
		, size_ {w1.size() + w2.size()}
	{}

	size_t size() const 
	{return size_;}

	value_type operator [](size_t idx) const 
	{
		if (idx >= w1_.size()) {
			return w2_[idx - w1_.size()];
		} else {
			return w1_[idx];
		}
	}

	const C1& w1_;
	const C2& w2_;
	size_t size_;
};

inline auto operator !(const word auto& w)
{
	return reverse_op {w};
}

inline auto operator ~(const word auto& w)
{
	return backtrack_op {w};
}

inline auto operator *(const word auto& w, const xform<int8_t>& T)
{
	return transform_op {w, T};
}

inline auto slice(const word auto& a, size_t from, size_t to = SIZE_MAX)
{
	return slice_op {a, from, to};
}

inline auto operator +(const word auto& a, const word auto& b)
{
	return concat_op {a, b};
}

inline std::ostream& operator <<(std::ostream& os, const word auto& w)
{
	os << '[';
	for (size_t idx = 0; idx < w.size(); ++idx) {
		if (idx > 0) {
			os << ' ';
		}
		os << w[idx];
	}
	return os << ']';
}
