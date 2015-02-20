/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once



namespace math
{
	//
	//
	// memory storage is row major (which is native to the c++ arrays)
    //     | 0  1  |
    // M = | 2  3  |
	//
	// index the matrix with matrix.m[row][column] -> _m12 means: first row, second column
	//
	//           m11 m12
	//           m21 m22
	//
	//
	template<typename T>
	class Matrix22
	{
	public:
        Matrix22();
		~Matrix22();
		Matrix22( const T &_11, const T &_12,
			      const T &_21, const T &_22 );

		// convience matrix creation functions
		static Matrix22<T>                                                                 Zero( void );  // returns the zeromatrix
		static Matrix22<T>                                                             Identity( void );  // returns the identitymatrix
		static Matrix22<T>                                             RotationMatrix( const T &angle );  // returns a matrix which defines a rotation with the T-specified amount (in radians)

		// public methods
		void                                                                          transpose( void );
		Matrix22<T>                                                             transposed( void )const;
		void                                                                             invert( void );
		T                                                                                       trace(); //returns sum of elements in main diagonal

		// operators
		bool                                                       operator==( const Matrix22<T> &rhs );
		bool                                                       operator!=( const Matrix22<T> &rhs );
		
		bool                                                       operator+=( const Matrix22<T> &rhs );
		bool                                                       operator-=( const Matrix22<T> &rhs );

		bool                                                                 operator+=( const T &rhs );
		bool                                                                 operator-=( const T &rhs );
		bool                                                                 operator*=( const T &rhs );
		bool                                                                 operator/=( const T &rhs );

		union
		{
			struct
			{
				T _11, _12;	
				T _21, _22;
			};
			T m[2][2];
			T ma[4];
		};
	};
	
	
	
	
	template<typename T>
	Matrix22<T>::Matrix22()
	{
		_11=_12=
		_21=_22=(T)0.0;
	}

	template<typename T>
	Matrix22<T>::Matrix22( const T &_11, const T &_12,
                          const T &_21, const T &_22 )
	{
        this->_11=_11; this->_12=_12;
        this->_21=_21; this->_22=_22;
	}

	template<typename T>
	Matrix22<T>::~Matrix22()
	{
	}

	// returns the zeromatrix
	template<typename T>
	Matrix22<T> Matrix22<T>::Zero( void )
	{
		return Matrix22<T>( (T)0.0, (T)0.0,
			              (T)0.0, (T)0.0);
	}

	// returns the identitymatrix
	template<typename T>
	Matrix22<T> Matrix22<T>::Identity( void )
	{
		return Matrix22<T>( (T)1.0, (T)0.0,
			              (T)0.0, (T)1.0);
	}


	// returns a matrix which defines a rotation with the T-specified amount (in radians)
	template<typename T>
	Matrix22<T> Matrix22<T>::RotationMatrix( const T &angle )
	{
		T s = sin( angle );
		T c = cos( angle );
		return Matrix22<T>( c, -s,
			                s, c );
	}

	template<typename T>
	void Matrix22<T>::transpose( void )
	{
		Matrix22<T> temp = *this;

		_11 = temp._11;
		_12 = temp._21;

		_21 = temp._12;
		_22 = temp._22;
	}

	template<typename T>
	Matrix22<T> Matrix22<T>::transposed( void )const
	{
		Matrix22<T> m = *this;
		m.transpose();
		return m;
	}

	template<typename T>
	void Matrix22<T>::invert( void )
	{
		// a b
		// c d
		//Then the inverse is 1/(ad-cb) * d -b-c a
		//T c = (T)1.0/( _11*_22 - _21*_12 );
		//*this = Matrix22<T>( c*_22, -c*_12, -c*_21, c*_11 );
	}


	//returns sum of elements in main diagonal
	template<typename T>
	T Matrix22<T>::trace()
	{
		return _11 + _22;
	}

	template<typename T>
	bool Matrix22<T>::operator==( const Matrix22<T> &rhs )
	{
		if( _11==rhs._11 && _12==rhs._12 &&
			_21==rhs._21 && _22==rhs._22 )
			return true;
		else
			return false; 
	}

	template<typename T>
	bool Matrix22<T>::operator!=( const Matrix22<T> &rhs )
	{
		return !((*this)==rhs);
	}

	template<typename T>
	bool Matrix22<T>::operator+=( const Matrix22<T> &rhs )
	{
		_11+=rhs._11;
		_12+=rhs._12;


		_21+=rhs._21;
		_22+=rhs._22;

		return true;
	}

	template<typename T>
	bool Matrix22<T>::operator-=( const Matrix22<T> &rhs )
	{
		_11-=rhs._11;
		_12-=rhs._12;


		_21-=rhs._21;
		_22-=rhs._22;

		return true;
	}

	template<typename T>
	bool Matrix22<T>::operator+=( const T &rhs )
	{
		_11+=rhs;
		_12+=rhs;


		_21+=rhs;
		_22+=rhs;

		return true;
	}


	template<typename T>
	bool Matrix22<T>::operator-=( const T &rhs )
	{
		_11-=rhs;
		_12-=rhs;

		_21-=rhs;
		_22-=rhs;

		return true;
	}

	template<typename T>
	bool Matrix22<T>::operator*=( const T &rhs )
	{
		_11*=rhs;
		_12*=rhs;

		_21*=rhs;
		_22*=rhs;

		return true;
	}


	template<typename T>
	bool Matrix22<T>::operator/=( const T &rhs )
	{
		_11/=rhs;
		_12/=rhs;

		_21/=rhs;
		_22/=rhs;

		return true;
	}



	typedef Matrix22<float> Matrix22f;
	typedef Matrix22<double> Matrix22d;
}
