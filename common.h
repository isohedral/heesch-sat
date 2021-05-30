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
