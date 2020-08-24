// TODO:
// packing
// attribute varname mapping
// ascii support

#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <houio/Attribute.h>
#include <houio/math/Math.h>
#include <houio/types.h>



namespace houio
{
	struct HouGeoAdapter
	{
		typedef std::shared_ptr<HouGeoAdapter> Ptr;

		struct RawPointer
		{
			typedef std::shared_ptr<RawPointer> Ptr;

			~RawPointer();
			static Ptr           create( void *ptr, bool freeOnDestruction = false );
			void*                ptr;
			bool                 freeOnDestruction;
		};

		struct AttributeAdapter
		{
			typedef std::shared_ptr<AttributeAdapter> Ptr;

			virtual ~AttributeAdapter();
			enum Type
			{
				ATTR_TYPE_INVALID  = 0,
				ATTR_TYPE_NUMERIC  = 1,
				ATTR_TYPE_STRING   = 2
			};
			enum Storage
			{
				ATTR_STORAGE_INVALID  = 0,
				ATTR_STORAGE_FPREAL32 = 1,
				ATTR_STORAGE_FPREAL64 = 2,
				ATTR_STORAGE_INT32 = 3
			};
			virtual std::string              getName()const;
			virtual Type                     getType()const;
			virtual int                      getTupleSize()const;
			virtual Storage                  getStorage()const;
			virtual void                     getPacking( std::vector<int> &packing )const;
			virtual int                      getNumElements()const;
			virtual RawPointer::Ptr          getRawPointer();
			virtual std::string              getString( int index )const=0;
			virtual std::string              getUniqueString( int index )const=0;
			virtual int                      getStringIndex( int index )const=0;
			virtual std::vector<std::string> getUniqueStrings()const=0;
			virtual std::vector<int>         getStringIndices()const=0;
			static Type                      type( const std::string &typeName );
			static Storage                   storage( const std::string &storageName );
			static int                       storageSize( Storage storage );
		};

		struct Topology
		{
			typedef std::shared_ptr<Topology> Ptr;

			virtual ~Topology();

			virtual void                          getIndices( std::vector<int> &indices )const=0;
			virtual void                          addIndices( std::vector<int> &indices )=0;
			virtual sint64                        getNumIndices()const=0;
		};

		struct Primitive
		{
			typedef std::shared_ptr<Primitive> Ptr;
			enum Type
			{
				PRIM_VOLUME,
				PRIM_POLY
			};
			virtual ~Primitive(){}

			// will be overloaded by primitive runs which represent multiple prims
			virtual int numPrimitives()const{return 1;}
		};

		struct VolumePrimitive : public Primitive
		{
			typedef std::shared_ptr<VolumePrimitive> Ptr;

			virtual math::M44f                 getTransform()const=0;
			virtual int                        getVertex()const=0;
			virtual math::Vec3i                getResolution()const;
			virtual real32                     getVoxel( int i, int j, int k )const=0;
			virtual RawPointer::Ptr            getRawPointer(); // returns raw pointer to the data
		};

		struct PolyPrimitive : public Primitive
		{
			typedef std::shared_ptr<PolyPrimitive> Ptr;

			virtual int                        numPolys()const;
			virtual int                        numVertices( int poly )const;
			virtual int const*                 vertices(int poly=0)const;
			virtual int numPrimitives()const override{return numPolys();}
			virtual bool closed()const;
		};


		virtual sint64                pointcount()const;
		virtual sint64                vertexcount()const;
		virtual sint64                primitivecount()const;
		virtual void                  getPointAttributeNames( std::vector<std::string> &names )const;
		virtual AttributeAdapter::Ptr getPointAttribute( const std::string &name );
		virtual void                  getVertexAttributeNames( std::vector<std::string> &names )const;
		virtual AttributeAdapter::Ptr getVertexAttribute( const std::string &name );
		virtual void                  getGlobalAttributeNames( std::vector<std::string> &names )const;
		virtual AttributeAdapter::Ptr getGlobalAttribute( const std::string &name );
		virtual bool                  hasPrimitiveAttribute( const std::string &name )const;
		virtual void                  getPrimitiveAttributeNames( std::vector<std::string> &names )const=0;
		virtual AttributeAdapter::Ptr getPrimitiveAttribute( const std::string &name )=0;
		virtual void                  getPrimitives( std::vector<HouGeoAdapter::Primitive::Ptr>& primitives );
		virtual Topology::Ptr         getTopology();
	};



	//Geometry::Ptr convertHouGeoPrimitive( HouGeoPtr houGeo, int prim = -1 );
}


