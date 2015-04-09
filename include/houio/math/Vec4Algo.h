/*---------------------------------------------------------------------

Vec4 related routines

----------------------------------------------------------------------*/
#pragma once

#include <math.h>
#include "Vec4.h"

namespace houio
{


namespace math
{

	template<typename T>
	inline Vec4<T> operator-( const Vec4<T> &rhs )
	{
		return Vec4<T>( -rhs.x, -rhs.y, -rhs.z );
	}

	template<typename T>
	inline Vec4<T> operator+( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w );
	}

	template<typename T>
	inline Vec4<T> operator-( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z );
	}

	template<typename T>
	inline Vec4<T> operator/( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs.x/rhs.x, lhs.y/rhs.y, lhs.z/rhs.z );
	}		

	template<typename T>
	inline Vec4<T> operator+( const Vec4<T> &lhs, const T &rhs )
	{
		return Vec4<T>( lhs.x+rhs, lhs.y+rhs, lhs.z+rhs );
	}

	template<typename T>
	inline Vec4<T> operator-( const Vec4<T> &lhs, const T &rhs )
	{
		return Vec4<T>( lhs.x-rhs, lhs.y-rhs, lhs.z-rhs );
	}

	template<typename T>
	inline Vec4<T> operator*( const Vec4<T> &lhs, const T &rhs )
	{
		return Vec4<T>( lhs.x*rhs, lhs.y*rhs, lhs.z*rhs, lhs.w*rhs );
	}

	template<typename T>
	inline Vec4<T> operator/( const Vec4<T> &lhs, const T &rhs )
	{
		return Vec4<T>( lhs.x/rhs, lhs.y/rhs, lhs.z/rhs );
	}

	template<typename T>
	inline Vec4<T> operator+( const T &lhs, const Vec4<T> &rhs )
	{
		return (rhs+lhs);
	}

	template<typename T>
	inline Vec4<T> operator-( const T &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs-rhs.x, lhs-rhs.y, lhs-rhs.z );
	}

	template<typename T>
	inline Vec4<T> operator*( const T &lhs, const Vec4<T> &rhs )
	{
		return (rhs*lhs);
	}

	template<typename T>
	inline Vec4<T> operator*( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>(rhs.x*lhs.x, rhs.y*lhs.y, rhs.z*lhs.z);
	}

	template<typename T>
	inline Vec4<T> operator/( const T &lhs, const Vec4<T> &rhs )
	{
		return (rhs/lhs);
	}

	template<typename T>
	inline Vec4<T> normalize( const Vec4<T> &vector )
	{
		Vec4<T> result = vector;
		result.normalize();
		return result;
	}

	template<typename T>
	inline T dotProduct( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);
	}

	template<typename T>
	inline T dot( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);
	}

	template<typename T>
	inline void dotProduct( T &result, const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		result = (lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);
	}


	template<typename T>
	inline Vec4<T> crossProduct( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs.y*rhs.z - lhs.z*rhs.y,
					  lhs.z*rhs.x - lhs.x*rhs.z,
					  lhs.x*rhs.y - lhs.y*rhs.x );
	}

	template<typename T>
	inline void crossProduct( Vec4<T> &result, const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		result.x = lhs.y*rhs.z - lhs.z*rhs.y;
		result.y = lhs.z*rhs.x - lhs.x*rhs.z;
		result.z = lhs.x*rhs.y - lhs.y*rhs.x;
	}


	template<typename T>
	inline Vec4<T> cross( const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		return Vec4<T>( lhs.y*rhs.z - lhs.z*rhs.y,
					  lhs.z*rhs.x - lhs.x*rhs.z,
					  lhs.x*rhs.y - lhs.y*rhs.x );
	}

	template<typename T>
	inline void cross( Vec4<T> &result, const Vec4<T> &lhs, const Vec4<T> &rhs )
	{
		result.x = lhs.y*rhs.z - lhs.z*rhs.y;
		result.y = lhs.z*rhs.x - lhs.x*rhs.z;
		result.z = lhs.x*rhs.y - lhs.y*rhs.x;
	}

	// For a given incident vector I and surface normal N reflect returns the reflection direction calculated as I - 2.0 * dot(N, I) * N. N should be normalized in order to achieve the desired result.
	template<typename T>
	inline Vec4<T> reflect( const math::Vec4<T> &i, const math::Vec4<T> &n )
	{
		return -i + 2.0f*dotProduct(n,i)*n;
	}


	// returns index of the component with the smallest value
	template<typename T>
	int nondominantAxis( const math::Vec4<T> &v )
	{
		if( abs(v.x) <= abs(v.y) )
		{
			if( abs(v.z) <= abs(v.x) )
				return 2;
			else
				return 0;
		}else
		{
			if( abs(v.z) <= abs(v.y) )
				return 2;
			else
				return 1;
		}
	}

	template<typename T>
	Vec4<T> baseVec4( int i )
	{
		switch(i)
		{
		case 0:return Vec4<T>((T)(1.0), (T)(0.0), (T)(0.0));
		case 1:return Vec4<T>((T)(0.0), (T)(1.0), (T)(0.0));
		default:
		case 2:return Vec4<T>((T)(0.0), (T)(0.0), (T)(1.0));
		};
	}

}

} // namespace houio
