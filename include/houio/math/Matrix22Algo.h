/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once

#include "Matrix22.h"
#include "Vec2.h"

namespace math
{

	template<typename T>
	inline Vec2<T> transform( const Vec2<T> &lhs, const Matrix22<T> &rhs )
	{
		return Vec2<T>(lhs.x*rhs._11 + lhs.y*rhs._21,
					 lhs.x*rhs._12 + lhs.y*rhs._22 );
	}

	template<typename T>
	inline Matrix22<T> outer( const Vec2<T> &lhs, const Vec2<T> &rhs )
	{
		Matrix22<T> result;

		result._11 = lhs.x * rhs.x;
		result._12 = lhs.y * rhs.x;

		result._21 = lhs.x * rhs.y;
		result._22 = lhs.y * rhs.y;

		result.transpose();

		return result;
	}

	template<typename T>
	inline Matrix22<T> operator+( const Matrix22<T> &lhs, const Matrix22<T> &rhs )
	{
		return Matrix22<T>(   lhs._11+rhs._11, lhs._12+rhs._12,
							lhs._21+rhs._21, lhs._22+rhs._22 );
	}

	template<typename T>
	inline Matrix22<T> operator-( const Matrix22<T> &lhs, const Matrix22<T> &rhs )
	{
		return Matrix22<T>(   lhs._11-rhs._11, lhs._12-rhs._12,
							lhs._21-rhs._21, lhs._22-rhs._22 );
	}

	template<typename T>	
	inline void matrixMultiply( Matrix22<T> &result, const Matrix22<T> &lhs, const Matrix22<T> &rhs )
	{
		result._11 = lhs._11*rhs._11 + lhs._12*rhs._21;
		result._12 = lhs._11*rhs._12 + lhs._12*rhs._22;

		result._21 = lhs._21*rhs._11 + lhs._22*rhs._21;
		result._22 = lhs._21*rhs._12 + lhs._22*rhs._22;
	}

	template<typename T>
	inline Matrix22<T> operator/( const Matrix22<T> &lhs, const T &rhs )
	{
		Matrix22<T> result;
		result = lhs;

		result._11 /= rhs;
		result._12 /= rhs;

		result._21 /= rhs;
		result._22 /= rhs;

		return result;
	}

	template<typename T>
	inline Matrix22<T> operator*( const Matrix22<T> &lhs, const T &rhs )
	{
		Matrix22<T> result;
		result = lhs;

		result._11 *= rhs;
		result._12 *= rhs;

		result._21 *= rhs;
		result._22 *= rhs;

		return result;
	}

	template<typename T>
	inline Matrix22<T> operator-( const Matrix22<T> &rhs )
	{
		return Matrix22<T>( -rhs._11, -rhs._12,
						  -rhs._21, -rhs._22);
	}

	template<typename T>
	inline Matrix22<T> operator*( const T &lhs, const Matrix22<T> &rhs )
	{
		return rhs * lhs;
	}

	template<typename T>
	inline Matrix22<T> operator/( const T &lhs, const Matrix22<T> &rhs )
	{
		return rhs / lhs;
	}

	template<typename T>
	inline Matrix22<T> operator*( const Matrix22<T> &lhs, const Matrix22<T> &rhs )
	{
		Matrix22<T> result;
		matrixMultiply( result, lhs, rhs );
		return result;
	}

	template<typename T>
	inline Matrix22<T> transpose( const Matrix22<T> &lhs )
	{
		Matrix22<T> result = lhs;
		result.transpose();
		return result;
	}

	template<typename T>
	inline T frobeniusNorm( const Matrix22<T> &lhs )
	{
		T result = (T)0.0;

		result += lhs._11*lhs._11;
		result += lhs._12*lhs._12;

		result += lhs._21*lhs._21;
		result += lhs._22*lhs._22;

		return sqrt(result);
	}

}

