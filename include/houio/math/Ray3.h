/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once
#include "Vec3.h"
#include "Vec3Algo.h"
#include "Matrix33.h"
#include "BoundingBox3.h"
#include <limits>
#include <algorithm>
#include <cassert>



namespace math
{
	/// \brief a class which holds an origin and a target and is used by intersection routines
	template<typename T>
	class Ray3
	{
	public:
		Ray3();                                                                                   // constructor
		Ray3( const math::Vec3<T> &origin, const math::Vec3<T> &direction, const T &_tmin, const T &_tmax = std::numeric_limits<T>::max());       // constructor

		math::Vec3<T>                                                    getPosition( T t )const; // returns origin+direction*t
		math::Vec3<T>                                                     operator()( T t )const; // returns origin+direction*t

		math::Vec3<T>                                                                          o; // point in space where the ray originates from
		math::Vec3<T>                                                                          d; // normalized direction of the ray
		T                                                                             tmin, tmax; // valid ray segment
	};

	

	// constructor
	template<typename T>
	Ray3<T>::Ray3() : tmin(0.0f), tmax(std::numeric_limits<T>::max())
	{
	}

	// constructor
	template<typename T>
	Ray3<T>::Ray3( const math::Vec3<T> &origin, const math::Vec3<T> &direction, const T &_tmin, const T &_tmax ) : o(origin), d(direction), tmin(_tmin), tmax(_tmax)
	{
	}


	// returns origin+direction*t
	template<typename T>
	math::Vec3<T> Ray3<T>::getPosition( T t )const
	{
		return o + d*t;
	}

	template<typename T>
	math::Vec3<T> Ray3<T>::operator()( T t )const // returns origin+direction*t
	{
		return o + d*t;
	}
	





	//
	// ray related functions
	//

	//bool RayHitSphere( const Vector &vSpherePosition, const float &fSphereRadius, CRay &oRay );
	//bool RayHitSphereValues( const Vector &vSpherePosition, const float &fSphereRadius, CRay &oRay, float &fHitNear, float &fHitFar, Vector *pHitPointNear, Vector *pHitPointFar );

	//bool RayHitPlane( const Vector &vPlaneNormal, const float &fD, CRay &oRay );
	//bool rayHitPlaneValues( const Vec3f &planeNormal, const float &planeDistance, Ray &ray, float &hitDistance, Vec3f *hitPoint );



	// computes a intersection between a ray and a plane
	//
	// returns true if an intersection occured, false otherwise
	template<typename T>
	bool intersectionRayPlane( const Ray3<T> &ray, const Vec3<T> &normal, const T &distance, Vec3<T> &hitPoint )
	{
		// project the ray direction onto the plane normal
		T temp = dot( ray.d, normal );

		// if result is zero, then the direction is parallel to the plane -> no intersection
		if( !temp )
			return false;

		float hitDistance = -(dot( normal, ray.o ) + distance) / temp;

		// the point must lie on the raysegment between origin and target to pass the test
		if( (hitDistance >= ray.tmax) || (hitDistance <= ray.tmin) )
			return false;

		hitPoint = ray.origin + hitDistance*ray.direction;

		return true;
	}


	// computes a intersection between a ray and another ray
	// algorithm based on Graphics Gems I page 304
	//
	// note: the 2 rays must be coplanar
	//
	// returns true if an intersection occured, false otherwise
	template<typename T>
	bool intersectionRayRay( const Ray3<T> &ray1, const Ray3<T> &ray2, Vec3<T> &hitPoint )
	{
		math::Vec3<T> cp = math::cross( ray1.d, ray2.d );
		T denom = cp.getSquaredLength();

		if( denom == (T)0.0 )
			// lines are parallel
			return false;

		// we need to compute s and t to test the line segments
		math::Vec3<T> c1 = ray2.o - ray1.o;

		T t = math::Matrix33<T>( c1.x, ray2.d.x, cp.x, c1.y, ray2.d.y, cp.y, c1.z, ray2.d.z, cp.z ).getDeterminant() / denom;
		T s = math::Matrix33<T>( c1.x, ray1.d.x, cp.x, c1.y, ray1.d.y, cp.y, c1.z, ray1.d.z, cp.z ).getDeterminant() / denom;


		// check line segments
		if( (t < ray1.tmin) || (s < ray2.tmin) || (t > ray1.tmax) || (s > ray2.tmax) )
			return false;

		// compute intersection point
		hitPoint = ray2.o + ray2.d*s;

		// check for coplanarity
		//if( hitPoint != (ray1.origin + ray1.direction*t) )
		//	return false;

		// done
		return true;
	}


	// sphere is assumed to be at origin, r is radius
	template<typename T>
	bool intersectionRaySphere( const Ray3<T> &ray, T radius, T &t0, T &t1 )
	{
		math::Vec3<T> vec = ray.o;
		T a = ray.d.x * ray.d.x + ray.d.y * ray.d.y + ray.d.z * ray.d.z;
		T b = 2 * (ray.d.x * vec.x +  ray.d.y * vec.y + ray.d.z * vec.z);
		T c = vec.x * vec.x + vec.y * vec.y + vec.z * vec.z - radius * radius;

		if (b == 0)
		{
			// handle special case where the the two vector ray.dir and V are perpendicular
			// with V = ray.orig - sphere.centre
			if (a == 0) return false;
			t0 = 0; t1 = sqrt(-c/a);
		}else
		{
			T discr = b * b - 4 * a * c;
			if (discr < 0) return false;
			T q = (b < 0) ? (T)-0.5 * (b - sqrt(discr)) : (T)-0.5 * (b + sqrt(discr));
			t0 = q / a;
			t1 = c / q;
		}


		if (t0 > t1) std::swap(t0, t1);
		return true;
	}

	template<typename T>
	bool intersectionRayBox( const Ray3<T> &ray, const BoundingBox3<T> &box, T &hitt0, T &hitt1 )
	{
		T t0 = ray.tmin, t1 = ray.tmax;
		for (int i = 0; i < 3; ++i)
		{
			// Update interval for _i_th bounding box slab
			T invRayDir = 1.f / ray.d[i];
			T tNear = (box.minPoint[i] - ray.o[i]) * invRayDir;
			T tFar  = (box.maxPoint[i] - ray.o[i]) * invRayDir;

			// Update parametric interval from slab intersection $t$s
			if (tNear > tFar) std::swap(tNear, tFar);
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar  < t1 ? tFar  : t1;
			if (t0 > t1) return false;
		}
		hitt0 = t0;
		hitt1 = t1;
		return true;
	}




	//bool RayHitTriangle( const Vector &vPoint1, const Vector &vPoint2, const Vector &vPoint3, CRay &oRay );
	//bool RayHitTriangleValues( const Vector &vPoint1, const Vector &vPoint2, const Vector &vPoint3, CRay &oRay, float &fHit, Vector *pHitPoint, float *pU, float *pV );

	typedef Ray3<float> Ray3f;
	typedef Ray3<double> Ray3d;
}
