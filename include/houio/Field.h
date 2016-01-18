#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <memory>
#include <cstring>

#include <houio/math/Math.h>

namespace houio
{

	template<typename T>
	struct Field
	{
		typedef std::shared_ptr< Field<T> > Ptr;
		typedef std::shared_ptr< const Field<T> > CPtr;

		static Ptr create( math::V3i res = math::V3i(10), math::M44f l2w = math::M44f() );
		static Ptr       create( math::V3i res, math::Box3f bound, T initialValue = T() );
		template<typename R> static Ptr                 create( typename Field<R>::Ptr t);
		static Ptr                                    load( const std::string &filename );

		Field();

		T                                           evaluate( const math::V3f &vsP )const;
		T                                              sample( int i, int j, int k )const;
		T                                                  &lvalue( int i, int j, int k );

		math::V3i                                                    getResolution()const;
		void                                                resize( int x, int y, int z );
		void                                               resize( math::V3i resolution );

		void                            setLocalToWorld( const math::M44f &localToWorld );
		void                                         setBound( const math::Box3f &bound );

		math::Box3f                                                          bound()const; // returns bounding box in worldspace
		math::V3f                                                     getVoxelSize()const; // returns voxelsize in worldspace

		math::V3f                               worldToLocal( const math::V3f &wsP )const; // converts given worldspace position to localspace
		math::V3f                               localToWorld( const math::V3f &lsP )const; // converts given localspace position to worldspace
		math::V3f                               worldToVoxel( const math::V3f &wsP )const; // converts given worldspace position to voxelspace
		math::V3f                               voxelToWorld( const math::V3f &vsP )const; // converts given voxelspace position to worldspace
		math::V3f                               localToVoxel( const math::V3f &lsP )const; // converts given localspace position to voxelspace
		math::V3f                               voxelToLocal( const math::V3f &vsP )const; // converts given voxelspace position to localspace

		T                                                                *getRawPointer();
		const T                                                     *getRawPointer()const;


		// utility functions ---
		void                                                               fill( T value ); // fills all voxels with the same value
		void                                   fill( T value, const math::Box3f &wsBound ); // fills all voxels with the same value within given (worldspace)bound
		void                                                           multiply( T value ); // multiplies all voxelswith given value
		void                                          store( const std::string &filename ); // saves field to file
		void                        storeWithoutBoundingBox( const std::string &filename ); // saves field to file



	//private:
		math::M44f                                                          m_localToWorld; // transforms 0-1 space to world space
		math::M44f                                                          m_worldToLocal;
		math::M44f                                                          m_worldToVoxel; // transforms from worldspace to voxelspace
		math::M44f                                                          m_voxelToWorld;

		math::V3f                                                           m_sampleLocation; // sample location within one voxel (allows staggered grids etc.)

		math::V3i                                                             m_resolution;

		math::Box3f                                                                m_bound;

		std::vector<T>                                                              m_data;

		static const int                                                        m_dataType; // e.g. float, double, v3f etc.
	};


	template<typename T>
	typename Field<T>::Ptr Field<T>::create( math::V3i res, math::M44f l2w )
	{
		Field<T>::Ptr field = Field<T>::Ptr(new Field<T>());
		field->resize(res);
		field->setLocalToWorld(l2w);
		return field;
	}

	template<typename T>
	typename Field<T>::Ptr Field<T>::create( math::V3i res, math::Box3f bound, T initialValue )
	{
		Field<T>::Ptr field = Field<T>::Ptr(new Field<T>());
		field->resize(res);
		field->setBound(bound);
		field->fill( initialValue );
		return field;
	}


	template<typename T>
	template<typename R>
	typename Field<T>::Ptr Field<T>::create( typename Field<R>::Ptr src)
	{
		Field<T>::Ptr dst = Field<T>::create( src->getResolution(), src->bound() );
		typename std::vector<R>::iterator srcIt = src->m_data.begin();
		typename std::vector<T>::iterator dstIt = dst->m_data.begin();
		typename std::vector<T>::iterator dstEnd = dst->m_data.end();
		for( ; dstIt != dstEnd; ++dstIt, ++srcIt )
			*dstIt = (T)*srcIt;
		return dst;
	}

	template<typename T>
	Field<T>::Field() : m_sampleLocation(0.5f)
	{
		resize( math::V3i(1) );
		setLocalToWorld( math::M44f::Identity() );
	}

	template<typename T>
	typename Field<T>::Ptr Field<T>::load( const std::string &filename )
	{
		Field<T>::Ptr field = Field<T>::create();

		// load bound, resolution, data from file
		std::ifstream in( filename.c_str(), std::ios_base::in | std::ios_base::binary );

		if( !in.good() )
			return field;

		in.read( (char *)&field->m_resolution, sizeof(int)*3 );
		in.read( (char *)&field->m_bound, sizeof(float)*6 );
		int dataType = 0;
		in.read( (char *)&dataType, sizeof(int) );

		if( dataType != m_dataType )
		{
			std::cerr << "Field<T>::load: error: datatype in " << filename << " doesnt match.\n";
			return Field<T>::Ptr();
		}

		int size = field->m_resolution.x*field->m_resolution.y*field->m_resolution.z;
		field->m_data.resize( size );
		in.read( (char *)&field->m_data[0], size*sizeof(T) );

		// need to to this to trigger update of matrices
		field->setBound( field->m_bound );

		return field;
	}

	template<typename T>
	void Field<T>::store( const std::string &filename )
	{
		std::ofstream out( filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );

		// save bound, resolution, data to file
		out.write( (const char *)&m_resolution, sizeof(int)*3 );
		math::Box3f b = bound();
		out.write( (const char *)&b, sizeof(float)*6 );
		out.write( (const char *)&m_dataType, sizeof(int) );
		out.write( (const char *)&m_data[0], sizeof(T)*m_resolution.x*m_resolution.y*m_resolution.z );
	}

	template<typename T>
	void Field<T>::storeWithoutBoundingBox( const std::string &filename )
	{
		std::ofstream out( filename.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );

		// save bound, resolution, data to file
		out.write( (const char *)&m_resolution, sizeof(int)*3 );
		out.write( (const char *)&m_dataType, sizeof(int) );
		out.write( (const char *)&m_data[0], sizeof(T)*m_resolution.x*m_resolution.y*m_resolution.z );
	}

	template<typename T>
	void Field<T>::resize( int x, int y, int z )
	{
		resize(math::V3i(x, y, z));
	}


	template<typename T>
	void Field<T>::resize( math::V3i resolution )
	{
		m_resolution = resolution;
		m_data.resize(m_resolution.x*m_resolution.y*m_resolution.z);
		memset( &m_data[0], 0, m_resolution.x*m_resolution.y*m_resolution.z*sizeof(T));
		m_worldToVoxel = m_worldToLocal*math::M44f().scale( math::V3f(m_resolution) );
		m_voxelToWorld = m_worldToVoxel.inverse();
	}

	template<typename T>
	T Field<T>::sample( int i, int j, int k )const
	{
		return m_data[k*m_resolution.x*m_resolution.y + j*m_resolution.x + i];
	}

	template<typename T>
	T &Field<T>::lvalue( int i, int j, int k )
	{
		return m_data[k*m_resolution.x*m_resolution.y + j*m_resolution.x + i];
	}

	template<typename T>
	T Field<T>::evaluate( const math::V3f &vsP )const
	{
		typedef float real_t;
		typedef math::Vec3<real_t> Vector;

		Vector vs = vsP;

		// take sample location within voxel into account
		vs -= m_sampleLocation;

		real_t tx = vs.x - floor(vs.x);
		real_t ty = vs.y - floor(vs.y);
		real_t tz = vs.z - floor(vs.z);

		// lower left corner
		math::V3i c1;
		c1.x = (int)floor(vs.x);
		c1.y = (int)floor(vs.y);
		c1.z = (int)floor(vs.z);

		// upper right corner
		math::V3i c2 = c1+math::V3i(1);
		math::V3i res = getResolution();

		// clamp the indexing coordinates
		c1.x = std::max(0, std::min(c1.x, res.x-1));
		c2.x = std::max(0, std::min(c2.x, res.x-1));
		c1.y = std::max(0, std::min(c1.y, res.y-1));
		c2.y = std::max(0, std::min(c2.y, res.y-1));
		c1.z = std::max(0, std::min(c1.z, res.z-1));
		c2.z = std::max(0, std::min(c2.z, res.z-1));

		//lerp...
		return math::lerp( math::lerp( math::lerp( sample( c1.x, c1.y, c1.z ), sample( c2.x, c1.y, c1.z ), (real_t)tx ), math::lerp( sample( c1.x, c2.y, c1.z ), sample( c2.x, c2.y, c1.z ), (real_t)tx ), (real_t)ty ), math::lerp( math::lerp( sample( c1.x, c1.y, c2.z ), sample( c2.x, c1.y, c2.z ), (real_t)tx ), math::lerp( sample( c1.x, c2.y, c2.z ), sample( c2.x, c2.y, c2.z ), (real_t)tx ), (real_t)ty ), (real_t)tz );
	}

	template<typename T>
	math::V3i Field<T>::getResolution()const
	{
		return m_resolution;
	}

	// returns voxelsize in worldspace
	template<typename T>
	math::V3f Field<T>::getVoxelSize()const
	{
		math::V3f v0 = voxelToWorld( math::V3f(0.0f,0.0f, 0.0f) );
		math::V3f v1 = voxelToWorld( math::V3f(1.0f,1.0f, 1.0f) );
		return math::V3f( v1.x - v0.x, v1.y - v0.y, v1.z - v0.z );
	}

	template<typename T>
	T *Field<T>::getRawPointer()
	{
		return &m_data[0];
	}

	template<typename T>
	const T *Field<T>::getRawPointer()const
	{
		return &m_data[0];
	}

	template<typename T>
	void Field<T>::setLocalToWorld( const math::M44f &localToWorld )
	{
		m_localToWorld = localToWorld;
		m_worldToLocal = m_localToWorld.inverse();
		m_worldToVoxel = m_worldToLocal*math::M44f().scale( math::V3f((float)m_resolution.x, (float)m_resolution.y, (float)m_resolution.z) );
		m_voxelToWorld = m_worldToVoxel.inverse();

		m_bound.makeEmpty();
		m_bound.extendBy( math::V3f( 0.0f, 0.0f, 0.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 1.0f, 0.0f, 0.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 0.0f, 1.0f, 0.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 1.0f, 1.0f, 0.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 0.0f, 0.0f, 1.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 1.0f, 0.0f, 1.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 0.0f, 1.0f, 1.0f ) * m_localToWorld );
		m_bound.extendBy( math::V3f( 1.0f, 1.0f, 1.0f ) * m_localToWorld );
	}

	template<typename T>
	void Field<T>::setBound( const math::Box3f &bound )
	{
		m_bound = bound;
		math::V3f dim = m_bound.size();

		m_localToWorld = math::M44f().scale( math::V3f(dim.x, dim.y, dim.z) )*math::M44f().translate(m_bound.minPoint);
		m_worldToLocal = m_localToWorld.inverse();
		m_worldToVoxel = m_worldToLocal*math::M44f().scale( math::V3f((float)m_resolution.x, (float)m_resolution.y, (float)m_resolution.z) );
		m_voxelToWorld = m_worldToVoxel.inverse();
	}

	// converts given worldspace position to voxelspace
	template<typename T>
	math::V3f Field<T>::worldToVoxel( const math::V3f &wsP )const
	{
		return wsP * m_worldToVoxel;
	}

	// converts given voxelspace position to worldspace
	template<typename T>
	math::V3f Field<T>::voxelToWorld( const math::V3f &vsP )const
	{
		return vsP * m_voxelToWorld;
	}


	// converts given worldspace position to localspace
	template<typename T>
	math::V3f Field<T>::worldToLocal( const math::V3f &wsP )const
	{
		return wsP * m_worldToLocal;
	}

	// converts given localspace position to worldspace
	template<typename T>
	math::V3f Field<T>::localToWorld( const math::V3f &lsP )const
	{
		return lsP * m_localToWorld;
	}

	// converts given voxelspace position to localspace
	template<typename T>
	math::V3f Field<T>::voxelToLocal( const math::V3f &vsP )const
	{
		return vsP * math::M44f::ScaleMatrix( 1.0f/float(m_resolution.x), 1.0f/float(m_resolution.y), 1.0f/float(m_resolution.z) );
	}

	// converts given localspace position to voxelspace
	template<typename T>
	math::V3f Field<T>::localToVoxel( const math::V3f &lsP )const
	{
		return lsP * math::M44f::ScaleMatrix( float(m_resolution.x), float(m_resolution.y), float(m_resolution.z) );
	}


	// returns bounding box in worldspace
	template<typename T>
	math::Box3f Field<T>::bound()const
	{
		return m_bound;
	}

	// fills all voxels with the same value
	template<typename T>
	void Field<T>::fill( T value )
	{
		for( typename std::vector<T>::iterator it = m_data.begin(), end = m_data.end(); it != end; ++it )
			*it = value;
	}

	template<typename T>
	void Field<T>::fill( T value, const math::Box3f &wsBound )
	{
		for( int k=0;k<m_resolution.z;++k )
			for( int j=0;j<m_resolution.y;++j )
				for( int i=0;i<m_resolution.x;++i )
				{
					math::V3f vsP( i+0.5f, j+0.5f, k+0.5f );
					math::V3f wsP = voxelToWorld( vsP );
					if( wsBound.encloses(wsP) )
						lvalue(i, j, k) = value;
				}
	}


	// multiplies all voxelswith given value
	template<typename T>
	void Field<T>::multiply( T value )
	{
		for( typename std::vector<T>::iterator it = m_data.begin(), end = m_data.end(); it != end; ++it )
			*it *= value;
	}


	// get maximum value
	template<typename T>
	T field_maximum( const Field<T> &field )
	{
		T max = -std::numeric_limits<T>::max();
		const T *ptr = field.getRawPointer();
		math::V3i res = field.getResolution();
		int numVoxels = res.x*res.y*res.z;
		for( int i=0;i<numVoxels;++i )
			max = std::max( ptr[i], max );
		return max;
	}

	template<typename T>
	void field_range( const Field<T> &field, T& min, T& max )
	{
		auto minmax = std::minmax_element( field.m_data.begin(), field.m_data.end() );
		min = *minmax.first;
		max = *minmax.second;
	}

	template<typename T>
	void field_writeplot( const Field<T> &field, math::V3f wsP0, math::V3f wsP1, int numSamples, std::ofstream &out )
	{
		math::Ray3f ray;
		ray.o = wsP0;
		ray.d = (wsP1-wsP0).normalized();
		float d = (wsP1-wsP0).getLength()/float(numSamples);
		for( int i=0;i<numSamples;++i )
		{
			float t = d*float(i);
			math::V3f wsP = ray(t);
			math::V3f vsP = field.worldToVoxel(wsP);
			T value = field.evaluate( vsP );
			out << t << " " << value << std::endl;
		}
	}

	template<typename T>
	void field_writeplot( const Field<math::Vec3<T>> &field, math::V3f wsP0, math::V3f wsP1, int numSamples, std::ofstream &out, int component )
	{
		math::Ray3f ray;
		ray.o = wsP0;
		ray.d = (wsP1-wsP0).normalized();
		float d = (wsP1-wsP0).getLength()/float(numSamples);
		for( int i=0;i<numSamples;++i )
		{
			float t = d*float(i);
			math::V3f wsP = ray(t);
			math::V3f vsP = field.worldToVoxel(wsP);
			math::Vec3<T> value = field.evaluate( vsP );
			out << t << " " << value[component] << std::endl;
		}
	}
	// LinearFieldInterp =================

	template<typename T,typename R = float>
	struct LinearFieldInterp
	{
		// takes field and voxelspace position
		static T eval( const Field<T> &field, const math::V3f &vsP )
		{
			typedef R real_t;
			typedef math::Vec3<real_t> Vector;

			Vector vs = vsP;

			// voxels defined at cell centers
			vs.x -= (real_t)(0.5);
			vs.y -= (real_t)(0.5);
			vs.z -= (real_t)(0.5);

			real_t tx = vs.x - floor(vs.x);
			real_t ty = vs.y - floor(vs.y);
			real_t tz = vs.z - floor(vs.z);

			// lower left corner
			math::V3i c1;
			c1.x = (int)floor(vs.x);
			c1.y = (int)floor(vs.y);
			c1.z = (int)floor(vs.z);

			// upper right corner
			math::V3i c2 = c1+math::V3i(1);
			math::V3i res = field.getResolution();

			// clamp the indexing coordinates
			c1.x = std::max(0, std::min(c1.x, res.x-1));
			c2.x = std::max(0, std::min(c2.x, res.x-1));
			c1.y = std::max(0, std::min(c1.y, res.y-1));
			c2.y = std::max(0, std::min(c2.y, res.y-1));
			c1.z = std::max(0, std::min(c1.z, res.z-1));
			c2.z = std::max(0, std::min(c2.z, res.z-1));

			//lerp...
			return math::lerp( math::lerp( math::lerp( field.sample( c1.x, c1.y, c1.z ), field.sample( c2.x, c1.y, c1.z ), (R)tx ), math::lerp( field.sample( c1.x, c2.y, c1.z ), field.sample( c2.x, c2.y, c1.z ), (R)tx ), (R)ty ), math::lerp( math::lerp( field.sample( c1.x, c1.y, c2.z ), field.sample( c2.x, c1.y, c2.z ), (R)tx ), math::lerp( field.sample( c1.x, c2.y, c2.z ), field.sample( c2.x, c2.y, c2.z ), (R)tx ), (R)ty ), (R)tz );
		}
	};

	typedef Field<float> Fieldf;
	typedef Field<float> ScalarField;
	typedef Field<math::V3f> Field3f;
	typedef Field<math::V3f> VectorField;
	typedef Field<double> Fieldd;
	typedef Field<math::V3d> Field3d;


}


