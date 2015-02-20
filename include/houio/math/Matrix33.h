/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once

#include "Vec3.h"

namespace math
{
	//
	//
	// memory storage is row major (which is native to the c++ arrays)
    //     | 0  1  2 |
    // M = | 3  4  5 |
	//     | 6  7  8 |
	//
	// index the matrix with matrix.m[row][column] -> _m12 means: first row, second column
	//
	//           m11 m12 m13
	//           m21 m22 m23
	//           m31 m32 m33
	//
	template<typename T>
	struct Matrix33
	{
        Matrix33();
		~Matrix33();
		Matrix33( const T &_11, const T &_12, const T &_13,
			       const T &_21, const T &_22, const T &_23,
				   const T &_31, const T &_32, const T &_33);

		// convience matrix creation functions
		static Matrix33<T>                                                                 Zero( void ); // returns the zeromatrix
		static Matrix33<T>                                                             Identity( void ); // returns the identitymatrix
		static Matrix33<T>                        RotationMatrix( const Vec3<T> &axis, const T &angle );  // returns a matrix with a transformation that rotates around a certain axis which starts at the origin

		// public methods
		void                                                                          transpose( void );
		Matrix33<T>                                                                  transposed( void );
		void                                                                             invert( void );

		T                                                                              getDeterminant(); // computes and returns the determinant
		T                                                                                       trace(); //returns sum of elements in main diagonal

		// operators
		bool                                                       operator==( const Matrix33<T> &rhs );
		bool                                                       operator!=( const Matrix33<T> &rhs );
		
		bool                                                       operator+=( const Matrix33<T> &rhs );
		bool                                                       operator-=( const Matrix33<T> &rhs );

		bool                                                                 operator+=( const T &rhs );
		bool                                                                 operator-=( const T &rhs );
		bool                                                                 operator*=( const T &rhs );
		bool                                                                 operator/=( const T &rhs );

		union
		{
			struct
			{
				T _11, _12, _13;
				T _21, _22, _23;
				T _31, _32, _33;
			};
			T m[3][3];
			T ma[9];
		};
	};


	
	template<typename T>
	Matrix33<T>::Matrix33()
	{
		_11=_12=_13=
		_21=_22=_23=
		_31=_32=_33=(T)0.0;
	}

	template<typename T>
	Matrix33<T>::Matrix33( const T &_11, const T &_12, const T &_13,
                          const T &_21, const T &_22, const T &_23,
						  const T &_31, const T &_32, const T &_33)
	{
        this->_11=_11; this->_12=_12; this->_13=_13;
        this->_21=_21; this->_22=_22; this->_23=_23;
		this->_31=_31; this->_32=_32; this->_33=_33;
	}

	template<typename T>
	Matrix33<T>::~Matrix33()
	{
	}

	// returns the zeromatrix
	template<typename T>
	Matrix33<T> Matrix33<T>::Zero( void )
	{
		return Matrix33<T>( (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0);
	}

	// returns the identitymatrix
	template<typename T>
	Matrix33<T> Matrix33<T>::Identity( void )
	{
		return Matrix33<T>( (T)1.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)1.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)1.0);
	}


	//
	// returns a matrix with a transformation that rotates around a certain axis which starts at the origin
	//
	// code from Graphics Gems (Glassner, Academic Press, 1990)
	//
	template<typename T>
	Matrix33<T> Matrix33<T>::RotationMatrix( const Vec3<T> &axis, const T &angle )
	{
		T c = cos( angle );
		T t = (T)1.0 - c;
		T s = sin( angle );
		T txy = t*axis.x*axis.y;
		T txz = t*axis.x*axis.z;
		T tyz = t*axis.y*axis.z;
		T sx = s*axis.x;
		T sy = s*axis.y;
		T sz = s*axis.z;
		return Matrix33<T>( t*axis.x*axis.x+c, txy+sz, txz-sy,
			              txy-sz, t*axis.y*axis.y+c, tyz+sx,
						  txz+sy, tyz-sx, t*axis.z*axis.z+c);
	}

	template<typename T>
	void Matrix33<T>::transpose( void )
	{
		Matrix33<T> temp = *this;

		_11 = temp._11;
		_12 = temp._21;
		_13 = temp._31;

		_21 = temp._12;
		_22 = temp._22;
		_23 = temp._32;

		_31 = temp._13;
		_32 = temp._23;
		_33 = temp._33;
	}

	template<typename T>
	Matrix33<T> Matrix33<T>::transposed( void )
	{
		Matrix33<T> m = *this;
		m.transpose();
		return m;
	}

	// computes the determinant of a 2x2 matrix
	//
	template<typename T>
	inline T Det( T &_11, T &_12,
					  T &_21, T &_22 )
	{
		return _11*_22 - _12*_21;
	}

	//returns sum of elements in main diagonal
	template<typename T>
	T Matrix33<T>::trace()
	{
		return _11+_22+_33;
	}

	// computes and returns the determinant
	template<typename T>
	T Matrix33<T>::getDeterminant()
	{
		// rule of sarrus
		return _11*_22*_33 + _21*_32*_13 + _31*_12*_23 - _13*_22*_31 - _23*_32*_11 - _33*_12*_21;
	}



	template<typename T>
	void Matrix33<T>::invert( void )
	{
		T det =	_11*Det(_22, _23,
							_32, _33 ) -
					_12*Det(_21, _23,
							_31, _33 ) +
					_13*Det(_21, _22,
							_31, _32 );
		// determinant must be not zero
		if( fabsf( det ) < (T)0.00001 )
			// error determinant is zero
			return;

		det = (T)1.0 / det;

		Matrix33<T> mMatrix;

		// Row1
		mMatrix._11 = Det( _22, _23,
						   _32, _33 );
		mMatrix._12 = -Det( _12, _13,
						   _32, _33 );
		mMatrix._13 = Det( _12, _13,
						   _22, _23 );
		// Row2
		mMatrix._21 = -Det( _21, _23,
						   _31, _33 );
		mMatrix._22 = Det( _11, _13,
						   _31, _33 );
		mMatrix._23 = -Det( _11, _13,
						   _21, _23 );
		// Row2
		mMatrix._31 = Det( _21, _22,
						   _31, _32 );
		mMatrix._32 = -Det( _11, _12,
						   _31, _32 );
		mMatrix._33 = Det( _11, _12,
						   _21, _22 );

		_11 = det*mMatrix._11;
		_12 = det*mMatrix._12;
		_13 = det*mMatrix._13;

		_21 = det*mMatrix._21;
		_22 = det*mMatrix._22;
		_23 = det*mMatrix._23;

		_31 = det*mMatrix._31;
		_32 = det*mMatrix._32;
		_33 = det*mMatrix._33;
	}

	template<typename T>
	bool Matrix33<T>::operator==( const Matrix33<T> &rhs )
	{
		if( _11==rhs._11 && _12==rhs._12 && _13==rhs._13 &&
			_21==rhs._21 && _22==rhs._22 && _23==rhs._23 &&
			_31==rhs._31 && _32==rhs._32 && _33==rhs._33 )
			return true;
		else
			return false; 
	}

	template<typename T>
	bool Matrix33<T>::operator!=( const Matrix33<T> &rhs )
	{
		return !((*this)==rhs);
	}

	template<typename T>
	bool Matrix33<T>::operator+=( const Matrix33<T> &rhs )
	{
		_11+=rhs._11;
		_12+=rhs._12;
		_13+=rhs._13;

		_21+=rhs._21;
		_22+=rhs._22;
		_23+=rhs._23;

		_31+=rhs._31;
		_32+=rhs._32;
		_33+=rhs._33;

		return true;
	}

	template<typename T>
	bool Matrix33<T>::operator-=( const Matrix33<T> &rhs )
	{
		_11-=rhs._11;
		_12-=rhs._12;
		_13-=rhs._13;

		_21-=rhs._21;
		_22-=rhs._22;
		_23-=rhs._23;

		_31-=rhs._31;
		_32-=rhs._32;
		_33-=rhs._33;

		return true;
	}

	template<typename T>
	bool Matrix33<T>::operator+=( const T &rhs )
	{
		_11+=rhs;
		_12+=rhs;
		_13+=rhs;

		_21+=rhs;
		_22+=rhs;
		_23+=rhs;

		_31+=rhs;
		_32+=rhs;
		_33+=rhs;
		return true;
	}

	template<typename T>
	bool Matrix33<T>::operator-=( const T &rhs )
	{
		_11-=rhs;
		_12-=rhs;
		_13-=rhs;

		_21-=rhs;
		_22-=rhs;
		_23-=rhs;

		_31-=rhs;
		_32-=rhs;
		_33-=rhs;
		return true;
	}

	template<typename T>
	bool Matrix33<T>::operator*=( const T &rhs )
	{
		_11*=rhs;
		_12*=rhs;
		_13*=rhs;

		_21*=rhs;
		_22*=rhs;
		_23*=rhs;

		_31*=rhs;
		_32*=rhs;
		_33*=rhs;
		return true;
	}


	template<typename T>
	bool Matrix33<T>::operator/=( const T &rhs )
	{
		_11/=rhs;
		_12/=rhs;
		_13/=rhs;

		_21/=rhs;
		_22/=rhs;
		_23/=rhs;

		_31/=rhs;
		_32/=rhs;
		_33/=rhs;
		return true;
	}






	typedef Matrix33<float> Matrix33f;
	typedef Matrix33<float> M33f;
	typedef Matrix33<double> Matrix33d;
	typedef Matrix33<double> M33d;
}
