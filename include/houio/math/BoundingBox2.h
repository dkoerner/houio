/*---------------------------------------------------------------------

The BoundingBox2<T> class is a very simple utility class for working with
axis aligned bounding boxes in 2d

----------------------------------------------------------------------*/
#pragma once
#include "Math.h"

namespace houio
{

namespace math
{
	///
	/// \brief The BoundingBox2<T> class is a very simple utility class for working
	/// with axis aligned bounding boxes
	///
	template<typename T>
	struct BoundingBox2
	{
		BoundingBox2();                                                        ///< constructor
		BoundingBox2( math::Vec2<T> _minPoint, math::Vec2<T> _maxPoint );          ///< constructor

		math::Vec2<T>                                          size( void ) const;  ///< returns a vector which represents the dimension in each axis
		void                             extend( const math::Vec2<T> &nextPoint );  ///< adobts the bounding borders if neccessary so that the given point lies within the bounding box
		math::Vec2<T>                                     getCenter( void ) const;  ///< returns the geometrical center of the box
		bool                         encloses( const math::Vec2<T> &point ) const;  ///< this utility function checks wether the given point is within the volume descripted by the bounding box
		bool encloses( const math::Vec2<T> &min, const math::Vec2<T> &max ) const;  ///< this utility function checks wether the given box is within the volume descripted by the bounding box

		math::Vec2<T>                                                    minPoint;  ///< the position of all lowends for each axis
		math::Vec2<T>                                                    maxPoint;  ///< the position of all highends for each axis
	};



	// constructor
	template<typename T>
	BoundingBox2<T>::BoundingBox2()
	{
		minPoint = math::Vec2<T>( (T)99999999999.0, (T)99999999999.0 );
		maxPoint = math::Vec2<T>( (T)-99999999999.0, (T)-99999999999.0 );
	}

	// constructor
	template<typename T>
	BoundingBox2<T>::BoundingBox2( math::Vec2<T> _minPoint, math::Vec2<T> _maxPoint )
	{
		minPoint = _minPoint;
		maxPoint = _maxPoint;
	}

	// returns a vector which represents the dimension in each axis
	template<typename T>
	math::Vec2<T> BoundingBox2<T>::size( void ) const
	{
		return math::Vec2<T>( maxPoint - minPoint );
	}

	// adobts the bounding borders if neccessary so that the given point lies within
	// the bounding box
	template<typename T>
	void BoundingBox2<T>::extend( const math::Vec2<T> &nextPoint )
	{
		if( nextPoint.x > maxPoint.x )
			maxPoint.x = nextPoint.x;
		if( nextPoint.y > maxPoint.y )
			maxPoint.y = nextPoint.y;

		if( nextPoint.x < minPoint.x )
			minPoint.x = nextPoint.x;
		if( nextPoint.y < minPoint.y )
			minPoint.y = nextPoint.y;
	}


	// returns the geometrical center of the box
	template<typename T>
	math::Vec2<T> BoundingBox2<T>::getCenter( void ) const
	{
		return (minPoint + maxPoint)*(T)0.5;
	}

	// this utility function checks wether the given point
	// is within the volume descripted by the bounding box
	template<typename T>
	bool BoundingBox2<T>::encloses( const math::Vec2<T> &point ) const
	{
		// check each dimension
		if( (point.x > minPoint.x)&&(point.x < maxPoint.x)&&
			(point.y > minPoint.y)&&(point.y < maxPoint.y))
			return true;
		else
			return false;
	}

	// this utility function checks wether the given box
	// is within the volume descripted by the bounding box
	template<typename T>
	bool BoundingBox2<T>::encloses( const math::Vec2<T> &min, const math::Vec2<T> &max ) const
	{
		// check each dimension for each point
		if( (min.x >= minPoint.x)&&(min.y >= minPoint.y)&&
			(max.x <= maxPoint.x)&&(max.y <= maxPoint.y))
			return true;
		else
			return false;
	}
}

} // namespace houio
