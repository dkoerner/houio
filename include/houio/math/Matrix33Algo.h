#pragma once

#include "Matrix33.h"


namespace math
{
	
	// global Matrix33<T> related ops
	template<typename T>
	inline void matrixMultiply( Matrix33<T> &result, const Matrix33<T> &lhs, const Matrix33<T> &rhs )
	{
		result._11 = lhs._11*rhs._11 + lhs._12*rhs._21 + lhs._13*rhs._31;
		result._12 = lhs._11*rhs._12 + lhs._12*rhs._22 + lhs._13*rhs._32;
		result._13 = lhs._11*rhs._13 + lhs._12*rhs._23 + lhs._13*rhs._33;

		result._21 = lhs._21*rhs._11 + lhs._22*rhs._21 + lhs._23*rhs._31;
		result._22 = lhs._21*rhs._12 + lhs._22*rhs._22 + lhs._23*rhs._32;
		result._23 = lhs._21*rhs._13 + lhs._22*rhs._23 + lhs._23*rhs._33;

		result._31 = lhs._31*rhs._11 + lhs._32*rhs._21 + lhs._33*rhs._31;
		result._32 = lhs._31*rhs._12 + lhs._32*rhs._22 + lhs._33*rhs._32;
		result._33 = lhs._31*rhs._13 + lhs._32*rhs._23 + lhs._33*rhs._33;
	}

	template<typename T>
	inline Matrix33<T> operator/( const Matrix33<T> &lhs, const T &rhs )
	{
		Matrix33<T> result;
		result = lhs;

		result._11 /= rhs;
		result._12 /= rhs;
		result._13 /= rhs;

		result._21 /= rhs;
		result._22 /= rhs;
		result._23 /= rhs;

		result._31 /= rhs;
		result._32 /= rhs;
		result._33 /= rhs;

		return result;
	}

	template<typename T>
	inline Matrix33<T> operator*( const Matrix33<T> &lhs, const T &rhs )
	{
		Matrix33<T> result;
		result = lhs;

		result._11 *= rhs;
		result._12 *= rhs;
		result._13 *= rhs;

		result._21 *= rhs;
		result._22 *= rhs;
		result._23 *= rhs;

		result._31 *= rhs;
		result._32 *= rhs;
		result._33 *= rhs;

		return result;
	}

	template<typename T>
	inline Matrix33<T> operator-( const Matrix33<T> &lhs, const T &rhs )
	{
		Matrix33<T> result;
		result = lhs;

		result._11 -= rhs;
		result._12 -= rhs;
		result._13 -= rhs;

		result._21 -= rhs;
		result._22 -= rhs;
		result._23 -= rhs;

		result._31 -= rhs;
		result._32 -= rhs;
		result._33 -= rhs;

		return result;
	}

	template<typename T>
	inline Matrix33<T> operator+( const Matrix33<T> &lhs, const T &rhs )
	{
		Matrix33<T> result;
		result = lhs;

		result._11 += rhs;
		result._12 += rhs;
		result._13 += rhs;

		result._21 += rhs;
		result._22 += rhs;
		result._23 += rhs;

		result._31 += rhs;
		result._32 += rhs;
		result._33 += rhs;

		return result;
	}

	template<typename T>
	inline Matrix33<T> operator-( const Matrix33<T> &rhs )
	{
		return Matrix33<T>( -rhs._11, -rhs._12, -rhs._13,
						  -rhs._21, -rhs._22, -rhs._23,
						  -rhs._31, -rhs._32, -rhs._33);
	}

	template<typename T>
	inline Matrix33<T> operator-( const Matrix33<T> &lhs, const Matrix33<T> &rhs )
	{
		return Matrix33<T>( lhs._11-rhs._11, lhs._12-rhs._12, lhs._13-rhs._13,
						  lhs._21-rhs._21, lhs._22-rhs._22, lhs._23-rhs._23,
						  lhs._31-rhs._31, lhs._32-rhs._32, lhs._33-rhs._33);
	}

	template<typename T>
	inline Matrix33<T> operator+( const Matrix33<T> &lhs, const Matrix33<T> &rhs )
	{
		return Matrix33<T>( lhs._11+rhs._11, lhs._12+rhs._12, lhs._13+rhs._13,
						  lhs._21+rhs._21, lhs._22+rhs._22, lhs._23+rhs._23,
						  lhs._31+rhs._31, lhs._32+rhs._32, lhs._33+rhs._33);
	}

	template<typename T>
	inline Matrix33<T> operator*( const T &lhs, const Matrix33<T> &rhs )
	{
		return rhs * lhs;
	}

	template<typename T>
	inline Matrix33<T> operator/( const T &lhs, const Matrix33<T> &rhs )
	{
		return rhs / lhs;
	}

	template<typename T>
	inline Matrix33<T> operator-( const T &lhs, const Matrix33<T> &rhs )
	{
		return rhs - lhs;
	}

	template<typename T>
	inline Matrix33<T> operator+( const T &lhs, const Matrix33<T> &rhs )
	{
		return rhs + lhs;
	}

	template<typename T>
	inline Matrix33<T> operator*( const Matrix33<T> &lhs, const Matrix33<T> &rhs )
	{
		Matrix33<T> result;
		matrixMultiply( result, lhs, rhs );
		return result;
	}

	template<typename T>
	inline Vec3<T> transform( const Vec3<T> &lhs, const Matrix33<T> &rhs )
	{
		return Vec3<T>(lhs.x*rhs._11 + lhs.y*rhs._21 + lhs.z*rhs._31,
					 lhs.x*rhs._12 + lhs.y*rhs._22 + lhs.z*rhs._32,
					 lhs.x*rhs._13 + lhs.y*rhs._23 + lhs.z*rhs._33);
	}

	template<typename T>
	inline Matrix33<T> transpose( const Matrix33<T> &lhs )
	{
		Matrix33<T> result = lhs;
		result.transpose();
		return result;
	}

	template<typename T>
	inline T frobeniusNorm( const Matrix33<T> &lhs )
	{
		T result = (T)0.0;

		result += lhs._11*lhs._11;
		result += lhs._12*lhs._12;
		result += lhs._13*lhs._13;

		result += lhs._21*lhs._21;
		result += lhs._22*lhs._22;
		result += lhs._23*lhs._23;

		result += lhs._31*lhs._31;
		result += lhs._32*lhs._32;
		result += lhs._33*lhs._33;

		return sqrt(result);
	}


	template<typename T>
	inline math::Matrix33<T> outer( const math::Vec3<T> &lhs, const math::Vec3<T> &rhs )
	{
		math::Matrix33<T> result;

		result._11 = lhs.x * rhs.x;
		result._12 = lhs.x * rhs.y;
		result._13 = lhs.x * rhs.z;
		result._21 = lhs.y * rhs.x;
		result._22 = lhs.y * rhs.y;
		result._23 = lhs.y * rhs.z;
		result._31 = lhs.z * rhs.x;
		result._32 = lhs.z * rhs.y;
		result._33 = lhs.z * rhs.z;

		return result;
	}

}