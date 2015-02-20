/*---------------------------------------------------------------------

simple vector class

----------------------------------------------------------------------*/
#pragma once
#include <iostream>


namespace math
{
	///
	/// \brief simple vector class
	///
	template<typename T>
	struct Vec3
	{

        Vec3( );
        Vec3( const T &x, const T &y, const T &z );
		Vec3( const T &xyz );
		template <typename S> Vec3(const Vec3<S> &v);
		~Vec3( );



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

		union
		{
			struct
			{
				T x, y, z;
			};
			T v[3];
		};
	};



	template<typename T>
	Vec3<T>::Vec3()
	{
        x=(T)0.0; y=(T)0.0; z=(T)0.0;
    }

	template<typename T>
	Vec3<T>::Vec3( const T &x, const T &y, const T &z )
    {
        this->x=x; this->y=y; this->z=z;
    }

	template<typename T>
	Vec3<T>::Vec3( const T &xyz ) : x(xyz), y(xyz), z(xyz)
	{
	}

	template <typename T>
	template <typename S>
	inline
	Vec3<T>::Vec3 (const Vec3<S> &v)
	{
		x = T (v.x);
		y = T (v.y);
		z = T (v.z);
	}

	template<typename T>
	Vec3<T>::~Vec3( )
	{
	}

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



	// stream output
	template <class T>
	std::ostream &
	operator << (std::ostream &s, const Vec3<T> &v)
	{
		return s << '(' << v.x << ' ' << v.y << ' ' << v.z <<')';
	}



	// shortcuts
	typedef Vec3<float> Vec3f;
	typedef Vec3<double> Vec3d;
	typedef Vec3<int> Vec3i;
	typedef Vec3<float> V3f;
	typedef Vec3<double> V3d;
	typedef Vec3<int> V3i;
}
