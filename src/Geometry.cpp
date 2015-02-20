#include <houio/Geometry.h>
#include <houio/math/Math.h>
#include <iostream>


//#include <util/tuple.h>
//#include "util/ObjIO.h"



namespace houio
{
	Geometry::Geometry( PrimitiveType pt ) :
		m_primitiveType(pt),
		m_numPrimitives(0),
		m_indexBufferIsDirty(true)
	{
		switch( pt )
		{
		case Geometry::POINT:m_numPrimitiveVertices = 1;break;
		case Geometry::LINE:m_numPrimitiveVertices = 2;break;
		default:
		case Geometry::TRIANGLE:m_numPrimitiveVertices = 3;break;
		case Geometry::QUAD:m_numPrimitiveVertices = 4;break;
		case Geometry::POLYGON:m_numPrimitiveVertices = 0;break;
		}
	}

	Attribute::Ptr Geometry::getAttr( const std::string &name )
	{
		std::map<std::string, Attribute::Ptr>::iterator it = m_attributes.find(name);
		if(it != m_attributes.end())
			return it->second;
		return Attribute::Ptr();
	}

	void Geometry::setAttr( const std::string &name, Attribute::Ptr attr )
	{
		m_attributes[name] = attr;
	}

	bool Geometry::hasAttr( const std::string &name )
	{
		std::map<std::string, Attribute::Ptr>::iterator it = m_attributes.find(name);
		return(it != m_attributes.end());
	}

	void Geometry::removeAttr( const std::string &name )
	{
		std::map<std::string, Attribute::Ptr>::iterator it = m_attributes.find(name);
		if( it != m_attributes.end() )
			m_attributes.erase( it );
	}

	Geometry::PrimitiveType Geometry::primitiveType()
	{
		return m_primitiveType;
	}

	unsigned int Geometry::numPrimitives()
	{
		return m_numPrimitives;
	}

	// Point=1; Line=2; Triangle=3; Quad=4; polygon=*
	unsigned int Geometry::numPrimitiveVertices()
	{
		return m_numPrimitiveVertices;
	}

	unsigned int Geometry::addPoint( unsigned int vId )
	{
		m_indexBuffer.push_back(vId);
		m_indexBufferIsDirty = true;
		return m_numPrimitives++;
	}

	unsigned int Geometry::addLine( unsigned int vId0, unsigned int vId1 )
	{
		m_indexBuffer.push_back(vId0);
		m_indexBuffer.push_back(vId1);
		m_indexBufferIsDirty = true;
		return m_numPrimitives++;
	}

	unsigned int Geometry::addTriangle( unsigned int vId0, unsigned int vId1, unsigned int vId2 )
	{
		m_indexBuffer.push_back(vId0);
		m_indexBuffer.push_back(vId1);
		m_indexBuffer.push_back(vId2);
		m_indexBufferIsDirty = true;
		return m_numPrimitives++;
	}

	unsigned int Geometry::addQuad( unsigned int vId0, unsigned int vId1, unsigned int vId2, unsigned int vId3 )
	{
		m_indexBuffer.push_back(vId0);
		m_indexBuffer.push_back(vId1);
		m_indexBuffer.push_back(vId2);
		m_indexBuffer.push_back(vId3);
		m_indexBufferIsDirty = true;
		return m_numPrimitives++;
	}

	unsigned int Geometry::addPolygonVertex( unsigned int v )
	{
		m_indexBuffer.push_back(v);
		++m_numPrimitiveVertices;
		m_indexBufferIsDirty = true;
		if( !m_numPrimitives )
			// only 0 or 1 number of primitives allowed with polygons
			m_numPrimitives = 1;
		return m_numPrimitives;
	}



	// reverses the order of vertices for each primitive (CW polys become CCW)
	void Geometry::reverse()
	{
		for( unsigned int i=0;i<m_numPrimitives;++i )
		{
			int offset = i*m_numPrimitiveVertices;
			std::reverse(m_indexBuffer.begin()+offset, m_indexBuffer.begin()+offset+m_numPrimitiveVertices );
		}
		m_indexBufferIsDirty = true;
	}

	// duplicates point and returns index of duplicate (all attributes are copied etc.)
	unsigned int Geometry::duplicatePoint( unsigned int index )
	{
		int newIndex = getAttr("P")->numElements();
		for( std::map< std::string, Attribute::Ptr >::iterator it = m_attributes.begin(); it != m_attributes.end(); ++it )
		{
			Attribute::Ptr attr = it->second;
			attr->resize( newIndex+1 );
			memcpy( attr->getRawPointer(newIndex), attr->getRawPointer(index), attr->numComponents()*attr->elementComponentSize() );
		}
		return newIndex;
	}

	void Geometry::transform( const math::M44f& tm )
	{
		Attribute::Ptr pAttr = getAttr("P");
		int numElements = pAttr->numElements();
		for( int i=0; i<numElements; ++i )
		{
			math::Vec3f v = math::transform( pAttr->get<math::Vec3f>(i), tm);
			pAttr->set<math::Vec3f>( (unsigned int)i, v );
		}
	}


	//
	// computes vertex normals
	// Assumes geometry to be triangles!
	//
	void Geometry::addNormals()
	{
		// only works with triangles and quads
		if( !((primitiveType() == Geometry::TRIANGLE)||(primitiveType() == Geometry::QUAD)) )
		{
			std::cerr << "apply_normals: can compute normals only on non triangle or quad geometry\n";
			return;
		}

		Attribute::Ptr normalAttr = getAttr("N");

		if( !normalAttr )
			normalAttr = Attribute::createV3f();
		else
			normalAttr->clear();

		Attribute::Ptr positions = getAttr("P");
		int numPoints = positions->numElements();
		for( int i=0; i < numPoints; ++i )
			normalAttr->appendElement( math::V3f(0.0f, 0.0f, 0.0f) );

		int numPrims = numPrimitives();
		int numPrimVerts = numPrimitiveVertices();
		for( int i=0; i < numPrims; ++i )
		{
			int idx[3];
			idx[0] = m_indexBuffer[i*numPrimVerts];
			idx[1] = m_indexBuffer[i*numPrimVerts+1];
			idx[2] = m_indexBuffer[i*numPrimVerts+2];

			math::V3f v1 = positions->get<math::V3f>( idx[1] )-positions->get<math::V3f>( idx[0] );
			math::V3f v2 = positions->get<math::V3f>( idx[2] )-positions->get<math::V3f>( idx[0] );
			math::V3f fn = math::normalize( math::cross( v1,v2 ) );

			for( int j=0; j<3; ++j )
				normalAttr->set<math::V3f>( idx[j], normalAttr->get<math::V3f>(idx[j])+fn );
		}

		for( int i=0; i < numPoints; ++i )
			normalAttr->set<math::V3f>( i, math::normalize(  normalAttr->get<math::V3f>(i) ) );

		setAttr( "N", normalAttr );
	}

	//
	// removes all attributes and primitives
	//
	void Geometry::clear()
	{
		for( std::map< std::string, Attribute::Ptr >::iterator it = m_attributes.begin(); it != m_attributes.end(); ++it )
			it->second->clear();

		m_indexBuffer.clear();
		m_numPrimitives = 0;
		m_indexBufferIsDirty = true;
	}

	Geometry::Ptr Geometry::createPointGeometry()
	{
		Geometry::Ptr result = Geometry::Ptr( new Geometry(POINT) );
		Attribute::Ptr positions = Attribute::Ptr( Attribute::createV3f() );
		result->setAttr( "P", positions);
		return result;
	}

	Geometry::Ptr Geometry::createLineGeometry()
	{
		Geometry::Ptr result = Geometry::Ptr( new Geometry(LINE) );
		Attribute::Ptr positions = Attribute::Ptr( Attribute::createV3f() );
		result->setAttr( "P", positions);
		return result;
	}


	Geometry::Ptr Geometry::createTriangleGeometry()
	{
		Geometry::Ptr result = Geometry::Ptr( new Geometry(TRIANGLE) );
		Attribute::Ptr positions = Attribute::Ptr( Attribute::createV3f() );
		result->setAttr( "P", positions);
		return result;
	}

	Geometry::Ptr Geometry::createQuadGeometry()
	{
		Geometry::Ptr result = Geometry::Ptr( new Geometry(QUAD) );
		Attribute::Ptr positions = Attribute::Ptr( Attribute::createV3f() );
		result->setAttr( "P", positions);
		return result;
	}

	Geometry::Ptr Geometry::createPolyGeometry()
	{
		Geometry::Ptr result = std::make_shared<Geometry>(POLYGON);
		Attribute::Ptr positions = Attribute::Ptr( Attribute::createV3f() );
		result->setAttr( "P", positions);
		return result;
	}
/*
	Geometry::Ptr Geometry::createReferenceMesh()
	{
		return importObj( path( "base" ) + "/data/meshref.obj" );
	}
*/
/*
	Geometry::Ptr geo_points( math::Vec3f p )
	{
		Geometry::Ptr result = Geometry::Ptr( new Geometry(Geometry::POINT) );
		Attribute::Ptr positions = Attribute::createVec3f();
		positions->appendElement( p );
		result->setAttr( "P", positions);
		result->addPoint( 0 );
		return result;
	}

*/

	Geometry::Ptr Geometry::createQuad(Geometry::PrimitiveType primType)
	{
		Geometry::Ptr result = std::make_shared<Geometry>(primType);

		Attribute::Ptr positions = Attribute::createV3f();
		Attribute::Ptr uvs = Attribute::createV2f();

		positions->appendElement( math::Vec3f(-1.0f,-1.0f,0.0f) );
		uvs->appendElement( .0f, .0f );
		positions->appendElement( math::Vec3f(-1.0f,1.0f,0.0f) );
		uvs->appendElement( .0f, 1.0f );
		positions->appendElement( math::Vec3f(1.0f,1.0f,0.0f) );
		uvs->appendElement( 1.0f, 1.0f );
		positions->appendElement( math::Vec3f(1.0f,-1.0f,0.0f) );
		uvs->appendElement( 1.0f, .0f );

		result->setAttr( "P", positions);
		result->setAttr( "UV", uvs );

		if( primType == Geometry::QUAD )
			result->addQuad( 3, 2, 1, 0 );
		else if( primType == Geometry::TRIANGLE )
		{
			result->addTriangle( 3, 2, 1 );
			result->addTriangle( 3, 1, 0 );
		}

		return result;
	}

	Geometry::Ptr Geometry::createGrid( int xres, int zres, Geometry::PrimitiveType primType )
	{
		Geometry::Ptr result = std::make_shared<Geometry>(primType);

		Attribute::Ptr positions = Attribute::createV3f();
		result->setAttr( "P", positions);

		Attribute::Ptr uvs = Attribute::createV2f();
		result->setAttr( "UV", uvs );

		for( int j=0; j<zres; ++j )
			for( int i=0; i<xres; ++i )
			{
				float u = i/(float)(xres-1);
				float v = j/(float)(zres-1);
				positions->appendElement( math::V3f(u-0.5f,0.0f,v-0.5f) );
				uvs->appendElement( u, 1.0f - v ); // 1.0 - v because opengl texture space
			}

		if( primType == Geometry::POINT )
		{
			int numPoints = xres*zres;
			for( int i=0;i<numPoints;++i )
				result->addPoint( i );
		}else
		if( primType == Geometry::LINE )
		{
			for( int j=0; j<zres; ++j )
			{
				for( int i=0; i<xres-1; ++i )
				{
					int vo = (j*xres);
					result->addLine( vo+i, vo+i+1 );
				}
			}
			for( int j=0; j<xres; ++j )
			{
				for( int i=0; i<zres-1; ++i )
				{
					int v0 = (i*xres) + j;
					int v1 = ((i+1)*xres) + j;
					result->addLine( v0, v1 );
				}
			}
		}else
		if( primType == Geometry::TRIANGLE )
			for( int j=0; j<zres-1; ++j )
				for( int i=0; i<xres-1; ++i )
				{
					int vo = (j*xres);
					result->addTriangle( vo+xres+i+1, vo+i+1, vo+i );
					result->addTriangle( vo+xres+i, vo+xres+i+1, vo+i );

				}


		return result;
	}

	Geometry::Ptr Geometry::createSphere( int uSubdivisions, int vSubdivisions, float radius, math::Vec3f center, Geometry::PrimitiveType primType )
	{
		Geometry::Ptr result = std::make_shared<Geometry>(primType);

		Attribute::Ptr positions = Attribute::createV3f();
		result->setAttr( "P", positions);

		Attribute::Ptr uvs = Attribute::createV2f();
		result->setAttr( "UV", uvs );

		float dPhi = MATH_2PIf/uSubdivisions;
		float dTheta = MATH_PIf/vSubdivisions;
		float theta, phi;

		// y
		for (theta=MATH_PIf/2.0f+dTheta;theta<=(3.0f*MATH_PIf)/2.0f-dTheta;theta+=dTheta)
		{
			math::V3f p;
			float y = sin(theta);
			// x-z
			phi = 0.0f;
			for( int j = 0; j<uSubdivisions; ++j  )
			{
				p.x = cos(theta) * cos(phi);
				p.y = y;
				p.z = cos(theta) * sin(phi);

				p = p*radius + center;

				positions->appendElement( p );
				phi+=dPhi;
			}
		}

		int pole1 = positions->appendElement( math::V3f(0.0f, 1.0f, 0.0f)*radius + center );
		int pole2 = positions->appendElement( math::V3f(0.0f, -1.0f, 0.0f)*radius + center );

		if( primType == Geometry::POINT )
		{
			int numVertices = positions->numElements();
			for( int i=0; i< numVertices; ++i )
				result->addPoint( i );
		}else
		if( primType == Geometry::LINE )
		{
			int numVertices = positions->numElements();
			for( int j=0; j<vSubdivisions-3;++j )
			{
				int offset = j*(uSubdivisions);
				int i = 0;
				for( i=0; i<uSubdivisions-1; ++i )
					result->addLine(offset+i, offset+i+1);
				result->addLine(offset+0, offset+i);
			}
		}else
		if( primType == Geometry::TRIANGLE )
		{
			// add faces
			for( int j=0; j<vSubdivisions-3;++j )
			{
				int offset = j*(uSubdivisions);
				int i = 0;
				for( i=0; i<uSubdivisions-1; ++i )
				{
					result->addTriangle(offset+i+1, offset+i + uSubdivisions, offset+i);
					result->addTriangle(offset+i+1, offset+i+uSubdivisions+1, offset+i + uSubdivisions);
				}
				result->addTriangle(offset+0,offset+i + uSubdivisions,offset+i);
				result->addTriangle(offset,offset + uSubdivisions,offset+i + uSubdivisions);
			}
			for( int i=0; i<uSubdivisions-1; ++i )
			{
				result->addTriangle(i+1, i,pole1);
				result->addTriangle(uSubdivisions*(vSubdivisions-3)+i, uSubdivisions*(vSubdivisions-3)+i+1, pole2);
			}
			result->addTriangle(0, uSubdivisions-1, pole1);
			result->addTriangle(uSubdivisions*(vSubdivisions-2)-1, uSubdivisions*(vSubdivisions-3), pole2);
		}
		return result;
	}
/*
	Geometry::Ptr geo_cube( const math::BoundingBox3f &bound, Geometry::PrimitiveType primType )
	{
		Geometry::Ptr result = Geometry::Ptr(new Geometry(primType));

		// unique points
		std::vector<math::Vec3f> pos;
		pos.push_back( math::Vec3f(bound.minPoint.x,bound.minPoint.y,bound.maxPoint.z) );
		pos.push_back( math::Vec3f(bound.minPoint.x,bound.maxPoint.y,bound.maxPoint.z) );
		pos.push_back( math::Vec3f(bound.maxPoint.x,bound.maxPoint.y,bound.maxPoint.z) );
		pos.push_back( math::Vec3f(bound.maxPoint.x,bound.minPoint.y,bound.maxPoint.z) );

		pos.push_back( math::Vec3f(bound.minPoint.x,bound.minPoint.y,bound.minPoint.z) );
		pos.push_back( math::Vec3f(bound.minPoint.x,bound.maxPoint.y,bound.minPoint.z) );
		pos.push_back( math::Vec3f(bound.maxPoint.x,bound.maxPoint.y,bound.minPoint.z) );
		pos.push_back( math::Vec3f(bound.maxPoint.x,bound.minPoint.y,bound.minPoint.z) );

		if( primType == Geometry::QUAD )
		{


			// quads
			std::vector< std::tuple<int, int, int, int> > quads;
			quads.push_back( std::make_tuple(3, 2, 1, 0) );
			quads.push_back( std::make_tuple(4, 5, 6, 7) );
			quads.push_back( std::make_tuple(7, 6, 2, 3) );
			quads.push_back( std::make_tuple(1, 5, 4, 0) );
			quads.push_back( std::make_tuple(6, 5, 1, 2) );
			quads.push_back( std::make_tuple(4, 7, 3, 0) );

			// split per face (because we have uv shells)
			Attribute::Ptr positions = Attribute::createVec3f();
			Attribute::Ptr uv = Attribute::createVec2f();


			for( std::vector< std::tuple<int, int, int, int> >::iterator it = quads.begin(); it != quads.end(); ++it )
			{
				std::tuple<int, int, int, int> &quad = *it;
				int i0, i1, i2, i3;

				i0 = positions->appendElement( pos[std::get<0>(quad)] );
				uv->appendElement( math::Vec2f(0.0f, 0.0f) );
				i1 = positions->appendElement( pos[std::get<1>(quad)] );
				uv->appendElement( math::Vec2f(1.0f, 0.0f) );
				i2 = positions->appendElement( pos[std::get<2>(quad)] );
				uv->appendElement( math::Vec2f(1.0f, 1.0f) );
				i3 = positions->appendElement( pos[std::get<3>(quad)] );
				uv->appendElement( math::Vec2f(0.0f, 1.0f) );

				result->addQuad(i0, i1, i2, i3);			
			}

			result->setAttr( "P", positions);
			result->setAttr( "UV", uv);
		}else
		if( primType == Geometry::LINE )
		{
			// points
			Attribute::Ptr positions = Attribute::createVec3f();
			for( std::vector<math::Vec3f>::iterator it = pos.begin(), end=pos.end(); it != end; ++it)
				positions->appendElement( *it );
			result->setAttr( "P", positions);
			//lines
			result->addLine( 0, 1 );
			result->addLine( 1, 2 );
			result->addLine( 2, 3 );
			result->addLine( 3, 0 );

			result->addLine( 4, 5 );
			result->addLine( 5, 6 );
			result->addLine( 6, 7 );
			result->addLine( 7, 4 );

			result->addLine( 0, 4 );
			result->addLine( 1, 5 );
			result->addLine( 2, 6 );
			result->addLine( 3, 7 );
		}

		return result;
	}

	//TODO: add uv mapping
	Geometry::Ptr geo_sphere( int uSubdivisions, int vSubdivisions, float radius, math::Vec3f center, Geometry::PrimitiveType primType )
	{
		Geometry::Ptr result = Geometry::Ptr(new Geometry(primType));

		Attribute::Ptr positions = Attribute::createVec3f();
		result->setAttr( "P", positions);

		Attribute::Ptr uvs = Attribute::createVec2f();
		result->setAttr( "UV", uvs );

		float dPhi = MATH_2PIf/uSubdivisions;
		float dTheta = MATH_PIf/vSubdivisions;
		float theta, phi;

		// y
		for (theta=MATH_PIf/2.0f+dTheta;theta<=(3.0f*MATH_PIf)/2.0f-dTheta;theta+=dTheta)
		{
			math::Vec3f p;
			float y = sin(theta);
			// x-z
			phi = 0.0f;
			for( int j = 0; j<uSubdivisions; ++j  )
			{
				p.x = cos(theta) * cos(phi);
				p.y = y;
				p.z = cos(theta) * sin(phi);

				p = p*radius + center;

				positions->appendElement( p );
				phi+=dPhi;
			}
		}

		int pole1 = positions->appendElement( math::Vec3f(0.0f, 1.0f, 0.0f)*radius + center );
		int pole2 = positions->appendElement( math::Vec3f(0.0f, -1.0f, 0.0f)*radius + center );

		if( primType == Geometry::POINT )
		{
			int numVertices = positions->numElements();
			for( int i=0; i< numVertices; ++i )
				result->addPoint( i );
		}else
		if( primType == Geometry::LINE )
		{
			int numVertices = positions->numElements();
			for( int j=0; j<vSubdivisions-3;++j )
			{
				int offset = j*(uSubdivisions);
				int i = 0;
				for( i=0; i<uSubdivisions-1; ++i )
					result->addLine(offset+i, offset+i+1);
				result->addLine(offset+0, offset+i);
			}
		}else
		if( primType == Geometry::TRIANGLE )
		{
			// add faces
			for( int j=0; j<vSubdivisions-3;++j )
			{
				int offset = j*(uSubdivisions);
				int i = 0;
				for( i=0; i<uSubdivisions-1; ++i )
				{
					result->addTriangle(offset+i+1, offset+i + uSubdivisions, offset+i);
					result->addTriangle(offset+i+1, offset+i+uSubdivisions+1, offset+i + uSubdivisions);
				}
				result->addTriangle(offset+0,offset+i + uSubdivisions,offset+i);
				result->addTriangle(offset,offset + uSubdivisions,offset+i + uSubdivisions);
			}
			for( int i=0; i<uSubdivisions-1; ++i )
			{
				result->addTriangle(i+1, i,pole1);
				result->addTriangle(uSubdivisions*(vSubdivisions-3)+i, uSubdivisions*(vSubdivisions-3)+i+1, pole2);
			}
			result->addTriangle(0, uSubdivisions-1, pole1);
			result->addTriangle(uSubdivisions*(vSubdivisions-2)-1, uSubdivisions*(vSubdivisions-3), pole2);
		}
		return result;
	}

	Geometry::Ptr geo_circle( int uSubdivisions, float radius, math::Vec3f center, Geometry::PrimitiveType primType )
	{
		Geometry::Ptr result = Geometry::Ptr(new Geometry(Geometry::LINE));

		Attribute::Ptr positions = Attribute::createVec3f();
		result->setAttr( "P", positions);

		Attribute::Ptr uvs = Attribute::createVec2f();
		result->setAttr( "UV", uvs );

		float dPhi = MATH_2PIf/uSubdivisions;
		float phi;

		{
			math::Vec3f p;
			// x-z
			phi = 0.0f;
			for( int j = 0; j<uSubdivisions; ++j  )
			{
				p.x = cos(phi);
				p.y = 0.0f;
				p.z = sin(phi);

				p = p*radius + center;

				positions->appendElement( p );
				phi+=dPhi;
			}
		}

		int pole1 = positions->appendElement( math::Vec3f(0.0f, 1.0f, 0.0f)*radius + center );
		int pole2 = positions->appendElement( math::Vec3f(0.0f, -1.0f, 0.0f)*radius + center );

		if( primType == Geometry::POINT )
		{
			int numVertices = positions->numElements();
			for( int i=0; i< numVertices; ++i )
				result->addPoint( i );
		}else
		if( primType == Geometry::LINE )
		{
			for( int i=0; i<uSubdivisions-1; ++i )
			{
				result->addLine(i, i+1);
			}
			result->addLine(uSubdivisions-1, 0);

//			// add faces
//			for( int j=0; j<vSubdivisions-3;++j )
//			{
//				int offset = j*(uSubdivisions);
//				int i = 0;
//				for( i=0; i<uSubdivisions-1; ++i )
//				{
//					result->addTriangle(offset+i+1, offset+i + uSubdivisions, offset+i);
//					result->addTriangle(offset+i+1, offset+i+uSubdivisions+1, offset+i + uSubdivisions);
//				}
//				result->addTriangle(offset+0,offset+i + uSubdivisions,offset+i);
//				result->addTriangle(offset,offset + uSubdivisions,offset+i + uSubdivisions);
//			}
//			for( int i=0; i<uSubdivisions-1; ++i )
//			{
//				result->addTriangle(i+1, i,pole1);
//				result->addTriangle(uSubdivisions*(vSubdivisions-3)+i, uSubdivisions*(vSubdivisions-3)+i+1, pole2);
//			}
//			result->addTriangle(0, uSubdivisions-1, pole1);
//			result->addTriangle(uSubdivisions*(vSubdivisions-2)-1, uSubdivisions*(vSubdivisions-3), pole2);

		}
		return result;
	}

	Geometry::Ptr geo_cone( math::Vec3f axis, float halfAngle,  float height, int uSubdivisions )
	{
		Geometry::Ptr result = Geometry::Ptr(new Geometry(Geometry::TRIANGLE));

		Attribute::Ptr positions = Attribute::createVec3f();
		result->setAttr( "P", positions);


		float r = height*tan(halfAngle);

		math::Matrix33f rm = math::Matrix33f::RotationMatrix( axis, (2.0f*MATH_PIf)/uSubdivisions );

		math::Vec3f p = height*axis+math::cross( math::baseVec3<float>( math::nondominantAxis(axis) ), axis )*r;

		// add apex
		positions->appendElement<math::Vec3f>(math::Vec3f(0.0f, 0.0f, 0.0f));

		// add base and triangles
		positions->appendElement<math::Vec3f>(p);
		float d = (p-height*axis).getLength();
		for( int i=1;i<uSubdivisions;++i )
		{
			p = math::transform( p, rm );

			d = (p-height*axis).getLength();

			positions->appendElement<math::Vec3f>(p);
			result->addTriangle( 0, i, i+1 );
		}


		return result;
	}










	void apply_transform( Geometry::Ptr geo, math::Matrix44f tm )
	{
		Attribute::Ptr pAttr = geo->getAttr("P");
		int numElements = pAttr->numElements();
		for( int i=0; i<numElements; ++i )
		{
			math::Vec3f v = math::transform( pAttr->get<math::Vec3f>(i), tm);
			pAttr->set<math::Vec3f>( (unsigned int)i, v );
		}
	}









	math::BoundingBox3f compute_bound( Geometry::Ptr geo )
	{
		math::BoundingBox3f bbox;
		Attribute::Ptr p = geo->getAttr( "P" );
		int numElements = p->numElements();
		for( int i=0;i<numElements;++i )
			bbox.extend(p->get<math::Vec3f>(i));
		return bbox;
	}
	*/
}





