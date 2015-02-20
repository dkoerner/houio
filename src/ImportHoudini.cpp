/*
#include "ImportHoudini.h"

#include <QFileInfo>

#include <plugins/primitives/Camera.h>
#include <plugins/houdini/HouGeoIO.h>
#include <core/Core.h>

#include "HouScene.h"







ImportHoudini::ImportHoudini() : core::GraphNode()
{
	addInputSocket( "file" );
	addOutputSocket( "output" );
}


ImportHoudini::~ImportHoudini()
{
}

void ImportHoudini::update( core::GraphNodeSocket *output )
{
	QString filename = core::expand(getSocket( "file" )->asString());
	qDebug() << "ImportHoudini::update from " << filename;

	if( QFileInfo( filename ).suffix() == "json" )
	{
		// houdini scene file
		HouScene::Ptr scn = std::make_shared<HouScene>();
		scn->load( filename );


		Camera::Ptr camera = scn->getCamera("cam1");
		if(camera)
			getSocket( "output" )->setData(camera);
	}else
	if( QFileInfo( filename ).suffix() == "bgeo" )
	{
		// load houdini file ================
		std::ifstream in( filename.toUtf8(), std::ios_base::in | std::ios_base::binary );
		houdini::HouGeo::Ptr hgeo = houdini::HouGeoIO::import( &in );
		if( hgeo )
		{
			int primIndex = 0;
			houdini::HouGeo::Primitive::Ptr prim = hgeo->getPrimitive(primIndex);
			if(std::dynamic_pointer_cast<houdini::HouGeo::HouVolume>(prim) )
			{
				houdini::HouGeo::HouVolume::Ptr volprim = std::dynamic_pointer_cast<houdini::HouGeo::HouVolume>(prim);
				getSocket( "output" )->setData(volprim->field);
			}
		}
	}
}

*/




