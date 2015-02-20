/*---------------------------------------------------------------------

simple vector class

----------------------------------------------------------------------*/
#pragma once



namespace math
{
	///
	/// \brief simple vector class
	///
	template<typename T>
	struct Vec4
	{

        Vec4( );
        Vec4( const T &x, const T &y, const T &z, const T &w );
		Vec4( const T &xyzw );
		template <typename S> Vec4(const Vec4<S> &v);
		~Vec4( );


		/*
        void              set( const T &x, const T &y, const T &z );

		T                                    getLength( void )const; ///< returns the cartesian length of the vector
		T                             getSquaredLength( void )const; ///< returns the un-square-rooted length of the vector 
		void                          setLength( const T &fLength ); ///< scales the vector to the specified length
		void                                      normalize( void ); ///< normalizes the vector
		Vec3<T>                            normalized( void ) const; ///< returnes normalized version if this the vector
		void                                         negate( void ); ///< negates the vector

		void                       reflect( const Vec3<T> &normal ); ///< reflects the vector at the given normal



		bool                       operator==( const Vec3<T> &rhs );
		bool                       operator!=( const Vec3<T> &rhs );
		
		bool                       operator+=( const Vec3<T> &rhs );
		bool                       operator-=( const Vec3<T> &rhs );

		bool                             operator+=( const T &rhs );
		bool                             operator-=( const T &rhs );
		bool                             operator*=( const T &rhs );
		bool                             operator/=( const T &rhs );

		const T&                          operator[]( int i ) const;
		T&                                      operator[]( int i );
		*/

		union
		{
			struct
			{
				T x, y, z, w;
			};
			T v[4];
		};
	};



	template<typename T>
	Vec4<T>::Vec4()
	{
        x=(T)0.0; y=(T)0.0; z=(T)0.0; w=(T)0.0;
    }

	template<typename T>
	Vec4<T>::Vec4( const T &x, const T &y, const T &z, const T &w )
    {
        this->x=x; this->y=y; this->z=z; this->w=w;
    }

	template<typename T>
	Vec4<T>::Vec4( const T &xyzw ) : x(xyzw), y(xyzw), z(xyzw)
	{
	}

	template <typename T>
	template <typename S>
	inline
	Vec4<T>::Vec4 (const Vec4<S> &v)
	{
		x = T (v.x);
		y = T (v.y);
		z = T (v.z);
		w = T (v.w);
	}

	template<typename T>
	Vec4<T>::~Vec4( )
	{
	}

	/*
	template<typename T>
	void Vec3<T>::set( const T &x, const T &y, const T &z )
	{
		this->x=x; this->y=y; this->z=z;
	}

	template<typename T>
	T Vec3<T>::getLength( void ) const
	{
        return sqrt( x*x + y*y + z*z );
	}

	template<typename T>
	T Vec3<T>::getSquaredLength( void ) const
	{
		return x*x + y*y + z*z;
	}

	template<typename T>
	void Vec3<T>::setLength( const T &length )
	{
		normalize();

		x*=length;
		y*=length;
		z*=length;
	}

	template<typename T>
	void Vec3<T>::normalize( void )
	{
		T length = getLength();

		if( length != (T)0.0 )
		{
			x /= length;
			y /= length;
			z /= length;	
		}
	}

	///< returnes normalized version if this the vector
	template<typename T>
	Vec3<T> Vec3<T>::normalized( void ) const
	{
		T length = getLength();

		if( length != (T)0.0 )
			return Vec3<T>(x/length, y/length, z/length);
		else
			return Vec3<T>((T)0.0, (T)0.0, (T)0.0);
	}

	template<typename T>
	void Vec3<T>::negate( void )
	{
		x=-x; y=-y; z=-z;
	}

	// reflects the vector at the given normal
	template<typename T>
	void Vec3<T>::reflect( const Vec3<T> &normal )
	{
		normalize();

		Vec3<T>	temp( x, y, z );
		float	value;
		
		value = dot( normal, *this );
		value *= (T)(2.0);

		x = normal.x * value;
		y = normal.y * value;
		z = normal.z * value;

		x -= temp.x;
		y -= temp.y;
		z -= temp.z;
	}

	template<typename T>
	bool Vec3<T>::operator==( const Vec3<T> &rhs )
	{
		// TODO: fix this
		if( (abs(x - rhs.x) < (T)0.00001) && (abs(y - rhs.y) < (T)0.00001) && (abs(z - rhs.z) < (T)0.00001) )
			return true;
		else
			return false;
	}

	template<typename T>
	bool Vec3<T>::operator!=( const Vec3<T> &rhs )
	{
		return !((*this)==rhs);
	}
	
	template<typename T>
	bool Vec3<T>::operator+=( const Vec3<T> &rhs )
	{
		x+=rhs.x;
		y+=rhs.y;
		z+=rhs.z;

		return true;
	}

	template<typename T>
	bool Vec3<T>::operator-=( const Vec3<T> &rhs )
	{
		x-=rhs.x;
		y-=rhs.y;
		z-=rhs.z;

		return true;
	}

	template<typename T>
	bool Vec3<T>::operator+=( const T &rhs )
	{
		x+=rhs;
		y+=rhs;
		z+=rhs;

		return true;
	}

	template<typename T>
	bool Vec3<T>::operator-=( const T &rhs )
	{
		x-=rhs;
		y-=rhs;
		z-=rhs;

		return true; 
	}

	template<typename T>
	bool Vec3<T>::operator*=( const T &rhs )
	{
		x*=rhs;
		y*=rhs;
		z*=rhs;

		return true;
	}

	template<typename T>
	bool Vec3<T>::operator/=( const T &rhs )
	{
		x/=rhs;
		y/=rhs;
		z/=rhs;

		return true; 
	}

	template<typename T>
	const T& Vec3<T>::operator[]( int i ) const
	{
		return v[i];
	}

	template<typename T>
	T& Vec3<T>::operator[]( int i )
	{
		return v[i];
	}



	*/

	typedef Vec4<float> Vec4f;
	typedef Vec4<float> V4f;
	typedef Vec4<double> Vec4d;
	typedef Vec4<double> V4d;
	typedef Vec4<int> Vec4i;
	typedef Vec4<int> V4i;

}
