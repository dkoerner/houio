/*---------------------------------------------------------------------



----------------------------------------------------------------------*/

#pragma once

#include "Matrix44.h"
#include "Vec3Algo.h"
#include <cassert>

namespace math
{
	template <typename T>
	void extractEulerXYZ (const Matrix44<T> &mat, Vec3<T> &rot)
	{
	    //
		// Normalize the local x, y and z axes to remove scaling.
		//
		Vec3<T> i (mat.m[0][0], mat.m[0][1], mat.m[0][2]);
		Vec3<T> j (mat.m[1][0], mat.m[1][1], mat.m[1][2]);
		Vec3<T> k (mat.m[2][0], mat.m[2][1], mat.m[2][2]);

		i.normalize();
		j.normalize();
		k.normalize();

		Matrix44<T> M(i[0], i[1], i[2], 0,
							j[0], j[1], j[2], 0,
							k[0], k[1], k[2], 0,
							0, 0, 0, 1);

		//
		// Extract the first angle, rot.x.
		//

		rot.x = atan2(M.m[1][2], M.m[2][2]);

		//
		// Remove the rot.x rotation from M, so that the remaining
		// rotation, N, is only around two axes, and gimbal lock
		// cannot occur.
		//
		Matrix44<T> N = Matrix44<T>::Identity();
		//N.rotate (Vec3<T> (-rot.x, 0, 0));
		N.rotateX(-rot.x);
		//N.rotateY(rot.y);
		//N.rotateZ(rot.z);
		N = N * M;

		//
		// Extract the other two angles, rot.y and rot.z, from N.
		//

		T cy = sqrt (N.m[0][0]*N.m[0][0] + N.m[0][1]*N.m[0][1]);
		rot.y = atan2 (-N.m[0][2], cy);
		rot.z = atan2 (-N.m[1][0], N.m[1][1]);
	}


	// m has to be pure rotation matrix
	//http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/index.htm
	//matrix access has been transposed because of row major matrix layout
	template <typename T>
	void extractAxisAngle( const Matrix44<T> &m, Vec3<T> &axis, T &angle )
	{
		T x,y,z; // variables for result
		T epsilon = (T)0.01; // margin to allow for rounding errors
		T epsilon2 = (T)0.1; // margin to distinguish between 0 and 180 degrees
		// optional check that input is pure rotation, 'isRotationMatrix' is defined at:
		// http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/
		//assert isRotationMatrix(m) : "not valid rotation matrix" ;// for debugging
		if ((abs(m.m[1][0]-m.m[0][1])< epsilon)
			&& (abs(m.m[2][0]-m.m[0][2])< epsilon)
			&& (abs(m.m[2][1]-m.m[1][2])< epsilon))
		{
			// singularity found
			// first check for identity matrix which must have +1 for all terms
			//  in leading diagonaland zero in other terms
			if ((abs(m.m[1][0]+m.m[0][1]) < epsilon2)
				&& (abs(m.m[2][0]+m.m[0][2]) < epsilon2)
				&& (abs(m.m[2][1]+m.m[1][2]) < epsilon2)
				&& (abs(m.m[0][0]+m.m[1][1]+m.m[2][2]-3) < epsilon2))
			{
				// this singularity is identity matrix so angle = 0
				axis = math::Vec3<T>((T)1.0,(T)0.0,(T)0.0);
				angle = (T)0;
				// zero angle, arbitrary axis
			}
			// otherwise this singularity is angle = 180
			angle = (T)MATH_PI;
			T xx = (m.m[0][0]+1)/2;
			T yy = (m.m[1][1]+1)/2;
			T zz = (m.m[2][2]+1)/2;
			T xy = (m.m[1][0]+m.m[0][1])/4;
			T xz = (m.m[2][0]+m.m[0][2])/4;
			T yz = (m.m[2][1]+m.m[1][2])/4;
			if ((xx > yy) && (xx > zz))
			{ // m[0][0] is the largest diagonal term
				if (xx< epsilon) {
					x = 0;
					y = (T)0.7071;
					z = (T)0.7071;
				} else {
					x = sqrt(xx);
					y = xy/x;
					z = xz/x;
				}
			} else if (yy > zz) { // m[1][1] is the largest diagonal term
				if (yy< epsilon) {
					x = (T)0.7071;
					y = (T)0;
					z = (T)0.7071;
				} else {
					y = sqrt(yy);
					x = xy/y;
					z = yz/y;
				}	
			} else { // m[2][2] is the largest diagonal term so base result on this
				if (zz< epsilon) {
					x = (T)0.7071;
					y = (T)0.7071;
					z = (T)0;
				} else {
					z = sqrt(zz);
					x = xz/z;
					y = yz/z;
				}
			}

			axis = Vec3<T>(x, y, z);
		}
		// as we have reached here there are no singularities so we can handle normally
		T s = sqrt((m.m[1][2] - m.m[2][1])*(m.m[1][2] - m.m[2][1])
					+(m.m[2][0] - m.m[0][2])*(m.m[2][0] - m.m[0][2])
					+(m.m[0][1] - m.m[1][0])*(m.m[0][1] - m.m[1][0])); // used to normalise
		if (abs(s) < (T)0.001)
			s=(T)1; 
		// prevent divide by zero, should not happen if matrix is orthogonal and should be
		// caught by singularity test above, but I've left it in just in case
		angle = acos(( m.m[0][0] + m.m[1][1] + m.m[2][2] - 1)/2);
		x = (m.m[1][2] - m.m[2][1])/s;
		y = (m.m[2][0] - m.m[0][2])/s;
		z = (m.m[0][1] - m.m[1][0])/s;
		axis = Vec3<T>(x, y, z);
	}

















	template<typename T>
	inline Matrix44<T> operator+( const Matrix44<T> &lhs, const Matrix44<T> &rhs )
	{
		return Matrix44<T>(   lhs._11+rhs._11, lhs._12+rhs._12, lhs._13+rhs._13, lhs._14+rhs._14,
							lhs._21+rhs._21, lhs._22+rhs._22, lhs._23+rhs._23, lhs._24+rhs._24,
							lhs._31+rhs._31, lhs._32+rhs._32, lhs._33+rhs._33, lhs._34+rhs._34,
							lhs._41+rhs._41, lhs._42+rhs._42, lhs._43+rhs._43, lhs._44+rhs._44 );
	}

	template<typename T>
	inline Matrix44<T> operator-( const Matrix44<T> &lhs, const Matrix44<T> &rhs )
	{
		return Matrix44<T>(   lhs._11-rhs._11, lhs._12-rhs._12, lhs._13-rhs._13, lhs._14-rhs._14,
							lhs._21-rhs._21, lhs._22-rhs._22, lhs._23-rhs._23, lhs._24-rhs._24,
							lhs._31-rhs._31, lhs._32-rhs._32, lhs._33-rhs._33, lhs._34-rhs._34,
							lhs._41-rhs._41, lhs._42-rhs._42, lhs._43-rhs._43, lhs._44-rhs._44 );
	}


	template<typename T>
	inline Matrix44<T> operator+( const Matrix44<T> &lhs, const float &rhs )
	{
		Matrix44<T> result;
		result = lhs;

		result._11 += rhs;
		result._12 += rhs;
		result._13 += rhs;
		result._14 += rhs;
		result._21 += rhs;
		result._22 += rhs;
		result._23 += rhs;
		result._24 += rhs;
		result._31 += rhs;
		result._32 += rhs;
		result._33 += rhs;
		result._34 += rhs;
		result._41 += rhs;
		result._42 += rhs;
		result._43 += rhs;
		result._44 += rhs;

		return result;
	}

	template<typename T>
	inline Matrix44<T> operator-( const Matrix44<T> &lhs, const float &rhs )
	{
		Matrix44<T> result;
		result = lhs;

		result._11 -= rhs;
		result._12 -= rhs;
		result._13 -= rhs;
		result._14 -= rhs;
		result._21 -= rhs;
		result._22 -= rhs;
		result._23 -= rhs;
		result._24 -= rhs;
		result._31 -= rhs;
		result._32 -= rhs;
		result._33 -= rhs;
		result._34 -= rhs;
		result._41 -= rhs;
		result._42 -= rhs;
		result._43 -= rhs;
		result._44 -= rhs;

		return result;
	}

	template<typename T>
	inline Matrix44<T> operator*( const Matrix44<T> &lhs, const float &rhs )
	{
		Matrix44<T> result;
		result = lhs;

		result._11 *= rhs;
		result._12 *= rhs;
		result._13 *= rhs;
		result._14 *= rhs;
		result._21 *= rhs;
		result._22 *= rhs;
		result._23 *= rhs;
		result._24 *= rhs;
		result._31 *= rhs;
		result._32 *= rhs;
		result._33 *= rhs;
		result._34 *= rhs;
		result._41 *= rhs;
		result._42 *= rhs;
		result._43 *= rhs;
		result._44 *= rhs;

		return result;
	}

	template<typename T>
	inline Matrix44<T> operator/( const Matrix44<T> &lhs, const float &rhs )
	{
		Matrix44<T> result;
		result = lhs;

		result._11 /= rhs;
		result._12 /= rhs;
		result._13 /= rhs;
		result._14 /= rhs;
		result._21 /= rhs;
		result._22 /= rhs;
		result._23 /= rhs;
		result._24 /= rhs;
		result._31 /= rhs;
		result._32 /= rhs;
		result._33 /= rhs;
		result._34 /= rhs;
		result._41 /= rhs;
		result._42 /= rhs;
		result._43 /= rhs;
		result._44 /= rhs;

		return result;
	}

	template<typename T>
	inline Matrix44<T> operator+( const float &lhs, const Matrix44<T> &rhs )
	{
		return (rhs+lhs);
	}

	template<typename T>
	inline Matrix44<T> operator-( const float &lhs, const Matrix44<T> &rhs )
	{
		return (rhs-lhs);
	}

	template<typename T>
	inline Matrix44<T> operator*( const float &lhs, const Matrix44<T> &rhs )
	{
		return (rhs*lhs);
	}

	template<typename T>
	inline Matrix44<T> operator/( const float &lhs, const Matrix44<T> &rhs )
	{
		return (rhs/lhs);
	}

	template<typename T>
	inline Matrix44<T> operator-( const Matrix44<T> &rhs )
	{
		return Matrix44<T>( -rhs._11, -rhs._12, -rhs._13, -rhs._13,
						  -rhs._21, -rhs._22, -rhs._23, -rhs._23,
						  -rhs._31, -rhs._32, -rhs._33, -rhs._33,
						  -rhs._41, -rhs._42, -rhs._43, -rhs._43);
	}

	template<typename T>
	inline void matrixMultiply( Matrix44<T> &result, const Matrix44<T> &lhs, const Matrix44<T> &rhs )
	{
		result._11 = lhs._11*rhs._11 + lhs._12*rhs._21 + lhs._13*rhs._31 + lhs._14*rhs._41;
		result._12 = lhs._11*rhs._12 + lhs._12*rhs._22 + lhs._13*rhs._32 + lhs._14*rhs._42;
		result._13 = lhs._11*rhs._13 + lhs._12*rhs._23 + lhs._13*rhs._33 + lhs._14*rhs._43;
		result._14 = lhs._11*rhs._14 + lhs._12*rhs._24 + lhs._13*rhs._34 + lhs._14*rhs._44;

		result._21 = lhs._21*rhs._11 + lhs._22*rhs._21 + lhs._23*rhs._31 + lhs._24*rhs._41;
		result._22 = lhs._21*rhs._12 + lhs._22*rhs._22 + lhs._23*rhs._32 + lhs._24*rhs._42;
		result._23 = lhs._21*rhs._13 + lhs._22*rhs._23 + lhs._23*rhs._33 + lhs._24*rhs._43;
		result._24 = lhs._21*rhs._14 + lhs._22*rhs._24 + lhs._23*rhs._34 + lhs._24*rhs._44;

		result._31 = lhs._31*rhs._11 + lhs._32*rhs._21 + lhs._33*rhs._31 + lhs._34*rhs._41;
		result._32 = lhs._31*rhs._12 + lhs._32*rhs._22 + lhs._33*rhs._32 + lhs._34*rhs._42;
		result._33 = lhs._31*rhs._13 + lhs._32*rhs._23 + lhs._33*rhs._33 + lhs._34*rhs._43;
		result._34 = lhs._31*rhs._14 + lhs._32*rhs._24 + lhs._33*rhs._34 + lhs._34*rhs._44;

		result._41 = lhs._41*rhs._11 + lhs._42*rhs._21 + lhs._43*rhs._31 + lhs._44*rhs._41;
		result._42 = lhs._41*rhs._12 + lhs._42*rhs._22 + lhs._43*rhs._32 + lhs._44*rhs._42;
		result._43 = lhs._41*rhs._13 + lhs._42*rhs._23 + lhs._43*rhs._33 + lhs._44*rhs._43;
		result._44 = lhs._41*rhs._14 + lhs._42*rhs._24 + lhs._43*rhs._34 + lhs._44*rhs._44;
	}

	template<typename T>
	inline Vec3<T> transform(  const Vec3<T> &lhs, const Matrix44<T> &rhs )
	{
		return Vec3<T>(lhs.x*rhs._11 + lhs.y*rhs._21 + lhs.z*rhs._31 + rhs._41,
					 lhs.x*rhs._12 + lhs.y*rhs._22 + lhs.z*rhs._32 + rhs._42,
					 lhs.x*rhs._13 + lhs.y*rhs._23 + lhs.z*rhs._33 + rhs._43 );
	}

	template<typename T>
	inline void transform( Vec3<T> &result, const Vec3<T> &lhs, const Matrix44<T> &rhs )
	{
		result.x = lhs.x*rhs._11 + lhs.y*rhs._21 + lhs.z*rhs._31 + rhs._41;
		result.y = lhs.x*rhs._12 + lhs.y*rhs._22 + lhs.z*rhs._32 + rhs._42;
		result.z = lhs.x*rhs._13 + lhs.y*rhs._23 + lhs.z*rhs._33 + rhs._43;
	}

	
	// same as transform
	template<typename T>
	inline Vec3<T> operator*( const Vec3<T> &lhs, const Matrix44<T> &rhs )
	{
		return Vec3<T>(lhs.x*rhs._11 + lhs.y*rhs._21 + lhs.z*rhs._31 + rhs._41,
					 lhs.x*rhs._12 + lhs.y*rhs._22 + lhs.z*rhs._32 + rhs._42,
					 lhs.x*rhs._13 + lhs.y*rhs._23 + lhs.z*rhs._33 + rhs._43 );
	}


	template<typename T>
	inline Matrix44<T> &transpose( math::Matrix44<T> &matrix )
	{
		matrix.transpose();
		return matrix;
	}

	template<typename T>
	inline Matrix44<T> operator*( const Matrix44<T> &lhs, const Matrix44<T> &rhs )
	{
		Matrix44<T> result;
		matrixMultiply( result, lhs, rhs );
		return result;
	}

	template<typename T>
	inline T frobeniusNorm( const Matrix44<T> &lhs )
	{
		float result = (T)0.0;
		result += lhs._11*lhs._11;
		result += lhs._12*lhs._12;
		result += lhs._13*lhs._13;
		result += lhs._14*lhs._14;
		result += lhs._21*lhs._21;
		result += lhs._22*lhs._22;
		result += lhs._23*lhs._23;
		result += lhs._24*lhs._24;
		result += lhs._31*lhs._31;
		result += lhs._32*lhs._32;
		result += lhs._33*lhs._33;
		result += lhs._34*lhs._34;
		result += lhs._41*lhs._41;
		result += lhs._42*lhs._42;
		result += lhs._43*lhs._43;
		result += lhs._44*lhs._44;
		return sqrt(result);
	}

	//
	// creates the matrix which descripes _only_ the orientation which comes from a
	// lookat constrain
	//
	// upVector must be normalized
	//
	template<typename T>
	Matrix44<T> createLookAtMatrix( const Vec3<T> &position, const Vec3<T> &target, const Vec3<T> &_up, bool orientationOnly )
	{
		// we negate forward because in opengl the cam looks down the negative z axis
		math::Vec3<T> forward = -normalize( target - position );

		math::Vec3<T> right = crossProduct( _up, forward );

		math::Vec3<T> up = crossProduct( right, forward );

		math::Matrix44<T> m = math::Matrix44<T>( right, up, forward );
		if( orientationOnly )
			return m;

		return math::Matrix44<T>::TranslationMatrix(math::Vec3<T>((T)0.0, (T)0.0, position.getLength()))*m;
	}

	// creates a transform matrix from the given polar coordinates
	// axis argument tells which axis should be up vector (usefull when building camera transform matrices where y needs to be up)
	template<typename T>
	Matrix44<T> createMatrixFromPolarCoordinates( const T &azimuth, const T &elevation, const T &distance, int axis = 2, bool orientationOnly = false )
	{
		Matrix44<T> m = Matrix44<T>::Identity();

		switch( axis )
		{
		case 0:break; // TODO!:
		case 1:
			{
				if(!orientationOnly)
					m.translate( Vec3<T>( (T)0.0, (T)distance, 0.0f ) );

				m.rotateZ( elevation );
				m.rotateY( azimuth );

				return Matrix44<T>( -m.getDir(), -m.getRight(), m.getUp(), m.getTranslation() );
			}break;
		default:
		case 2:
			{
				if(!orientationOnly)
					m.translate( Vec3<T>( (T)0.0, (T)distance, 0.0f ) );

				m.rotateZ( elevation );
				m.rotateY( azimuth );
				//m.rotateZ( twist );
			}break;
		};


		return m;
	}




	// generates a transform matrix with v as the z axis, taken from PBRT
	template<typename T>
	inline void basisFromVector(const Vec3<T>& w, Vec3<T>* v, Vec3<T>* u)
	{
		if (abs(w.x) > abs(w.y))
		{
			T invLen = (T)(1.0f / sqrt(w.x*w.x + w.z*w.z));
			*u = Vec3<T>(-w.z*invLen, 0.0f, w.x*invLen);
		}
		else
		{
			T invLen = (T)(1.0f / sqrt(w.y*w.y + w.z*w.z));
			*u = Vec3<T>(0.0f, w.z*invLen, -w.y*invLen);
		}

		*v = cross(*u,w);

		assert(abs(length(*u)-1.0f) < 0.01f);
		assert(abs(length(*v)-1.0f) < 0.01f);
	}

	// same as above but returns a matrix
	template<typename T>
	inline Matrix44<T> transformFromVector(const Vec3<T>& w, const Vec3<T>& t=Vec3<T>(0.0f, 0.0f, 0.0f))
	{
		Matrix44<T> m = Matrix44<T>::Identity();
		m.setDir( Vec3<T>(w.x, w.y, w.z));
		m.setTranslation(Vec3<T>(t.x, t.y, t.z));

		basisFromVector(w, (Vec3<T>*)m.m[0], (Vec3<T>*)m.m[1]);

		return m;
	}

	template<typename T>
	inline math::Matrix44<T> orthographicProjectionMatrix( T left, T right, T bottom, T top, T znear, T zfar )
	{
		return math::Matrix44<T>( (T)2.0/(right-left), (T)0.0, (T)0.0, (T)0.0,
								(T)0.0, (T)2.0/(top-bottom), (T)0.0, (T)0.0,
								(T)0.0, (T)0.0, (T)-2.0/(zfar-znear), (T)0.0,
								(T)-(right+left)/(right-left), (T)-(top+bottom)/(top-bottom), (T)-(zfar+znear)/(zfar-znear), (T)1.0 );
	}

	template<typename T>
	inline Matrix44<T> projectionMatrix( T fov, T aspectRatio, T znear, T zfar )
	{
		Matrix44<T> m = Matrix44<T>::Identity();

		//calculate width and height at 1 unit in front of the camera
		T height = 2 * tanf( T(fov/2) );
		T width = height * aspectRatio;


		//create projection matrix
		T sc = ( zfar + znear ) / ( zfar - znear );
		T of = 2 * zfar * znear / ( zfar - znear );
		m.ma[ 0] = 2/width;
		m.ma[ 1] = 0;
		m.ma[ 2] = 0;
		m.ma[ 3] = 0;
		m.ma[ 4] = 0;
		m.ma[ 5] = 2/height;
		m.ma[ 6] = 0;
		m.ma[ 7] = 0;
		m.ma[ 8] = 0;
		m.ma[ 9] = 0;
		m.ma[10] = -sc;
		m.ma[11] = -1;
		m.ma[12] = 0;
		m.ma[13] = 0;
		m.ma[14] = -of;
		m.ma[15] = 0;

		return m;
	}
}

