/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#include <houio/math/Math.h>

namespace houio
{


namespace math
{
	



	/*
	//
	//
	//
	bool rayHitPlaneValues( const Vec3f &planeNormal, const float &planeDistance, Ray &ray, float &hitDistance, Vec3f *hitPoint )
	{
        Vec3f rayOrigin    = ray.getOrigin();
		Vec3f rayDirection = ray.getDirection();//ray.getTarget() - rayOrigin;
		//Vec3f rayOrigin    = Vec3f( 0.0f, 10.0f, 0.0f );
		//Vec3f rayDirection = Vec3f( 0.0f, -1.0f, 0.0f );


		float temp = dotProduct( rayDirection, planeNormal );

		// 
		//if( temp >= 0.0f )
		//	return false;

		hitDistance = -(dotProduct( planeNormal, rayOrigin ) + planeDistance) / temp;

		// the point must lie on the raysegment between origin and target to pass the test
		if( (hitDistance > ray.getLength()) || (hitDistance < 0.0f) )
			return false;

		if( hitPoint )
			*hitPoint = rayOrigin + hitDistance*rayDirection;

		return true;
	}
	*/







	//
	// computes area of an triangle
	//
	float area( const Vec3f &p0, const Vec3f &p1, const Vec3f &p2 )
	{
		float la = (p1 - p0).getLength(); // compute lengths of the triangle sides
		float lb = (p2 - p1).getLength();
		float lc = (p2 - p0).getLength();
		float s = 0.5f*( la+lb+lc ); // compute the semiperimeter
		return sqrt( s*(s-la)*(s-lb)*(s-lc) ); // compute the area
	}



	//
	// returns the distance of the given point to the line specified by two points
	//
	float distancePointLine( const math::Vec3f &point, const Vec3f &p1, const Vec3f &p2 )
	{
		math::Vec3f vec = point - p1;
		math::Vec3f direction = math::normalize( p2 - p1 );

		return (vec - dotProduct( vec, direction  ) * direction).getLength();
	}


	// returns distance to the closest point on triangle given
	float distancePointTriangle( const Vec3f &point, const Vec3f &p1, const Vec3f &p2, const Vec3f &p3  )
	{
		// copied from http://www.mathworks.com/matlabcentral/fileexchange/22857-distance-between-a-point-and-a-triangle-in-3d

		// rewrite triangle in normal form
		math::Vec3f B = p1;
		math::Vec3f E0 = p2-B;
		//E0 = E0/sqrt(sum(E0.^2)); %normalize vector
		math::Vec3f E1 = p3-B;
		//E1 = E1/sqrt(sum(E1.^2)); %normalize vector


		math::Vec3f D = B - point;
		float a = dot(E0,E0);
		float b = dot(E0,E1);
		float c = dot(E1,E1);
		float d = dot(E0,D);
		float e = dot(E1,D);
		float f = dot(D,D);

		float det = a*c - b*b; // do we have to use abs here?
		float s   = b*e - c*d;
		float t   = b*d - a*e;

		float sqrDistance, invDet, tmp0, tmp1, numer, denom;

		// Terible tree of conditionals to determine in which region of the diagram
		// shown above the projection of the point into the triangle-plane lies.
	
		if ((s+t) <= det)
		{
		  if (s < 0)
		  {
			if (t < 0)
			{
			  // region4
			  if (d < 0)
			  {
				t = 0;
				if (-d >= a)
				{
				  s = 1;
				  sqrDistance = a + 2*d + f;
				}else
				{
				  s = -d/a;
				  sqrDistance = d*s + f;
				}
			  }else
			  {
				s = 0;
				if (e >= 0)
				{
				  t = 0;
				  sqrDistance = f;
				}else
				{
				  if (-e >= c)
				  {
					t = 1;
					sqrDistance = c + 2*e + f;
				  }else
				  {
					t = -e/c;
					sqrDistance = e*t + f;
				  }
				}
			   } //end %of region 4
			}else
			{
			  // region 3
			  s = 0;
			  if (e >= 0)
			  {
				t = 0;
				sqrDistance = f;
			  }else
			  {
				if (-e >= c)
				{
				  t = 1;
				  sqrDistance = c + 2*e +f;
				}else
				{
				  t = -e/c;
				  sqrDistance = e*t + f;
				}
			  }
			} //end %of region 3 
		  }else
		  {
			if (t < 0)
			{
			  // region 5
			  t = 0;
			  if (d >= 0)
			  {
				s = 0;
				sqrDistance = f;
			  }else
			  {
				if (-d >= a)
				{
				  s = 1;
				  sqrDistance = a + 2*d + f; // GF 20101013 fixed typo d*s ->2*d
				}else
				{
				  s = -d/a;
				  sqrDistance = d*s + f;
				}
			  }
			}else
			{
			  //% region 0
			  invDet = 1/det;
			  s = s*invDet;
			  t = t*invDet;
			  sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
			}
		  }
		}else
		{
		  if (s < 0)
		  {
			// region 2
			tmp0 = b + d;
			tmp1 = c + e;
			if (tmp1 > tmp0) // minimum on edge s+t=1
			{
			  numer = tmp1 - tmp0;
			  denom = a - 2*b + c;
			  if(numer >= denom)
			  {
				s = 1;
				t = 0;
				sqrDistance = a + 2*d + f; // GF 20101014 fixed typo 2*b -> 2*d
			  }else
			  {
				s = numer/denom;
				t = 1-s;
				sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
			  }
			}else       // minimum on edge s=0
			{
			  s = 0;
			  if (tmp1 <= 0)
			  {
				t = 1;
				sqrDistance = c + 2*e + f;
			  }else
				if (e >= 0)
				{
				  t = 0;
				  sqrDistance = f;
				}else
				{
				  t = -e/c;
				  sqrDistance = e*t + f;
				}
			 } //of region 2
		  }else
		  {
			if (t < 0)
			{
			  //region6 
			  tmp0 = b + e;
			  tmp1 = a + d;
			  if (tmp1 > tmp0)
			  {
				numer = tmp1 - tmp0;
				denom = a-2*b+c;
				if (numer >= denom)
				{
				  t = 1;
				  s = 0;
				  sqrDistance = c + 2*e + f;
				}else
				{
				  t = numer/denom;
				  s = 1 - t;
				  sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
				}
			  }else  
			  {
				t = 0;
				if (tmp1 <= 0)
				{
					s = 1;
					sqrDistance = a + 2*d + f;
				}else
				{
				  if (d >= 0)
				  {
					  s = 0;
					  sqrDistance = f;
				  }else
				  {
					  s = -d/a;
					  sqrDistance = d*s + f;
				  }
				}
			   }
			  //end region 6
			}else
			{
			  // region 1
			  numer = c + e - b - d;
			  if (numer <= 0)
			  {
				s = 0;
				t = 1;
				sqrDistance = c + 2*e + f;
			  }else
			  {
				denom = a - 2*b + c;
				if (numer >= denom)
				{
				  s = 1;
				  t = 0;
				  sqrDistance = a + 2*d + f;
				}else
				{
				  s = numer/denom;
				  t = 1-s;
				  sqrDistance = s*(a*s + b*t + 2*d) + t*(b*s + c*t + 2*e) + f;
				}
			  } // of region 1
			}
		  }
		}



		// account for numerical round-off error
		if (sqrDistance < 0)
		  sqrDistance = 0;

		float dist = sqrt(sqrDistance);

		//if nargout>1
		//  PP0 = B + s*E0 + t*E1;
		//end
		return dist;
	}


	//
	// computes the distance of a point to a plane
	//
	inline float distancePointPlane( const math::Vec3f &point, const Vec3f &normal, const float &distance )
	{
		return dotProduct( normal, point ) + distance;
	}

	//
	// computes the euclidian distance between 2 points in space
	//
	float distance( const Vec3f &p0, const Vec3f &p1 )
	{
		return (p1-p0).getLength();
	}

	//
	// computes the squared euclidian distance between 2 points in space
	//
	inline float squaredDistance( const Vec3f &p0, const Vec3f &p1 )
	{
		return (p1-p0).getSquaredLength();
	}





	//
	// returns the projection of the given point on the normal and distance specified plane
	//
	math::Vec3f projectPointOnPlane( const math::Vec3f &normal, const float &distance, const math::Vec3f &point )
	{
		return point - distancePointPlane( point, normal, distance )*normal;
	}

	math::Vec3f projectPointOnLine( const math::Vec3f &point, const math::Vec3f &p1, const math::Vec3f &p2 )
	{
		math::Vec3f direction = math::normalize( p2 - p1 );

		return p1 + dotProduct( point - p1, direction  ) * direction;
	}













	//
	//
	//
	float mapValueToRange( const float &sourceRangeMin, const float &sourceRangeMax, const float &targetRangeMin, const float &targetRangeMax, const float &value )
	{
		return (value-sourceRangeMin) / (sourceRangeMax - sourceRangeMin) * (targetRangeMax - targetRangeMin) + targetRangeMin;
	}

	//
	//
	//
	float mapValueTo0_1( const float &sourceRangeMin, const float &sourceRangeMax, const float &value )
	{
		return (value-sourceRangeMin) / (sourceRangeMax - sourceRangeMin);
	}








	Vec3f slerp( Vec3f v0, Vec3f v1, float t  )
	{
		 // Dot product - the cosine of the angle between 2 vectors.
		 float dot = dotProduct(v0, v1);
		 // Clamp it to be in the range of Acos()
		 clamp(dot, -1.0f, 1.0f);
		 // Acos(dot) returns the angle between start and end,
		 // And multiplying that by percent returns the angle between
		 // start and the final result.
		 float theta = acos(dot)*t;
		 Vec3f RelativeVec = v1 - v0*dot;
		 RelativeVec.normalize();
		 // The final result.
		 return ((v0*std::cos(theta)) + (RelativeVec*std::sin(theta)));
	}

	float clamp( float x, float left, float right )
	{
		return (x < left) ? left : (x > right ? right : x);
	}

	float smoothstep( float x )
	{
		return (x) * (x) * (3 - 2 * (x));
	}

	float smoothstep(float edge0, float edge1, float x)
	{
		// Scale, and clamp x to 0..1 range
		x = clamp((x - edge0)/(edge1 - edge0), 0.0f, 1.0f);
		// Evaluate polynomial
		return x*x*x*(x*(x*6.0f - 15.0f) + 10.0f);
	}

	static signed char coefs[16] = {
		-1, 2,-1, 0,
		 3,-5, 0, 2,
		-3, 4, 1, 0,
		 1,-1, 0, 0 };

	void evalCatmullRom( const float *keyPos, const float *keyT, int num, int dim, float t, float *v )
	{
		if( t<0.0f )t=0.0f;
		if( t>1.0f )t=1.0f;

		// find key
		int k = 0;while( keyT[k] < t )k++;

		// interpolant
		const float h = (t-keyT[k-1])/(keyT[k]-keyT[k-1]);

		// init result
		for( int i=0; i < dim; i++ ) v[i] = 0.0f;

		// add basis functions
		for( int i=0; i<4; i++ )
		{
			int kn = k+i-2;
			if( kn<0 ) kn=0;
			else if( kn>(num-1) )
				kn=num-1;

			const signed char *co = coefs + 4*i;

			const float b  = 0.5f*(((co[0]*h + co[1])*h + co[2])*h + co[3]);

			for( int j=0; j < dim; j++ ) v[j] += b * keyPos[kn*dim+j];
		}
	}

	void evalLinear( const float *keyPos, const float *keyT, int num, int dim, float t, float *v )
	{
		if( t<0.0f )t=0.0f;
		if( t>1.0f )t=1.0f;

		// find key
		int k = 0;while( (keyT[k] < t)&&(k<num) )k++;
		if(k == num)
		{
			for( int i=0; i < dim; i++ ) v[i] = keyPos[k-1*dim+i];
			return;
		}
		int kn = k-1;

		if(kn < 0)
		{
			for( int i=0; i < dim; i++ ) v[i] = keyPos[0*dim+i];
			return;
		}

		// interpolant
		const float h = (t-keyT[kn])/(keyT[k]-keyT[kn]);

		// init result
		for( int i=0; i < dim; i++ ) v[i] = (1.0f - h)*keyPos[(kn)*dim+i] + h*keyPos[k*dim+i];
	}
}

} // namespace houio
