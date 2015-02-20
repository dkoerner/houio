/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once

#include "Vec2.h"
#include "Vec2Algo.h"


namespace math
{


	template<typename T>
	inline Vec2<T> operator-( const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		return Vec2<T>( lhs.x-rhs.x, lhs.y-rhs.y );
	}

	template<typename T>
	inline Vec2<T> operator+( const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		return Vec2<T>( lhs.x+rhs.x, lhs.y+rhs.y );
	}

	template<typename T>
	inline Vec2<T> operator*( const Vec2<T> &lhs, const float &rhs )
	{
		return Vec2<T>( lhs.x*rhs, lhs.y*rhs );
	}

	template<typename T>
	inline Vec2<T> operator*( const T &lhs, const Vec2<T> &rhs )
	{
		return (rhs*lhs);
	}

	template<typename T>
	inline Vec2<T> operator*( const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		return Vec2<T>( lhs.x*rhs.x, lhs.y*rhs.y );
	}

	template<typename T>
	inline Vec2<T> operator-( const Vec2<T> &rhs )
	{
		return Vec2<T>( -rhs.x, -rhs.y );
	}

	template<typename T>
	inline float crossProduct( const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		return lhs.x*rhs.y - lhs.y*rhs.x;
	}

	template<typename T>
	inline void crossProduct( float &result, const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		result = lhs.x*rhs.y - lhs.y*rhs.x;
	}



}
