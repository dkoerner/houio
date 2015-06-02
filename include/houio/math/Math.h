/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once

#include <math.h>

#define MATH_PIf 3.14159265f
#define MATH_PI 3.14159265

#define MATH_2PIf 6.2831853f
#define MATH_INV_PIf 0.31830988618379067154f
#define MATH_INV_2PIf 0.15915494309189533577f
#define MATH_INV_4PIf 0.07957747154594766788f




#include "Vec2.h"
#include "Vec2Algo.h"
#include "Vec3.h"
#include "Vec3Algo.h"
#include "Vec4.h"
#include "Vec4Algo.h"
#include "Ray3.h"
#include "Matrix22.h"
#include "Matrix22Algo.h"
#include "Matrix33.h"
#include "Matrix33Algo.h"
#include "Matrix44.h"
#include "Matrix44Algo.h"
#include "Color.h"
#include "BoundingBox3.h"
#include "BoundingBox2.h"
#include "RNG.h"

namespace houio
{


///
/// \brief vectors matrices and mathematical operations
///
namespace math
{
/*
	inline int isnan( float f )
	{
		#if defined(_WINDOWS)
		return _isnan(f);
		#else
		return isnan(f);
		#endif
	}

	inline int isinf( float f )
	{
		#if defined(_WINDOWS)
		return !_finite((f));
		#else
		return isinf(f);
		#endif
	}
*/
	inline float radToDeg( float rad )
	{
		return (float) ( (rad * 180.0f) / MATH_PIf );
	}

	inline float degToRad( float degree )
	{
		return (float) (( degree *  MATH_PIf)/180.0f );
	}

	inline double radToDeg( double rad )
	{
		return (double) ( (rad * 180.0) / MATH_PI );
	}

	inline double degToRad( double degree )
	{
		return (double) (( degree *  MATH_PI)/180.0 );
	}

	inline float sign( float f )
	{
		return f < 0.0f ? -1.0f : 1.0f;
	}

	//
	// spherical coordinate convention:
	// 
	//                  Y
	//                  ^
	//                  |  Z
	// elevation ----¬  | /
	// angle between  / |/ ) azimuth: -angle between X and YZ plane,
	// Y and XZ plane \ +------> X    -is 0 for (1,0,0)
	// is 0 for (0,1,0)               -increases towards Z+
	//
	//

	// converts spherical to cartesian coordinates
	template<typename T>
	Vec3<T> sphericalToCartesian( T azimuth, T elevation, T r = 1.0f )
	{
		T sint = sin(elevation);
		return Vec3<T>( r * sint * cos(azimuth), r * cos(elevation), r * sint * sin(azimuth));
	}


	// x=azimuth, y=elevation, z=distance
	template<typename T>
	void cartesianToSpherical( Vec3<T> p, T &azimuth, T &elevation, T &distance )
	{
		distance = p.getLength();
		elevation = acos( p.y / distance );
		azimuth = atan2( p.z, p.x );
	}

	//
	// color conversion functions
	//





	/*
	#define ALPHAMASK	0xff000000
	#define REDMASK		0x00ff0000
	#define GREENMASK	0x0000ff00
	#define BLUEMASK	0x000000ff

	#define ALPHASHIFT 24
	#define REDSHIFT   16
	#define GREENSHIFT 8
	#define BLUESHIFT  0
	*/
	// RGBA
	#define REDMASK		0x000000ff
	#define GREENMASK	0x0000ff00
	#define BLUEMASK	0x00ff0000
	#define ALPHAMASK	0xff000000

	#define REDSHIFT   0
	#define GREENSHIFT 8
	#define BLUESHIFT  16
	#define ALPHASHIFT 24

	inline unsigned int getAlpha( const unsigned long &color )
	{
		return ((color&ALPHAMASK) >> ALPHASHIFT);
	}
	inline unsigned int	getRed( const unsigned long &color )
	{
		return ((color&REDMASK) >> REDSHIFT);
	}
	inline unsigned int	getGreen( const unsigned long &color )
	{
		return ((color&GREENMASK) >> GREENSHIFT);
	}
	inline unsigned int	getBlue( const unsigned long &color )
	{
		return ((color&BLUEMASK) >> BLUESHIFT);
	}
	inline unsigned long setColor( const unsigned int &r, const unsigned int &g, const unsigned int &b, const unsigned int &a )
	{
		return ((a << ALPHASHIFT) + (r << REDSHIFT) + (g << GREENSHIFT) + (b << BLUESHIFT));
	}

	inline unsigned int	getAlpha( const Color &color )
	{
		return (unsigned int)(color.a*255.0f);
	}

	inline unsigned int	getRed(		const Color &color )
	{
		return (unsigned int)(color.r*255.0f);
	}

	inline unsigned int	getGreen(	const Color &color )
	{
		return (unsigned int)(color.g*255.0f);
	}

	inline unsigned int	getBlue(	const Color &color )
	{
		return (unsigned int)(color.b*255.0f);
	}

	inline Color setRGBColor(const unsigned int &r, const unsigned int &g, const unsigned int &b, const unsigned int &a )
	{
		return Color( ((float)r)/255.0f, ((float)g)/255.0f, ((float)b)/255.0f, ((float)a)/255.0f );
	}

	//
	// Color related ops
	//

	inline Color operator+( const Color &lhs, const Color &rhs )
	{
	return Color( lhs.r+rhs.r, lhs.g+rhs.g, lhs.b+rhs.b, lhs.a+rhs.a );
	}
	inline Color operator-( const Color &lhs, const Color &rhs )
	{
	return Color( lhs.r-rhs.r, lhs.g-rhs.g, lhs.b-rhs.b, lhs.a-rhs.a );
	}
	inline Color operator*( const Color &lhs, const Color &rhs )
	{
	return Color( lhs.r*rhs.r, lhs.g*rhs.g, lhs.b*rhs.b, lhs.a*rhs.a );
	}
	inline Color operator/( const Color &lhs, const Color &rhs )
	{
	return Color( lhs.r/rhs.r, lhs.g/rhs.g, lhs.b/rhs.b, lhs.a/rhs.a );
	}

	inline Color operator+( const Color &lhs, const float &rhs )
	{
	return Color( lhs.r+rhs, lhs.g+rhs, lhs.b+rhs, lhs.a+rhs );
	}
	inline Color operator-( const Color &lhs, const float &rhs )
	{
	return Color( lhs.r-rhs, lhs.g-rhs, lhs.b-rhs, lhs.a-rhs );
	}
	inline Color operator*( const Color &lhs, const float &rhs )
	{
	return Color( lhs.r*rhs, lhs.g*rhs, lhs.b*rhs, lhs.a*rhs );
	}
	inline Color operator/( const Color &lhs, const float &rhs )
	{
	return Color( lhs.r/rhs, lhs.g/rhs, lhs.b/rhs, lhs.a/rhs );
	}

	inline Color operator+( const float &lhs, const Color &rhs )
	{
	return Color( (lhs + rhs.r), (lhs + rhs.g), (lhs + rhs.b), (lhs + rhs.a) );
	}
	inline Color operator-( const float &lhs, const Color &rhs )
	{
	return (rhs-lhs);
	}
	inline Color operator*( const float &lhs, const Color &rhs )
	{
	return (rhs*lhs);
	}
	inline Color operator/( const float &lhs, const Color &rhs )
	{
	return (rhs/lhs);
	}

	//
	// Inverts only the color channels of the given color, not the alpha channel
	//
	inline Color invert( const Color &col )
	{
		return Color( 1.0f - col.r, 1.0f - col.g, 1.0f - col.b, col.a );
	}




	float                                                   area( const Vec3f &p0, const Vec3f &p1, const Vec3f &p2 ); // computes area of an triangle

	float                                                                distance( const Vec3f &p0, const Vec3f &p1 ); // computes the euclidian distance between 2 points in space
	float                                                         squaredDistance( const Vec3f &p0, const Vec3f &p1 ); // computes the squared euclidian distance between 2 points in space
	float                  distancePointPlane( const math::Vec3f &point, const Vec3f &normal, const float &distance ); // computes the distance of a point to a given plane
	float                             distancePointLine( const math::Vec3f &point, const Vec3f &p1, const Vec3f &p2 ); // returns the distance of the given point to the line specified by two points
	float             distancePointTriangle( const Vec3f &point, const Vec3f &p1, const Vec3f &p2, const Vec3f &p3  ); // returns distance to the closest point on triangle given


	math::Vec3f     projectPointOnPlane( const math::Vec3f &normal, const float &distance, const math::Vec3f &point ); // returns the projection of the given point on the normal and distance specified plane
	math::Vec3f          projectPointOnLine( const math::Vec3f &point, const math::Vec3f &p1, const math::Vec3f &p2 );

	//
	// Misc mathematical utilities
	//

	//
	//
	//
	float mapValueToRange( const float &sourceRangeMin, const float &sourceRangeMax, const float &targetRangeMin, const float &targetRangeMax, const float &value );
	float mapValueTo0_1( const float &sourceRangeMin, const float &sourceRangeMax, const float &value );

	template<typename T, typename R>
	inline T lerp( T x0, T x1, R t )
	{
		return T(x0*((R)(1.0)-t) + x1*t);
	}


	template<typename T>
	inline T max( T x, T y )
	{
		return x > y ? x : y;
	}

	template<typename T>
	inline T min( T x, T y )
	{
		return x < y ? x : y;
	}

	template<typename T>
	inline T clamp( T x, T _min, T _max )
	{
		return min( _max, max( _min, x ) );
	}

	template<typename T>
	inline T step( T x, T edge )
	{
		return x < edge ? T(0.0) : T(1.0);
	}

	Vec3f slerp( Vec3f v0, Vec3f v1, float t  );
	float clamp( float x, float left, float right );
	float smoothstep( float x );
	float smoothstep(float edge0, float edge1, float x);
	void evalCatmullRom( const float *keyPos, const float *keyT, int num, int dim, float t, float *v );
	void evalLinear( const float *keyPos, const float *keyT, int num, int dim, float t, float *v );


	inline float sRGBToLinear( float c_srgb )
	{
		const float a = 0.055f;
		float c_srgb_clamped = clamp( c_srgb, 0.0f, 1.0f );

		if( c_srgb_clamped <= 0.04045f )
			return c_srgb_clamped/12.92f;
		else
			return pow( (c_srgb_clamped+a)/(1.0f+a), 2.4f);
	}


	inline float linearToSRGB( float c_linear )
	{
		if( c_linear <= 0.0031308f )
			return 12.92f*c_linear;
		else
		{
			const float a = 0.055f;
			return (1.0f+a)*pow( c_linear, 1.0f/2.4f ) - a;
		}
	}

	inline bool quadratic(float A, float B, float C, float *t0, float *t1)
	{
		// Find quadratic discriminant
		float discrim = B * B - 4.f * A * C;
		if (discrim <= 0.) return false;
		float rootDiscrim = sqrtf(discrim);

		// Compute quadratic _t_ values
		float q;
		if (B < 0) q = -.5f * (B - rootDiscrim);
		else       q = -.5f * (B + rootDiscrim);
		*t0 = q / A;
		*t1 = C / q;
		if (*t0 > *t1) std::swap(*t0, *t1);
		return true;
	}

}

} // namespace houio
