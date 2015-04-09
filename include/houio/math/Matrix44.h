/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once



#include "Vec3.h"

namespace houio
{


namespace math
{
	/// \brief matrix class which is specialized in the usage as transform matrix
	///
	/// Layout of base vectors within matrices
    ///     | Ix  Iy  Iz  0 |
    /// M = | Jx  Jy  Jz  0 |
    ///     | Kx  Ky  Kz  0 |
    ///     | Tx  Ty  Tz  1  |
	/// where I,J and K are the base vectors of |R³
	///
	/// memory storage is row major (which is native to the c++ arrays)
    ///     | 0  1  2  3 |
    /// M = | 4  5  6  7 |
    ///     | 8  9  10 11 |
    ///     | 12 13 14 15 |
	///
	/// index the matrix with matrix.m[row][column] -> _m12 means: first row, second column
	///
	///              m11 m12 m13 m14
	/// (x y z 1)    m21 m22 m23 m24
	///              m31 m32 m33 m34
	///              m41 m42 m43 m44
	///
	/// matrix multiplication with a vector assumes row vectors which are multiplied on the left side like vector*matrix
	/// this is from how we have layed out our base vectors
	///
	/// In terms of computergraphics the matrix is layed out in the spirit of direct x in terms of vector-multiplication
	/// and basevectors and in terms of memory layout.
	/// Since opengl has layed out its base vectors in a transposed fashion but in addition assumes column-major memory
	/// layout, this matrix can be used for opengl as well.
	///
	///
	template<typename T>
	struct Matrix44
	{
        Matrix44();
		~Matrix44();
		Matrix44( const T &_11, const T &_12, const T &_13, const T &_14,
			       const T &_21, const T &_22, const T &_23, const T &_24,
				   const T &_31, const T &_32, const T &_33, const T &_34,
                   const T &_41, const T &_42, const T &_43, const T &_44 );
		Matrix44( const Vec3<T> &right, const Vec3<T> &up, const Vec3<T> &forward, const Vec3<T> &translation = Vec3<T>((T)0.0) );
		template<typename S>
		Matrix44( const Matrix44<S> &other );

		// convenience matrix creation functions
		static Matrix44<T>                                                               Zero( void );  ///< returns the zeromatrix
		static Matrix44<T>                                                           Identity( void );  ///< returns the identitymatrix
		static Matrix44<T>                                          RotationMatrixX( const T &angle );  ///< returns a matrix which defines a rotation around the x axis with the float-specified amount
		static Matrix44<T>                                          RotationMatrixY( const T &angle );  ///< returns a matrix which defines a rotation around the y axis with the float-specified amount
		static Matrix44<T>                                          RotationMatrixZ( const T &angle );  ///< returns a matrix which defines a rotation around the z axis with the float-specified amount
		static Matrix44<T>                      RotationMatrix( const Vec3<T> &axis, const T &angle );  ///< returns a matrix with a transformation that rotates around a certain axis which starts at the origin
		static Matrix44<T>                     RotationMatrix( const Vec3<T> &v1, const Vec3<T> &v2 );  ///< returns a matrix with a transformation that rotates from v1 to v2
		static Matrix44<T>                            TranslationMatrix( const Vec3<T> &translation );  ///< returns a matrix which defines a translation of the specified translation vector
		static Matrix44<T>                    TranslationMatrix( const T &x, const T &y, const T &z );  ///< returns a matrix which defines a translation of the specified translation vector
		static Matrix44<T>                                       ScaleMatrix( const T &uniformScale );  ///< returns a matrix which defines a uniform scale
		static Matrix44<T>                          ScaleMatrix( const T &x, const T &y, const T &z );  ///< returns a matrix which defines a non-uniform scale
		static Matrix44<T>                                      ScaleMatrix( const math::Vec3<T> &s );  ///< returns a matrix which defines a non-uniform scale

		// public methods
		void                                                                        transpose( void );
		Matrix44<T>                                                           transposed( void )const;
		void                                                                           invert( void );
		Matrix44<T>                                                             inverted( void )const;
		Matrix44<T>                                                              inverse( void )const; // same as inverted (here for Imath compatibility)

		Vec3<T>                                         getRight( const bool &normalized=true ) const;
		Vec3<T>                                            getUp( const bool &normalized=true ) const;
		Vec3<T>                                           getDir( const bool &normalized=true ) const;
		Vec3<T>                                                          getTranslation( void ) const;

		Matrix44<T>                                                       getOrientation( void )const;
		Matrix44<T>                                             getNormalizedOrientation( void )const;
		Matrix44<T>                                                        getTransposed( void )const;

		// convinience functions for low level matrix manipulation
		void                                                         setRight( const Vec3<T> &right );
		void                                                               setUp( const Vec3<T> &up );
		void                                                             setDir( const Vec3<T> &dir );
		void                                             setTranslation( const Vec3<T> &translation );

		// convience functions for higher level matrix manipulations
		void                                                                rotateX( const T &angle ); ///< rotates the current transform around the x-axis (angle in radians)
		void                                                                rotateY( const T &angle ); ///< rotates the current transform around the y-axis (angle in radians)
		void                                                                rotateZ( const T &angle ); ///< rotates the current transform around the z-axis (angle in radians)
		Matrix44<T>                                          &translate( const Vec3<T> &translation ); ///< translates the current transform
		void                                          translate( const T &x, const T &y, const T &z ); ///< translates the current transform
		void                                                           scale( const T &uniformScale ); ///< scales the current transform uniformly
		void                                              scale( const T &x, const T &y, const T &z ); ///< scales the current transform non-uniformly
		Matrix44<T>                                                        &scale( const Vec3<T> &s ); ///< scales the current transform non-uniformly
		//void rotate( const Vec3f &origin, const Vec3f &axis, const float &angle ); // will multiply the matrix with a transformation that rotates around a certain axis anywhere in space
		//void                      rotate( const Vec3f &axis, const float &angle ); // will multiply the matrix with a transformation that rotates around a certain axis which starts at the origin

		// operators
		bool                                                     operator==( const Matrix44<T> &rhs );
		bool                                                     operator!=( const Matrix44<T> &rhs );
		
		bool                                                     operator+=( const Matrix44<T> &rhs );
		bool                                                     operator-=( const Matrix44<T> &rhs );

		bool                                                               operator+=( const T &rhs );
		bool                                                               operator-=( const T &rhs );
		bool                                                               operator*=( const T &rhs );
		bool                                                               operator/=( const T &rhs );
		
		union
		{
			struct
			{
				T _11, _12, _13, _14;	
				T _21, _22, _23, _24;	
				T _31, _32, _33, _34;	
				T _41, _42, _43, _44;	
			};
			T m[4][4];
			T ma[16];
		};
	};











	template<typename T>
	template<typename S>
	Matrix44<T>::Matrix44( const Matrix44<S> &other )
	{
        this->_11=(T)other._11; this->_12=(T)other._12; this->_13=(T)other._13; this->_14=(T)other._14;
        this->_21=(T)other._21; this->_22=(T)other._22; this->_23=(T)other._23; this->_24=(T)other._24;
        this->_31=(T)other._31; this->_32=(T)other._32; this->_33=(T)other._33; this->_34=(T)other._34;
        this->_41=(T)other._41; this->_42=(T)other._42; this->_43=(T)other._43; this->_44=(T)other._44;
	}



	template<typename T>
	Matrix44<T>::Matrix44()
	{
		_12=_13=_14=
		_21=_23=_24=
		_31=_32=_34=
		_41=_42=_43=(T)0.0;
		_11=_22=_33=_44=(T)1.0;
	}

	template<typename T>
	Matrix44<T>::Matrix44( const T &_11, const T &_12, const T &_13, const T &_14,
                          const T &_21, const T &_22, const T &_23, const T &_24,
                          const T &_31, const T &_32, const T &_33, const T &_34,
                          const T &_41, const T &_42, const T &_43, const T &_44 )
	{
        this->_11=_11; this->_12=_12; this->_13=_13; this->_14=_14;
        this->_21=_21; this->_22=_22; this->_23=_23; this->_24=_24;
        this->_31=_31; this->_32=_32; this->_33=_33; this->_34=_34;
        this->_41=_41; this->_42=_42; this->_43=_43; this->_44=_44;
	}

	// creates the Matrix from 3 rows
	template<typename T>
	Matrix44<T>::Matrix44( const Vec3<T> &right, const Vec3<T> &up, const Vec3<T> &forward, const Vec3<T> &translation )
	{
		this->_11=right.x; this->_12=right.y; this->_13=right.z; this->_14=(T)0.0;
        this->_21=up.x; this->_22=up.y; this->_23=up.z; this->_24=(T)0.0;
        this->_31=forward.x; this->_32=forward.y; this->_33=forward.z; this->_34=(T)0.0;
		this->_41=(T)translation.x; this->_42=(T)translation.y; this->_43=(T)translation.z; this->_44=(T)1.0;
	}

	template<typename T>
	Matrix44<T>::~Matrix44()
	{
	}

	// returns the zeromatrix
	template<typename T>
	Matrix44<T> Matrix44<T>::Zero( void )
	{
		return Matrix44<T>( (T)0.0, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)0.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)0.0);
	}

	//
	// returns the identitymatrix
	//
	template<typename T>
	Matrix44<T> Matrix44<T>::Identity( void )
	{
		return Matrix44<T>( (T)1.0, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)1.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)1.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	//
	// returns a matrix which defines a rotation about the x axis with the T-specified amount
	//
	template<typename T>
	Matrix44<T> Matrix44<T>::RotationMatrixX( const T &angle )
	{
		T s = sin( angle );
		T c = cos( angle );

		return Matrix44<T>( (T)1.0, (T)0.0, (T)0.0, (T)0.0,
						  (T)0.0, c, -s, (T)0.0,
						  (T)0.0, s, c, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	//
	// returns a matrix which defines a rotation about the y axis with the T-specified amount
	//
	template<typename T>
	Matrix44<T> Matrix44<T>::RotationMatrixY( const T &angle )
	{
		T s = sin( angle );
		T c = cos( angle );

		return Matrix44<T>( c, (T)0.0, s, (T)0.0,
			              (T)0.0, (T)1.0, (T)0.0, (T)0.0,
						  -s, (T)0.0, c, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	//
	// returns a matrix which defines a rotation about the z axis with the T-specified amount
	//
	template<typename T>
	Matrix44<T> Matrix44<T>::RotationMatrixZ( const T &angle )
	{
		T s = sin( angle );
		T c = cos( angle );
		return Matrix44<T>( c, -s, (T)0.0, (T)0.0,
			              s, c, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)1.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	// returns a matrix with a transformation that rotates around
	// a certain axis which starts at the origin
	template<typename T>
	Matrix44<T> Matrix44<T>::RotationMatrix( const Vec3<T> &axis, const T &angle )
	{
		// code from Graphics Gems (Glassner, Academic Press, 1990)
		T c = cos( angle );
		T t = (T)1.0 - c;
		T s = sin( angle );
		T txy = t*axis.x*axis.y;
		T txz = t*axis.x*axis.z;
		T tyz = t*axis.y*axis.z;
		T sx = s*axis.x;
		T sy = s*axis.y;
		T sz = s*axis.z;
		return Matrix44<T>( t*axis.x*axis.x+c, txy+sz, txz-sy, (T)0.0,
			              txy-sz, t*axis.y*axis.y+c, tyz+sx, (T)0.0,
						  txz+sy, tyz-sx, t*axis.z*axis.z+c, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	///< returns a matrix with a transformation that rotates from v1 to v2
	template<typename T>
	Matrix44<T> Matrix44<T>::RotationMatrix( const Vec3<T> &v1, const Vec3<T> &v2 )
	{
		math::Vec3<T> v1n = v1.normalized();
		math::Vec3<T> v2n = v2.normalized();
		// get angle between vectors
		T cosAngle = math::dot( v1.normalized(), v2.normalized() );
		// get rotation axis
		math::Vec3<T> axis = math::cross( v1n, v2n );
		// construct rotation
		return Matrix44<T>::RotationMatrix( axis, acos(cosAngle) );
	}


	// returns a matrix which defines a translation of the specified translation vector
	template<typename T>
	Matrix44<T> Matrix44<T>::TranslationMatrix( const Vec3<T> &translation )
	{
		return Matrix44<T>( (T)1.0, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)1.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)1.0, (T)0.0,
						  translation.x, translation.y, translation.z, (T)1.0);		
	}

	// returns a matrix which defines a translation of the specified translation vector
	template<typename T>
	Matrix44<T> Matrix44<T>::TranslationMatrix( const T &x, const T &y, const T &z )
	{
		return Matrix44<T>( (T)1.0, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, (T)1.0, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, (T)1.0, (T)0.0,
						  x, y, z, (T)1.0);		
	}

	// returns a matrix which defines a uniform scale
	template<typename T>
	Matrix44<T> Matrix44<T>::ScaleMatrix( const T &uniformScale )
	{
		return Matrix44<T>( uniformScale, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, uniformScale, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, uniformScale, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	// returns a matrix which defines a non-uniform scale
	template<typename T>
	Matrix44<T> Matrix44<T>::ScaleMatrix( const T &x, const T &y, const T &z )
	{
		return Matrix44<T>( x, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, y, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, z, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	///< returns a matrix which defines a non-uniform scale
	template<typename T>
	Matrix44<T> Matrix44<T>::ScaleMatrix( const math::Vec3<T> &s )
	{
		return Matrix44<T>( s.x, (T)0.0, (T)0.0, (T)0.0,
			              (T)0.0, s.y, (T)0.0, (T)0.0,
						  (T)0.0, (T)0.0, s.z, (T)0.0,
						  (T)0.0, (T)0.0, (T)0.0, (T)1.0);
	}

	template<typename T>
	void Matrix44<T>::rotateX( const T &angle )
	{
		*this = *this * RotationMatrixX( angle );
	}

	template<typename T>
	void Matrix44<T>::rotateY( const T &angle )
	{
		*this = *this * RotationMatrixY( angle );
	}

	template<typename T>
	void Matrix44<T>::rotateZ( const T &angle )
	{
		*this = *this * RotationMatrixZ( angle );
	}

	template<typename T>
	Matrix44<T> &Matrix44<T>::translate( const Vec3<T> &translation )
	{
		*this = *this * TranslationMatrix( translation );
		return *this;
	}

	template<typename T>
	void Matrix44<T>::translate( const T &x, const T &y, const T &z )
	{
		*this = *this * TranslationMatrix( x, y, z ) ;
	}

	// scales the current transform
	template<typename T>
	void Matrix44<T>::scale( const T &uniformScale )
	{
		*this = *this * ScaleMatrix( uniformScale );
	}

	// scales the current transform non-uniformly
	template<typename T>
	void Matrix44<T>::scale( const T &x, const T &y, const T &z )
	{
		*this = *this * ScaleMatrix( x, y, z );
	}

	///< scales the current transform non-uniformly
	template<typename T>
	Matrix44<T> & Matrix44<T>::scale( const Vec3<T> &s )
	{
		*this = *this * ScaleMatrix( s.x, s.y, s.z );
		return *this;
	}

	template<typename T>
	void Matrix44<T>::transpose( void )
	{
		Matrix44<T> temp = *this;

		_12 = temp._21;
		_13 = temp._31;
		_14 = temp._41;

		_21 = temp._12;
		_23 = temp._32;
		_24 = temp._42;

		_31 = temp._13;
		_32 = temp._23;
		_34 = temp._43;

		_41 = temp._14;
		_42 = temp._24;
		_43 = temp._34;
	}

	template<typename T>
	Matrix44<T> Matrix44<T>::transposed( void )const
	{
		Matrix44<T> m = *this;
		m.transpose();
		return m;
	}

	// computes the determinant of a 3x3 matrix after the rule of Sarrus
	template<typename T>
	inline T Det(	T &f_11, T &f_12, T &f_13,
						T &f_21, T &f_22, T &f_23,
						T &f_31, T &f_32, T &f_33 )
	{
		return f_11*f_22*f_33 + f_21*f_32*f_13 + f_31*f_12*f_23 - f_13*f_22*f_31 - f_23*f_32*f_11 - f_33*f_12*f_21;
	}


	template<typename T>
	void Matrix44<T>::invert( void )
	{
		T fDet =	_11*Det(	_22, _23, _24,
									_32, _33, _34,
									_42, _43, _44 ) -

						_12*Det(	_21, _23, _24,
									_31, _33, _34,
									_41, _43, _44 ) +

						_13*Det(	_21, _22, _24,
									_31, _32, _34,
									_41, _42, _44 ) -

						_14*Det(	_21, _22, _23,
									_31, _32, _33,
									_41, _42, _43 );

		// determinant must be not zero
		if( abs( fDet ) < (T)0.00001 )
			// error determinant is zero
			return;

		fDet = (T)1.0 / fDet;

		Matrix44<T> mMatrix;

		// Row1
		mMatrix._11 =		   Det(	_22, _23, _24,
									_32, _33, _34,
									_42, _43, _44 );

		mMatrix._12 = -(T)1.0 * Det(	_21, _23, _24,
									_31, _33, _34,
									_41, _43, _44 );

		mMatrix._13 =		   Det(	_21, _22, _24,
									_31, _32, _34,
									_41, _42, _44 );

		mMatrix._14 = -(T)1.0 * Det(	_21, _22, _23,
									_31, _32, _33,
									_41, _42, _43 );


		// Row2
		mMatrix._21 = -(T)1.0 * Det(	_12, _13, _14,
									_32, _33, _34,
									_42, _43, _44 );

		mMatrix._22 =		   Det(	_11, _13, _14,
									_31, _33, _34,
									_41, _43, _44 );

		mMatrix._23 = -(T)1.0 * Det(	_11, _12, _14,
									_31, _32, _34,
									_41, _42, _44 );

		mMatrix._24 =		   Det(	_11, _12, _13,
									_31, _32, _33,
									_41, _42, _43 );


		// Row3
		mMatrix._31 =		   Det(	_12, _13, _14,
									_22, _23, _24,
									_42, _43, _44 );

		mMatrix._32 = -(T)1.0 * Det(	_11, _13, _14,
									_21, _23, _24,
									_41, _43, _44 );

		mMatrix._33 =		   Det(	_11, _12, _14,
									_21, _22, _24,
									_41, _42, _44 );

		mMatrix._34 = -(T)1.0 * Det(	_11, _12, _13,
									_21, _22, _23,
									_41, _42, _43 );


		// Row4
		mMatrix._41 = -(T)1.0 * Det(	_12, _13, _14,
									_22, _23, _24,
									_32, _33, _34 );

		mMatrix._42 =		   Det(	_11, _13, _14,
									_21, _23, _24,
									_31, _33, _34 );

		mMatrix._43 = -(T)1.0 * Det(	_11, _12, _14,
									_21, _22, _24,
									_31, _32, _34 );

		mMatrix._44 =		   Det(	_11, _12, _13,
									_21, _22, _23,
									_31, _32, _33 );

		_11 = fDet*mMatrix._11;
		_12 = fDet*mMatrix._21;
		_13 = fDet*mMatrix._31;
		_14 = fDet*mMatrix._41;

		_21 = fDet*mMatrix._12;
		_22 = fDet*mMatrix._22;
		_23 = fDet*mMatrix._32;
		_24 = fDet*mMatrix._42;

		_31 = fDet*mMatrix._13;
		_32 = fDet*mMatrix._23;
		_33 = fDet*mMatrix._33;
		_34 = fDet*mMatrix._43;

		_41 = fDet*mMatrix._14;
		_42 = fDet*mMatrix._24;
		_43 = fDet*mMatrix._34;
		_44 = fDet*mMatrix._44;
	}

	template<typename T>
	Matrix44<T> Matrix44<T>::inverted( void )const
	{
		Matrix44<T> m = *this;
		m.invert();
		return m;
	}

	// same as inverted (here for Imath compatibility
	template<typename T>
	Matrix44<T> Matrix44<T>::inverse( void )const
	{
		Matrix44<T> m = *this;
		m.invert();
		return m;
	}



	template<typename T>
	Vec3<T> Matrix44<T>::getRight( const bool &normalized ) const
	{
		Vec3<T> right( _11, _12, _13 );

		if( normalized )
			right.normalize();

		return right;
	}

	template<typename T>
	Vec3<T> Matrix44<T>::getUp( const bool &normalized ) const
	{
		Vec3<T> up( _21, _22, _23 );

		if( normalized )
			up.normalize();

		return up;
	}

	template<typename T>
	Vec3<T> Matrix44<T>::getDir( const bool &normalized ) const
	{
		Vec3<T> dir( _31, _32, _33 );

		if( normalized )
			dir.normalize();

		return dir; 
	}

	template<typename T>
	Vec3<T> Matrix44<T>::getTranslation( void ) const
	{
		return Vec3<T>( _41, _42, _43 );
	}

	template<typename T>
	Matrix44<T> Matrix44<T>::getOrientation( void )const
	{
		return Matrix44<T>( _11, _12, _13, (T)0.0,
                          _21, _22, _23, (T)0.0,
                          _31, _32, _33, (T)0.0,
                           (T)0.0, (T)0.0, (T)0.0, (T)1.0 );
	}

	template<typename T>
	Matrix44<T> Matrix44<T>::getTransposed( void )const
	{
		return Matrix44<T>( _11, _21, _31, _41,
                          _12, _22, _32, _42,
                          _13, _23, _33, _43,
                          _14, _24, _34, _44 );
	}

	template<typename T>
	Matrix44<T> Matrix44<T>::getNormalizedOrientation( void )const
	{
		static Vec3<T> g_vRight;
		static Vec3<T> g_vUp;
		static Vec3<T> g_vDir;

		g_vRight	= getRight();
		g_vUp		= getUp();
		g_vDir		= getDir();

		return Matrix44<T>(	g_vRight.x, g_vRight.y, g_vRight.z, (T)0.0,
					    	g_vUp.x,	g_vUp.y, g_vUp.z, (T)0.0,
					    	g_vDir.x,	g_vDir.y, g_vDir.z, (T)0.0,
						    (T)0.0, (T)0.0, (T)0.0, (T)1.0 );
	}

	template<typename T>
	void Matrix44<T>::setRight( const Vec3<T> &right )
	{
		_11 = right.x;
		_12 = right.y;
		_13 = right.z;
	}

	template<typename T>
	void Matrix44<T>::setUp( const Vec3<T> &up )
	{
		_21 = up.x;
		_22 = up.y;
		_23 = up.z;
	}

	template<typename T>
	void Matrix44<T>::setDir( const Vec3<T> &dir )
	{
		_31 = dir.x;
		_32 = dir.y;
		_33 = dir.z;
	}

	template<typename T>
	void Matrix44<T>::setTranslation( const Vec3<T> &translation )
	{
		_41 = translation.x;
		_42 = translation.y;
		_43 = translation.z;
	}

	template<typename T>
	bool Matrix44<T>::operator==( const Matrix44<T> &rhs )
	{
		if( _11==rhs._11 && _12==rhs._12 && _13==rhs._13 && _14==rhs._14 &&
			_21==rhs._21 && _22==rhs._22 && _23==rhs._23 && _24==rhs._24 &&
			_31==rhs._31 && _32==rhs._32 && _33==rhs._33 && _34==rhs._34 &&
			_41==rhs._41 && _42==rhs._42 && _43==rhs._43 && _44==rhs._44 )
			return true;
		else
			return false; 
	}

	template<typename T>
	bool Matrix44<T>::operator!=( const Matrix44<T> &rhs )
	{
		return !((*this)==rhs);
	}
	
	template<typename T>
	bool Matrix44<T>::operator+=( const Matrix44<T> &rhs )
	{
		_11+=rhs._11;
		_12+=rhs._12;
		_13+=rhs._13;
		_14+=rhs._14;

		_21+=rhs._21;
		_22+=rhs._22;
		_23+=rhs._23;
		_24+=rhs._24;

		_31+=rhs._31;
		_32+=rhs._32;
		_33+=rhs._33;
		_34+=rhs._34;

		_41+=rhs._41;
		_42+=rhs._42;
		_43+=rhs._43;
		_44+=rhs._44;

		return true;
	}

	template<typename T>
	bool Matrix44<T>::operator-=( const Matrix44<T> &rhs )
	{
		_11-=rhs._11;
		_12-=rhs._12;
		_13-=rhs._13;
		_14-=rhs._14;

		_21-=rhs._21;
		_22-=rhs._22;
		_23-=rhs._23;
		_24-=rhs._24;

		_31-=rhs._31;
		_32-=rhs._32;
		_33-=rhs._33;
		_34-=rhs._34;

		_41-=rhs._41;
		_42-=rhs._42;
		_43-=rhs._43;
		_44-=rhs._44;

		return true;
	}

	template<typename T>
	bool Matrix44<T>::operator+=( const T &rhs )
	{
		_11+=rhs;
		_12+=rhs;
		_13+=rhs;
		_14+=rhs;

		_21+=rhs;
		_22+=rhs;
		_23+=rhs;
		_24+=rhs;

		_31+=rhs;
		_32+=rhs;
		_33+=rhs;
		_34+=rhs;

		_41+=rhs;
		_42+=rhs;
		_43+=rhs;
		_44+=rhs;

		return true;
	}

	template<typename T>
	bool Matrix44<T>::operator-=( const T &rhs )
	{
		_11-=rhs;
		_12-=rhs;
		_13-=rhs;
		_14-=rhs;

		_21-=rhs;
		_22-=rhs;
		_23-=rhs;
		_24-=rhs;

		_31-=rhs;
		_32-=rhs;
		_33-=rhs;
		_34-=rhs;

		_41-=rhs;
		_42-=rhs;
		_43-=rhs;
		_44-=rhs;

		return true;
	}

	template<typename T>
	bool Matrix44<T>::operator*=( const T &rhs )
	{
		_11*=rhs;
		_12*=rhs;
		_13*=rhs;
		_14*=rhs;

		_21*=rhs;
		_22*=rhs;
		_23*=rhs;
		_24*=rhs;

		_31*=rhs;
		_32*=rhs;
		_33*=rhs;
		_34*=rhs;

		_41*=rhs;
		_42*=rhs;
		_43*=rhs;
		_44*=rhs;

		return true;
	}


	template<typename T>
	bool Matrix44<T>::operator/=( const T &rhs )
	{
		_11/=rhs;
		_12/=rhs;
		_13/=rhs;
		_14/=rhs;

		_21/=rhs;
		_22/=rhs;
		_23/=rhs;
		_24/=rhs;

		_31/=rhs;
		_32/=rhs;
		_33/=rhs;
		_34/=rhs;

		_41/=rhs;
		_42/=rhs;
		_43/=rhs;
		_44/=rhs;

		return true;
	}


	typedef Matrix44<float> Matrix44f;
	typedef Matrix44<double> Matrix44d;
	typedef Matrix44<float> M44f;
	typedef Matrix44<double> M44d;

}

} // namespace houio
