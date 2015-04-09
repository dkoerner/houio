/*---------------------------------------------------------------------

The BoundingBox class is a very simple utility class for working with
axis aligned bounding boxes

----------------------------------------------------------------------*/
#pragma once

#include "Vec3.h"
#include <limits>

namespace houio
{

namespace math
{
	///
	/// \brief The BoundingBox class is a very simple utility class for working
	/// with axis aligned bounding boxes
	///
	template<typename T>
	struct BoundingBox3
	{
		BoundingBox3();                                                            ///< constructor
		BoundingBox3( math::Vec3<T> _minPoint, math::Vec3<T> _maxPoint );          ///< constructor
		BoundingBox3( T minx, T miny, T minz, T maxx, T maxy, T maxz );            ///< constructor
		template<typename S>
		BoundingBox3( const BoundingBox3<S> &other );

		math::Vec3<T>                                          size( void ) const;  ///< returns a vector which represents the dimension in each axis
		void                             extend( const math::Vec3<T> &nextPoint );  ///< adobts the bounding borders if neccessary so that the given point lies within the bounding box
		void                           extendBy( const math::Vec3<T> &nextPoint );  ///< adobts the bounding borders if neccessary so that the given point lies within the bounding box - just here for Imath compatibility
		void                           extend( const BoundingBox3<T> &nextBound );  ///< adobts the bounding borders if neccessary so that the given boundingbox lies within the bounding box
		math::Vec3<T>                                     getCenter( void ) const;  ///< returns the geometrical center of the box
		bool                         encloses( const math::Vec3<T> &point ) const;  ///< this utility function checks wether the given point is within the volume descripted by the bounding box
		bool encloses( const math::Vec3<T> &min, const math::Vec3<T> &max ) const;  ///< this utility function checks wether the given box is within the volume descripted by the bounding box
		bool                                                      isEmpty() const;
		void                                                          makeEmpty();
		int                                                      maxExtend()const;

		math::Vec3<T>                                                    minPoint;  ///< the position of all lowends for each axis
		math::Vec3<T>                                                    maxPoint;  ///< the position of all highends for each axis
	};







	// constructor
	template<typename T>
	BoundingBox3<T>::BoundingBox3()
	{
		makeEmpty();
	}

	// constructor
	template<typename T>
	BoundingBox3<T>::BoundingBox3( math::Vec3<T> _minPoint, math::Vec3<T> _maxPoint )
	{
		minPoint = _minPoint;
		maxPoint = _maxPoint;
	}

	template<typename T>
	BoundingBox3<T>::BoundingBox3( T minx, T miny, T minz, T maxx, T maxy, T maxz )
	{
		minPoint = math::Vec3<T>( minx, miny, minz );
		maxPoint = math::Vec3<T>( maxx, maxy, maxz );
	}

	// constructor
	template<typename T>
	template<typename S>
	BoundingBox3<T>::BoundingBox3( const BoundingBox3<S> &other )
	{
		minPoint = other.minPoint;
		maxPoint = other.maxPoint;
	}

	// returns a vector which represents the dimension in each axis
	template<typename T>
	math::Vec3<T> BoundingBox3<T>::size( void ) const
	{
		return math::Vec3<T>( maxPoint - minPoint );
	}

	template<typename T>
	bool BoundingBox3<T>::isEmpty() const
	{
		return (minPoint.x >= maxPoint.x)&&(minPoint.y >= maxPoint.y)&&(minPoint.z >= maxPoint.z);
	}

	template<typename T>
	void BoundingBox3<T>::makeEmpty()
	{
		minPoint = math::Vec3<T>( (T)std::numeric_limits<T>::max(), (T)std::numeric_limits<T>::max(), (T)std::numeric_limits<T>::max() );
		maxPoint = math::Vec3<T>( (T)-std::numeric_limits<T>::max(), (T)-std::numeric_limits<T>::max(), (T)-std::numeric_limits<T>::max() );
	}

	template<typename T>
	int BoundingBox3<T>::maxExtend()const
	{
		Vec3<T> diag = maxPoint - minPoint;
		if (diag.x > diag.y && diag.x > diag.z)
			return 0;
		else if (diag.y > diag.z)
			return 1;
		else
			return 2;
	}

	// adobts the bounding borders if neccessary so that the given point lies within
	// the bounding box
	template<typename T>
    void BoundingBox3<T>::extend( const math::Vec3<T> &nextPoint )
	{
		if( nextPoint.x > maxPoint.x )
			maxPoint.x = nextPoint.x;
		if( nextPoint.y > maxPoint.y )
			maxPoint.y = nextPoint.y;
		if( nextPoint.z > maxPoint.z )
			maxPoint.z = nextPoint.z;

		if( nextPoint.x < minPoint.x )
			minPoint.x = nextPoint.x;
		if( nextPoint.y < minPoint.y )
			minPoint.y = nextPoint.y;
		if( nextPoint.z < minPoint.z )
			minPoint.z = nextPoint.z;
	}

	// adobts the bounding borders if neccessary so that the given point lies within
	// the bounding box
	template<typename T>
    void BoundingBox3<T>::extendBy( const math::Vec3<T> &nextPoint )
	{
		if( nextPoint.x > maxPoint.x )
			maxPoint.x = nextPoint.x;
		if( nextPoint.y > maxPoint.y )
			maxPoint.y = nextPoint.y;
		if( nextPoint.z > maxPoint.z )
			maxPoint.z = nextPoint.z;

		if( nextPoint.x < minPoint.x )
			minPoint.x = nextPoint.x;
		if( nextPoint.y < minPoint.y )
			minPoint.y = nextPoint.y;
		if( nextPoint.z < minPoint.z )
			minPoint.z = nextPoint.z;
	}

	///< adobts the bounding borders if neccessary so that the given boundingbox lies within the bounding box
	template<typename T>
	void BoundingBox3<T>::extend( const BoundingBox3<T> &nextBound )
	{
		extend( nextBound.minPoint );
		extend( nextBound.maxPoint );
	}


	// returns the geometrical center of the box
	template<typename T>
    math::Vec3<T> BoundingBox3<T>::getCenter( void ) const
	{
		return (minPoint + maxPoint)*(T)0.5;
	}

	// this utility function checks wether the given point
	// is within the volume descripted by the bounding box
	template<typename T>
	bool BoundingBox3<T>::encloses( const math::Vec3<T> &point ) const
	{
		// check each dimension
		if( (point.x > minPoint.x)&&(point.x < maxPoint.x)&&
			(point.y > minPoint.y)&&(point.y < maxPoint.y)&&
			(point.z > minPoint.z)&&(point.z < maxPoint.z))
			return true;
		else
			return false;
	}

	// this utility function checks wether the given box
	// is within the volume descripted by the bounding box
	template<typename T>
	bool BoundingBox3<T>::encloses( const math::Vec3<T> &min, const math::Vec3<T> &max ) const
	{
		// check each dimension for each point
		if( (min.x >= minPoint.x)&&(min.y >= minPoint.y)&&(min.z >= minPoint.z)&&
			(max.x <= maxPoint.x)&&(max.y <= maxPoint.y)&&(max.z <= maxPoint.z))
			return true;
		else
			return false;
	}

	typedef BoundingBox3<float> BoundingBox3f;
	typedef BoundingBox3<double> BoundingBox3d;
	typedef BoundingBox3<float> Box3f;
	typedef BoundingBox3<double> Box3d;

}

} // namespace houio
