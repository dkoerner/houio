#pragma once
#include <memory>
#include <string>
#include <map>

#include <houio/Attribute.h>
#include <houio/math/Math.h>



namespace houio
{

	class Geometry
	{
	public:
		typedef std::shared_ptr<Geometry> Ptr;

		enum PrimitiveType
		{
			POINT = 0, // GL_POINTS
			LINE = 1,
			TRIANGLE = 4,
			QUAD = 7,
			POLYGON = 9
		};

		Geometry( PrimitiveType pt = TRIANGLE );


		void                                                        clear(); // removes all attributes and primitives
		//Geometry                                                  *copy();




		//
		// manipulation
		//
		void                                                        reverse(); // reverses the order of vertices for each primitive (CW polys become CCW)
		unsigned int                                                duplicatePoint( unsigned int index ); // duplicates point and returns index of duplicate (all attributes are copied etc.)
		void                                                        transform( const math::M44f& tm );
		void                                                        addNormals();



		//
		// Attribute management
		//
		Attribute::Ptr                                              getAttr( const std::string &name );
		void                                                        setAttr( const std::string &name, Attribute::Ptr attr );
		bool                                                        hasAttr( const std::string &name );
		void                                                        getAttrNames( std::vector<std::string>& attrNames )const;
		void                                                        removeAttr( const std::string &name );
		void                                                        getAttrNames(std::vector<std::string>& names);

		std::map< std::string, Attribute::Ptr >                     m_attributes;


		//
		// primitive management
		//
		PrimitiveType                                               primitiveType();
		unsigned int                                                numPrimitives();
		unsigned int                                                numPrimitiveVertices(); // Point=1; Line=2; Triangle=3; Quad=4
		unsigned int                                                addPoint( unsigned int vId );
		unsigned int                                                addLine( unsigned int vId0, unsigned int vId1 );
		unsigned int                                                addTriangle( unsigned int vId0, unsigned int vId1, unsigned int vId2 );
		unsigned int                                                addQuad( unsigned int vId0, unsigned int vId1, unsigned int vId2, unsigned int vId3 );
		unsigned int                                                addPolygonVertex( unsigned int v );


		PrimitiveType                                               m_primitiveType; // determines the primitive type indexBuffer is pointing to...
		std::vector<unsigned int>                                   m_indexBuffer;
		bool                                                        m_indexBufferIsDirty;
		unsigned int                                                m_numPrimitives;
		unsigned int                                                m_numPrimitiveVertices; // Point=1; Line=2; Triangle=3; Quad=4; polygon:*

		//
		// OpenGL specific
		//
		unsigned int                                                m_indexBufferId;


		//
		// static creators
		//
		static Ptr                                                  createPointGeometry();
		static Ptr                                                  createLineGeometry();
		static Ptr                                                  createTriangleGeometry();
		static Ptr                                                  createQuadGeometry();
		static Ptr                                                  createPolyGeometry();
		//static GeometryPtr                                                           createReferenceMesh();

		static Ptr                                                  createQuad(Geometry::PrimitiveType primType = Geometry::TRIANGLE);
		static Ptr                                                  createGrid( int xres, int zres, Geometry::PrimitiveType primType = Geometry::TRIANGLE );
		static Ptr                                                  createSphere( int uSubdivisions, int vSubdivisions, float radius, math::Vec3f center = math::V3f(0.0f), Geometry::PrimitiveType primType = Geometry::TRIANGLE );
	};





	//
	// geometry generation functions
	//
	/*
	GeometryPtr                                     geo_points( math::Vec3f p );
	GeometryPtr                                     geo_quad(Geometry::PrimitiveType primType = Geometry::QUAD);

	GeometryPtr                                     geo_cube( const math::BoundingBox3f &bound = math::BoundingBox3f(math::Vec3f(-0.5f), math::Vec3f(0.5f)), Geometry::PrimitiveType primType = Geometry::QUAD );
	GeometryPtr                                     geo_sphere( int uSubdivisions, int vSubdivisions, float radius, math::Vec3f center = math::Vec3f(0.0f,0.0f,0.0f), Geometry::PrimitiveType primType = Geometry::TRIANGLE );
	GeometryPtr                                     geo_circle( int uSubdivisions, float radius, math::Vec3f center = math::Vec3f(0.0f,0.0f,0.0f), Geometry::PrimitiveType primType = Geometry::LINE );
	GeometryPtr                                     geo_cone( math::Vec3f axis, float halfAngle,  float height, int uSubdivisions = 30 );


	//
	// geometry manipulation functions
	//
	void                                            apply_transform( GeometryPtr geo, math::Matrix44f tm );
	void                                            apply_normals( GeometryPtr geo );

	// geometry utility functions
	math::BoundingBox3f                             compute_bound( GeometryPtr geo );
	*/
}
