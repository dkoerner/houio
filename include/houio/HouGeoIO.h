#pragma once
#include <houio/HouGeo.h>
#include <houio/Geometry.h>


namespace houio
{
	struct HouGeoIO
	{
		static HouGeo::Ptr                      import( std::istream *in );
		static Geometry::Ptr                    importGeometry( const std::string &path );
		static ScalarField::Ptr                 importVolume(const std::string &path);
		static void                             makeLog( const std::string &path, std::ostream *out );

		static Geometry::Ptr                    convertToGeometry(HouGeo::Ptr houGeo, HouGeoAdapter::Primitive::Ptr houPrim ); // converts primitive with the given index to geometry

		static bool                             xport( const std::string& filename, ScalarField::Ptr volume ); // convinience funcion for quickly saving volume to bgeo
		static bool                             xport( const std::string& filename, Geometry::Ptr geo ); // convinience funcion for quickly saving geometry to bgeo
		static bool                             xport( std::ostream *out, HouGeoAdapter::Ptr geo , bool binary = true);

	private:
		static bool                             exportAttribute( HouGeoAdapter::AttributeAdapter::Ptr attr );
		static bool                             exportTopology( HouGeoAdapter::Topology::Ptr topo );
		static bool                             exportPrimitive( HouGeoAdapter::VolumePrimitive::Ptr volume );
		static bool                             exportPrimitive( HouGeoAdapter::PolyPrimitive::Ptr poly );
		static HouGeoAdapter*                   g_geo;
		static json::BinaryWriter*              g_writer;
	};
}
