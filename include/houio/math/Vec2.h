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
	struct Vec2
	{
        Vec2( );
        Vec2( const T &x, const T &y );
		Vec2( const T &xy );
		~Vec2( );

		T                                    getLength( void )const; ///< returns the cartesian length of the vector
		T                             getSquaredLength( void )const; ///< returns the un-square-rooted length of the vector 
		void                          setLength( const T &fLength ); ///< scales the vector to the specified length
		void                                      normalize( void ); ///< normalizes the vector
		Vec2<T>                            normalized( void ) const; ///< returnes normalized version if this the vector
		void                                         negate( void ); ///< negates the vector

		//void                       reflect( const Vec2<T> &normal ); ///< reflects the vector at the given normal



		bool                       operator==( const Vec2<T> &rhs );
		bool                       operator!=( const Vec2<T> &rhs );
		
		bool                       operator+=( const Vec2<T> &rhs );
		bool                       operator-=( const Vec2<T> &rhs );

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
				T x, y;
			};
			T v[2];
		};
	};


	template<typename T>
	Vec2<T>::Vec2()
	{
        x=(T)0.0; y=(T)0.0;
    }

	template<typename T>
	Vec2<T>::Vec2( const T &x, const T &y )
    {
        this->x=x; this->y=y;
    }

	template<typename T>
	Vec2<T>::Vec2( const T &xyz ) : x(xyz), y(xyz)
	{
	}

	template<typename T>
	Vec2<T>::~Vec2( )
	{
	}
	
	///< returnes normalized version if this the vector
	template<typename T>
	Vec2<T> Vec2<T>::normalized( void ) const
	{
		T length = getLength();

		if( length != (T)0.0 )
			return Vec2<T>(x/length, y/length);
		else
			return Vec2<T>((T)0.0, (T)0.0);
	}

	template<typename T>
	void Vec2<T>::negate( void )
	{
		x=-x; y=-y;
	}
	/*
	// reflects the vector at the given normal
	template<typename T>
	void Vec2<T>::reflect( const Vec2<T> &normal )
	{
		normalize();

		Vec2<T>	temp( x, y );
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
	*/

	template<typename T>
	bool Vec2<T>::operator==( const Vec2<T> &rhs )
	{
		// TODO: fix this
		if( (abs(x - rhs.x) < (T)0.00001) && (abs(y - rhs.y) < (T)0.00001) )
			return true;
		else
			return false;
	}

	template<typename T>
	bool Vec2<T>::operator!=( const Vec2<T> &rhs )
	{
		return !((*this)==rhs);
	}
	
	template<typename T>
	bool Vec2<T>::operator+=( const Vec2<T> &rhs )
	{
		x+=rhs.x;
		y+=rhs.y;

		return true;
	}

	template<typename T>
	bool Vec2<T>::operator-=( const Vec2<T> &rhs )
	{
		x-=rhs.x;
		y-=rhs.y;

		return true;
	}

	template<typename T>
	bool Vec2<T>::operator+=( const T &rhs )
	{
		x+=rhs;
		y+=rhs;

		return true;
	}

	template<typename T>
	bool Vec2<T>::operator-=( const T &rhs )
	{
		x-=rhs;
		y-=rhs;

		return true; 
	}

	template<typename T>
	bool Vec2<T>::operator*=( const T &rhs )
	{
		x*=rhs;
		y*=rhs;

		return true;
	}

	template<typename T>
	bool Vec2<T>::operator/=( const T &rhs )
	{
		x/=rhs;
		y/=rhs;

		return true; 
	}

	template<typename T>
	const T& Vec2<T>::operator[]( int i ) const
	{
		return v[i];
	}

	template<typename T>
	T& Vec2<T>::operator[]( int i )
	{
		return v[i];
	}


	// stream output
	template <class T>
	std::ostream &
	operator << (std::ostream &s, const Vec2<T> &v)
	{
		return s << '(' << v.x << ' ' << v.y << ')';
	}




	// shortcuts
	typedef Vec2<float> Vec2f;
	typedef Vec2<double> Vec2d;
	typedef Vec2<int> Vec2i;
	typedef Vec2<float> V2f;
	typedef Vec2<double> V2d;
	typedef Vec2<int> V2i;
	
}
