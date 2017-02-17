#pragma once


#include <map>
#include <string>

#include <houio/Field.h>
#include <houio/Attribute.h>

#include <houio/json.h>
#include <houio/HouGeoAdapter.h>


namespace houio
{


	// HouGeo ============================================================
	struct HouGeo : public HouGeoAdapter
	{
		typedef std::shared_ptr<HouGeo> Ptr;

		struct HouAttribute : public AttributeAdapter
		{
			typedef std::shared_ptr<HouAttribute> Ptr;

			HouAttribute();
			HouAttribute( const std::string &name, Attribute::Ptr attr );

			virtual std::string                   getName()const;
			virtual Type                          getType()const;
			virtual int                           getTupleSize()const;
			virtual Storage                       getStorage()const;
			virtual void                          getPacking( std::vector<int> &packing )const;
			virtual int                           getNumElements()const;
			virtual std::string                   getString( int index )const;
			virtual RawPointer::Ptr               getRawPointer();

			//int                                   addV4f(math::V4f value);
			int                                   addString(const std::string &value);

			std::string                           m_name;
			int                                   tupleSize;
			Storage                               m_storage;
			Type                                  m_type;
			//std::vector<char>                     data;
			std::vector<std::string>              strings; // used in case of type==string
			int                                   numElements;

			Attribute::Ptr                        m_attr; // primitives::Attribute
		};

		struct HouTopology : public Topology
		{
			HouTopology();
			typedef std::shared_ptr<HouTopology> Ptr;
			virtual void                          getIndices( std::vector<int> &indices )const override;
			virtual void                          addIndices( std::vector<int> &indices );
			virtual sint64                        getNumIndices()const;

			std::vector<int>                      indexBuffer;
		};


		struct HouVolume : public VolumePrimitive
		{
			typedef std::shared_ptr<HouVolume> Ptr;
			virtual math::M44f                                getTransform()const;
			virtual int                                       getVertex()const;
			virtual math::Vec3i                               getResolution()const;
			virtual RawPointer::Ptr                           getRawPointer(); // returns raw pointer to the data
			virtual real32                                    getVoxel( int i, int j, int k )const;

			ScalarField::Ptr                                  field;
			int                                               vertex; // hougeo uses point indices to encode translation
		};

		struct HouPoly : public PolyPrimitive
		{
			typedef std::shared_ptr<HouPoly> Ptr;
			virtual int                                       numPolys()const override;
			virtual int                                       numVertices( int poly )const override;
			virtual int const*                                vertices(int poly=0)const override;
			virtual bool                                      closed()const override;
			int                                               m_numPolys;
			std::vector<int>                                  m_perPolyVertexCount; // holds number of vertices for each polygon
			std::vector<int>                                  m_perPolyVertexListOffset; // holds offset into m_vertices per poly
			std::vector<int>                                  m_vertices; // vertex indicess for each vertex
			bool                                              m_closed;
		};




		HouGeo();

		static HouGeo::Ptr                                   create();

		void                                                 setPointAttribute( HouAttribute::Ptr attr );
		void                                                 setPrimitiveAttribute( const std::string &name, HouAttribute::Ptr attr );
		void                                                 addPrimitive( ScalarField::Ptr field );
		void                                                 addPrimitive( PolyPrimitive::Ptr poly );
		void                                                 setTopology( HouTopology::Ptr topo );


		// inherited from HouGeoAdapter
		virtual sint64                                       pointcount()const override;
		virtual sint64                                       vertexcount()const override;
		virtual sint64                                       primitivecount()const override;
		virtual void                                         getPointAttributeNames( std::vector<std::string> &names )const override;
		virtual AttributeAdapter::Ptr                        getPointAttribute( const std::string &name ) override;
		virtual void                                         getVertexAttributeNames( std::vector<std::string> &names )const override;
		virtual AttributeAdapter::Ptr                        getVertexAttribute( const std::string &name ) override;
		virtual bool                                         hasPrimitiveAttribute( const std::string &name )const override;
		virtual void                                         getPrimitiveAttributeNames( std::vector<std::string> &names )const override;
		virtual AttributeAdapter::Ptr                        getPrimitiveAttribute( const std::string &name ) override;
		virtual void                                         getPrimitives( std::vector<HouGeoAdapter::Primitive::Ptr>& primitives )override;
		virtual void                                         getGlobalAttributeNames( std::vector<std::string> &names )const override;
		virtual AttributeAdapter::Ptr                        getGlobalAttribute( const std::string &name ) override;
		virtual Topology::Ptr                                getTopology() override;




		// this structure carries some global json data which I dont want to have as members of hougeo
		struct SharedPrimitiveData
		{
			std::map<std::string, json::ObjectPtr> sharedVoxelData;
		};

		void                                                 load( json::ObjectPtr o ); // a has to be the root of the array from hou geo
		HouAttribute::Ptr                                    loadAttribute( json::ArrayPtr attribute, sint64 elementCount );
		void                                                 loadTopology( json::ObjectPtr o );
		void                                                 loadPrimitive( json::ArrayPtr primitive, SharedPrimitiveData& sharedPrimitiveData );
		void                                                 loadVolumePrimitive( json::ObjectPtr volume, SharedPrimitiveData& sharedPrimitiveData );
		void                                                 loadPolyPrimitive( json::ObjectPtr poly );
		void                                                 loadPolyPrimitiveRun( json::ObjectPtr def, json::ArrayPtr run );

		void                                                 loadVoxelData( json::ObjectPtr voxels, const math::V3i& res, float* volData );


		static json::ObjectPtr                               toObject( json::ArrayPtr a ); // turns json array into jsonObject (every first entry is key, every second is value)

	private:
		std::vector<Primitive::Ptr>                                              m_primitives;
		std::map<std::string, HouAttribute::Ptr>                            m_pointAttributes;
		std::map<std::string, HouAttribute::Ptr>                           m_vertexAttributes;
		std::map<std::string, HouAttribute::Ptr>                        m_primitiveAttributes;
		std::map<std::string, HouAttribute::Ptr>                           m_globalAttributes;
		HouTopology::Ptr                                                           m_topology;
	};


}  // namespace houio
