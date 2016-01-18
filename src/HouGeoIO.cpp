#include <houio/HouGeoIO.h>



namespace houio
{
	HouGeoAdapter*                  HouGeoIO::g_geo = 0;
	json::BinaryWriter*             HouGeoIO::g_writer = 0;

	HouGeo::Ptr HouGeoIO::import( std::istream *in )
	{
		json::JSONReader reader;
		json::Parser p;

		//core::Timer timer;
		//timer.start();
		if(!p.parse( in, &reader ))
			return HouGeo::Ptr();
		//timer.stop();
		//float time_parse = timer.elapsedSeconds();

		//timer.reset();
		// now reader contains the json data (structured after the scheme of the file)
		// we will create an empty HouGeo and have it load its data from the json data
		HouGeo::Ptr houGeo = HouGeo::create();
		//timer.start();
		houGeo->load( HouGeo::toObject(reader.getRoot().asArray()) );
		//timer.stop();
		//float time_load = timer.elapsedSeconds();
		//qDebug() << "loading time: " << time_load;
		//qDebug() << "total time: " << (time_load + time_parse);
		return houGeo;
	}

	Geometry::Ptr HouGeoIO::importGeometry( const std::string &path )
	{
		Geometry::Ptr result;
		std::ifstream in( path.c_str(), std::ios_base::in | std::ios_base::binary );
		HouGeo::Ptr hgeo = HouGeoIO::import( &in );
		if( hgeo )
		{
			int primIndex = 0;
			HouGeo::Primitive::Ptr prim = hgeo->getPrimitive(primIndex);

			//geo
			if(std::dynamic_pointer_cast<HouGeo::HouPoly>(prim) )
				result = convertToGeometry(hgeo, primIndex);
		}else
		{
			std::cout << "HouGeoIO::importGeometry: failed to import houGeo\n";
		}
		return result;
	}

	ScalarField::Ptr HouGeoIO::importVolume( const std::string &path )
	{
		ScalarField::Ptr result;
		std::ifstream in( path.c_str(), std::ios_base::in | std::ios_base::binary );
		HouGeo::Ptr hgeo = HouGeoIO::import( &in );
		if( hgeo )
		{
			int primIndex = 0;
			HouGeo::Primitive::Ptr prim = hgeo->getPrimitive(primIndex);

			//volume
			if(std::dynamic_pointer_cast<HouGeo::HouVolume>(prim) )
			{
				HouGeo::HouVolume::Ptr houVolume = std::dynamic_pointer_cast<HouGeo::HouVolume>(prim);
				result = houVolume->field;
			}
		}else
		{
			std::cout << "HouGeoIO::importVolume: failed to import houGeo\n";
		}
		return result;
	}

	// prim -1 means we will get a simple pointsgeometry
	Geometry::Ptr HouGeoIO::convertToGeometry(HouGeo::Ptr houGeo, int prim )
	{
		Geometry::Ptr result;

		HouGeo::Primitive::Ptr houPrim = houGeo->getPrimitive(0);

		// check if pointer is valid
		if( !houPrim )
		{
			std::cout << "error no primitive!\n";
			return Geometry::Ptr();
		}

		// cast and get some first numbers...
		HouGeo::HouPoly::Ptr poly = std::dynamic_pointer_cast<HouGeo::HouPoly>(houPrim);
		sint64 numPolys = poly->numPolys();
		sint64 numPoints = houGeo->pointcount();
		sint64 numVertices = houGeo->vertexcount();

		// we only support geometry with non mixed primitives (e.g. triangles only)
		// this code checks if that is the case...
		int numVerticesPerPoly = 0;
		if( prim > -1 )
		{
			bool hasConstantVertexCountPerPoly = true;
			{
				numVerticesPerPoly = numPolys > 0 ? poly->numVertices(0) : 0;
				for( int i=0;i<numPolys;++i )
					if(poly->numVertices(i) != numVerticesPerPoly)
					{
						hasConstantVertexCountPerPoly = false;
						break;
					}
			}
			if( !hasConstantVertexCountPerPoly )
				throw std::runtime_error( "convertHouGeoPrimitive: non constant polygon primitvies" );
		}

		// create the right kind of geometry depending on vertexcount per primitive (point, line or triangle geometry)
		if( prim == -1 )
			result = Geometry::createPointGeometry();
		else
		if( numVerticesPerPoly == 3 )
			result = Geometry::createTriangleGeometry();
		else
		if( numVerticesPerPoly == 4 )
			result = Geometry::createQuadGeometry();

		// attributes ---
		std::vector<Attribute::Ptr> geoAttrs;

		std::vector<std::string> pointAttributesNames;
		houGeo->getPointAttributeNames(pointAttributesNames);

		std::vector<std::string> vertexAttributesNames;
		houGeo->getVertexAttributeNames(vertexAttributesNames);

		// convert point attributes ---
		for( std::vector<std::string>::iterator it = pointAttributesNames.begin(), end = pointAttributesNames.end();  it != end; ++it )
		{
			std::string attrName = *it;
			HouGeoAdapter::AttributeAdapter::Ptr houAttr = houGeo->getPointAttribute(attrName);

			int numComponents = houAttr->getTupleSize();

			Attribute::Ptr attr;
			if( attrName == "P" )
			{
				attr = result->getAttr("P");
				float *ptr = (float*)houGeo->getPointAttribute( attrName )->getRawPointer()->ptr;
				for( int i=0;i<numPoints;++i )
				{
					math::Vec3f p;
					p.x = *ptr++; p.y = *ptr++; p.z = *ptr++;ptr++;
					attr->appendElement( p );
				}
			}else
			if( (attrName == "UV")||(attrName == "uv") )
			{
				float *ptr = (float*)houGeo->getPointAttribute( attrName )->getRawPointer()->ptr;
				attrName = "UV";
				attr = Attribute::createV2f();
				for( int i=0;i<numPoints;++i )
				{
					math::Vec2f uv;
					uv.x = *ptr++; uv.y = *ptr++;ptr++;
					attr->appendElement( uv );
				}
			}else
			if( houAttr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL32 )
				attr = Attribute::create( numComponents, Attribute::FLOAT, (unsigned char *)houAttr->getRawPointer()->ptr, houAttr->getNumElements() );

			result->setAttr( attrName, attr );
		}

		// convert vertex attributes ---
		std::vector<std::pair<Attribute::Ptr, Attribute::Ptr>> vertex2pointAttr;
		for( std::vector<std::string>::iterator it = vertexAttributesNames.begin(), end = vertexAttributesNames.end();  it != end; ++it )
		{
			std::string attrName = *it;
			HouGeoAdapter::AttributeAdapter::Ptr houAttr = houGeo->getVertexAttribute(attrName);

			int numComponents = houAttr->getTupleSize();

			Attribute::Ptr attr;
			if( (attrName == "UV")||(attrName == "uv") )
			{
				float *ptr = (float*)houGeo->getVertexAttribute( attrName )->getRawPointer()->ptr;
				attrName = "UV";
				attr = Attribute::createV2f();

				for( int i=0;i<numVertices;++i )
				{
					math::Vec2f uv;
					uv.x = *ptr++; uv.y = *ptr++;ptr++;
					attr->appendElement( uv );
				}
			}else
			if( houAttr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL32 )
				attr = Attribute::create( numComponents, Attribute::FLOAT, (unsigned char *)houAttr->getRawPointer()->ptr, houAttr->getNumElements() );


			// create point attribute which we will derive from this vertex attribute
			Attribute::Ptr pointAttr = attr->copy();
			pointAttr->resize( (int)numPoints );
			memset( pointAttr->getRawPointer(), 0, numPoints*pointAttr->elementComponentSize()*pointAttr->numComponents() );

			result->setAttr( attrName, pointAttr );
			vertex2pointAttr.push_back( std::make_pair( attr, pointAttr ) );
		}


		// only done when we have primitives...
		if( prim > -1 )
		{
			// since opengl doesnt support face varying data we also dont support it in our geometry
			// therefore we need convert our vertex attribute to point attributes and duplicate those points
			// which have different vertex data

			const int *vertex_ptr = poly->vertices();

			std::vector<bool> pointsToSplit(numPoints, false);
			std::vector<bool> pointsInitialized(numPoints, false);

			// mark all points which have to be duplicated
			std::vector<int> firstVertex(numPoints, -1);
			for( int i=0;i<numVertices;++i )
			{
				int point = vertex_ptr[i];
				if( !pointsToSplit[point] )
					if( firstVertex[point] >=0 )
					{
						// compare
						for( std::vector<std::pair<Attribute::Ptr, Attribute::Ptr>>::iterator it = vertex2pointAttr.begin(), end = vertex2pointAttr.end(); it != end; ++it )
							if( memcmp( it->first->getRawPointer(i), it->first->getRawPointer(firstVertex[point]), it->first->elementComponentSize()*it->first->numComponents() ) )
							{
								pointsToSplit[point] = true;
								//result->getAttr("Cd")->set( point, 1.0f, 0.0f, 0.0f );
								break;
							}
					}else
						firstVertex[point] = i;
			}


			int vertexIndex = 0;
			for( int i=0;i<numPolys;++i )
			{
				int numVerts = poly->numVertices(i);
				std::vector<int> vertices;
				vertices.reserve(4);

				for( int j=0;j<numVerts;++j, ++vertexIndex )
				{
					unsigned int finalPointIndex = vertex_ptr[j];

					if( pointsToSplit[finalPointIndex] && pointsInitialized[finalPointIndex] )
					{
						finalPointIndex = result->duplicatePoint(finalPointIndex);
						// copy vertex attributes to duplicated point location
						for( std::vector<std::pair<Attribute::Ptr, Attribute::Ptr>>::iterator it = vertex2pointAttr.begin(), end = vertex2pointAttr.end(); it != end; ++it )
							memcpy( it->second->getRawPointer(finalPointIndex), it->first->getRawPointer(vertexIndex), it->first->elementComponentSize()*it->first->numComponents());
					}
					else if( !pointsInitialized[finalPointIndex]  )
					{
						pointsInitialized[finalPointIndex] = true;
						// copy vertex attributes to duplicated point location
						for( std::vector<std::pair<Attribute::Ptr, Attribute::Ptr>>::iterator it = vertex2pointAttr.begin(), end = vertex2pointAttr.end(); it != end; ++it )
							memcpy( it->second->getRawPointer(finalPointIndex), it->first->getRawPointer(vertexIndex), it->first->elementComponentSize()*it->first->numComponents());
					}

					//std::cout << i << ":" << j << " " << finalPointIndex << "    " << vertex2pointAttr[0].first->get<math::Vec2f>(vertexIndex).x << "  " << vertex2pointAttr[0].first->get<math::Vec2f>(vertexIndex).y << std::endl;
					//std::cout << i << ":" << j << " " << finalPointIndex << "    " << vertex2pointAttr[0].second->get<math::Vec2f>(finalPointIndex).x << "  " << vertex2pointAttr[0].second->get<math::Vec2f>(finalPointIndex).y << std::endl;

					//if( vertex2pointAttr[0].first->get<math::Vec2f>(vertexIndex).x == result->getAttr("UV")->get<math::Vec2f>(finalPointIndex).x )
					//	result->getAttr("Cd")->set(finalPointIndex, 1.0f, 0.0f, 0.0f);

					vertices.push_back(finalPointIndex);
				}

				if( numVerts == 3 )
					result->addTriangle( vertices[0], vertices[1], vertices[2] );
				else
				if( numVerts == 4 )
					result->addQuad( vertices[0], vertices[1], vertices[2], vertices[3] );

				vertex_ptr+=numVerts;
			}

			// houdini polys are CW - opengl defaults to CCW
			result->reverse();
		}

		return result;
	}

	void HouGeoIO::makeLog( const std::string &path, std::ostream *out )
	{
		std::ifstream in( path.c_str(), std::ios_base::in | std::ios_base::binary );
		json::JSONLogger logger(*out);
		json::Parser p;
		p.parse( &in, &logger );
	}



	// convinience funcion for quickly saving volume to bgeo
	bool HouGeoIO::xport( const std::string& filename, ScalarField::Ptr volume )
	{
		std::ofstream out( filename.c_str(), std::ios_base::out | std::ios_base::binary );
		HouGeo::Ptr houGeo = std::make_shared<HouGeo>();
		houGeo->addPrimitive(volume);
		return HouGeoIO::xport( &out, houGeo );
	}

	bool HouGeoIO::xport(const std::string &filename, Geometry::Ptr geo)
	{
		std::ofstream out( filename.c_str(), std::ios_base::out | std::ios_base::binary );
		HouGeo::Ptr houGeo = std::make_shared<HouGeo>();


		// contruct point attributes  ---
		std::vector<std::string> pointAttributeNames;
		geo->getAttrNames(pointAttributeNames);
		for( auto name : pointAttributeNames )
		{
			Attribute::Ptr attr = geo->getAttr(name);

			// some attributes need special treatment
			if( name == "P" && attr->numComponents() == 3 )
			{
				// promote P attribute from v3f to v4f
				Attribute::Ptr attr_new = std::make_shared<Attribute>( 4, Attribute::FLOAT);
				int numElements = attr->numElements();
				for( int i=0;i<numElements;++i )
				{
					math::V3f p = attr->get<math::V3f>(i);
					attr_new->appendElement<math::V4f>( math::V4f(p.x, p.y, p.z, 1.0) );
				}
				attr = attr_new;
			}

			std::cout << "adding " << name << std::endl;
			HouGeo::HouAttribute::Ptr hattr = std::make_shared<HouGeo::HouAttribute>( name, attr );
			houGeo->setPointAttribute( hattr );
		}


		return HouGeoIO::xport( &out, houGeo );
	}




	bool HouGeoIO::xport( std::ostream *out, HouGeoAdapter::Ptr geo, bool binary )
	{
		if( binary )
			g_writer = new json::BinaryWriter( out );
		else
			// todo: ascii writer...
			g_writer = new json::BinaryWriter( out );

		g_writer->jsonBeginArray();

		g_writer->jsonString( "pointcount" );
		g_writer->jsonInt( geo->pointcount() );

		g_writer->jsonString( "vertexcount" );
		g_writer->jsonInt( geo->vertexcount() );

		g_writer->jsonString( "primitivecount" );
		g_writer->jsonInt( geo->primitivecount() );

		// -- topology (required)
		g_writer->jsonString( "topology" );
		g_writer->jsonBeginArray();
			if( geo->getTopology() )
				exportTopology( geo->getTopology() );
		g_writer->jsonEndArray();


		// -- attributes
		g_writer->jsonString( "attributes" );
		g_writer->jsonBeginArray();

			// -- point attributes
			g_writer->jsonString( "pointattributes" );
			g_writer->jsonBeginArray();
				std::vector<std::string> pointAttrNames;
				geo->getPointAttributeNames(pointAttrNames);
				for( std::vector<std::string>::iterator it = pointAttrNames.begin(); it != pointAttrNames.end(); ++it )
					exportAttribute( geo->getPointAttribute(*it) );
			g_writer->jsonEndArray(); // pointattributes


			// -- primitive attributes
			g_writer->jsonString( "primitiveattributes" );
			g_writer->jsonBeginArray();
				std::vector<std::string> primitiveAttrNames;
				geo->getPrimitiveAttributeNames(primitiveAttrNames);
				for( std::vector<std::string>::iterator it = primitiveAttrNames.begin(); it != primitiveAttrNames.end(); ++it )
					exportAttribute( geo->getPrimitiveAttribute(*it) );
			g_writer->jsonEndArray(); // primitiveattributes

/*
			// -- global attributes
			g_writer->jsonString( "globalattributes" );
			g_writer->jsonBeginArray();
				std::vector<std::string> globalAttrNames;
				geo->getGlobalAttributeNames(globalAttrNames);
				for( std::vector<std::string>::iterator it = globalAttrNames.begin(); it != globalAttrNames.end(); ++it )
					exportAttribute( geo->getGlobalAttribute(*it) );
			g_writer->jsonEndArray(); // globalattributes
*/
		g_writer->jsonEndArray(); // attributes


		// -- primitives
		if( geo->primitivecount() > 0 )
		{
			g_writer->jsonString( "primitives" );
			g_writer->jsonBeginArray();
			for( int i=0;i<geo->primitivecount();++i )
			{
				HouGeoAdapter::Primitive::Ptr prim = geo->getPrimitive(i);

				if( std::dynamic_pointer_cast<HouGeoAdapter::VolumePrimitive>(prim) )
					exportPrimitive(std::dynamic_pointer_cast<HouGeoAdapter::VolumePrimitive>(prim));

			}
			g_writer->jsonEndArray(); // primitives
		}



		g_writer->jsonEndArray(); // /root

		// done
		delete g_writer;g_writer=0;

		return true;
	}






	bool HouGeoIO::exportAttribute( HouGeoAdapter::AttributeAdapter::Ptr attr )
	{
		if( !attr )
			return false;

		std::string type;
		std::string storage;
		int size = attr->getTupleSize();
		std::string name = attr->getName();

		if( attr->getType() == HouGeoAdapter::AttributeAdapter::ATTR_TYPE_NUMERIC )
			type = "numeric";
		else if( attr->getType() == HouGeoAdapter::AttributeAdapter::ATTR_TYPE_STRING )
			type = "string";

		if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL32 )
				storage = "fpreal32";
		else if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL64 )
				storage = "fpreal64";
		else if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_INT32 )
				storage = "int32";


		// P attribute has to have 4 components, otherwise houdini will become unstable and eventually crash
		if( (name == "P")&&(size!=4)  )
			throw std::runtime_error( "HouGeoIO::exportAttribute: P attribute has to have 4 components, otherwise houdini will become unstable and eventually crash" );

		g_writer->jsonBeginArray();


		// attribute definition ------------
		g_writer->jsonBeginArray();

		g_writer->jsonString( "name" );
		g_writer->jsonString( name );

		g_writer->jsonString( "type" );
		g_writer->jsonString(type);

		g_writer->jsonEndArray(); // definition

		// attribute content ------------
		g_writer->jsonBeginArray();

		if( attr->getType() == HouGeoAdapter::AttributeAdapter::ATTR_TYPE_NUMERIC )
		{
			g_writer->jsonString( "size" );
			g_writer->jsonInt( size );

			g_writer->jsonString( "storage" );
			g_writer->jsonString( storage );


			g_writer->jsonString( "values" );
			g_writer->jsonBeginArray();

				g_writer->jsonString( "size" );
				g_writer->jsonInt( attr->getTupleSize() );

				g_writer->jsonString( "storage" );
				g_writer->jsonString( storage );

				g_writer->jsonString( "pagesize" );
				g_writer->jsonInt( 1024 );


				//g_writer->jsonString( "packing" );
				//std::vector<int> packing;
				//attr->getPacking( packing );
				//g_writer->jsonUniformArray( packing );

				//std::cout << "CHECK " << name << std::endl;
				//std::cout << "CHECK " << attr->getNumElements() << std::endl;
				//std::cout << "CHECK " << attr->getTupleSize() << std::endl;
				//attr->getRawPointer();

				g_writer->jsonString( "rawpagedata" );
				if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL32 )
					g_writer->jsonUniformArray<real32>(  (const real32*) attr->getRawPointer()->ptr, attr->getNumElements()*attr->getTupleSize() );
				else if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_FPREAL64 )
					g_writer->jsonUniformArray<real64>( (const real64*) attr->getRawPointer()->ptr, attr->getNumElements()*attr->getTupleSize() );
				else if( attr->getStorage() == HouGeoAdapter::AttributeAdapter::ATTR_STORAGE_INT32 )
					g_writer->jsonUniformArray<sint32>( (const sint32*) attr->getRawPointer()->ptr, attr->getNumElements()*attr->getTupleSize() );

			g_writer->jsonEndArray(); // values
		}else
		if(attr->getType() == HouGeoAdapter::AttributeAdapter::ATTR_TYPE_STRING )
		{
			g_writer->jsonString( "size" );
			g_writer->jsonInt( size );

			g_writer->jsonString( "storage" );
			g_writer->jsonString( "int32" );

			g_writer->jsonString( "strings" );
			g_writer->jsonBeginArray();
				for( int i=0;i<attr->getNumElements();++i )
					g_writer->jsonString( attr->getString(i) );
			g_writer->jsonEndArray();

			g_writer->jsonString( "indices" );
			g_writer->jsonBeginArray();
				g_writer->jsonString( "size" );
				g_writer->jsonInt32( 1 );

				g_writer->jsonString( "storage" );
				g_writer->jsonString( "int32" );

				g_writer->jsonString( "pagesize" );
				g_writer->jsonInt32( 1024 );

				g_writer->jsonString( "rawpagedata" );
				std::vector<int> indices;
				for( int i=0;i<attr->getNumElements();++i )
					indices.push_back(i);
				g_writer->jsonUniformArray(indices);

			g_writer->jsonEndArray();


		}



		g_writer->jsonEndArray(); // attribute content



		g_writer->jsonEndArray(); // attribute

		return true;
	}

	// export topo
	// TODO: datatype is int ~~~~
	bool HouGeoIO::exportTopology( HouGeoAdapter::Topology::Ptr topo )
	{
		std::vector<int> indices;
		topo->getIndices(indices);
		std::vector<sint16> indicesInt16;
		for( int i=0;i<indices.size();++i )
			indicesInt16.push_back(sint16(indices[i]));

		g_writer->jsonString( "pointref" );
		g_writer->jsonBeginArray();
			g_writer->jsonString( "indices" );
			g_writer->jsonUniformArray<sint16>(indicesInt16);
		g_writer->jsonEndArray();

		return true;
	}


	bool HouGeoIO::exportPrimitive( HouGeoAdapter::VolumePrimitive::Ptr volume )
	{
		math::V3i res = volume->getResolution();

		g_writer->jsonBeginArray();

		// primitive type
		g_writer->jsonBeginArray();
			g_writer->jsonString("type");
			g_writer->jsonString("Volume");
		g_writer->jsonEndArray();

		g_writer->jsonBeginArray();
			g_writer->jsonString("vertex");
			g_writer->jsonInt32(volume->getVertex());

			g_writer->jsonString("transform");
			math::M44f houLocalToWorldTranslation = math::M44f::TranslationMatrix(volume->getTransform().getTranslation());
			math::M44f houLocalToWorldRotationScale = math::M44f::ScaleMatrix(0.5f)*math::M44f::TranslationMatrix(1.0,1.0,1.0)*volume->getTransform()*houLocalToWorldTranslation.inverted();
			math::M33f transform( houLocalToWorldRotationScale.ma[0], houLocalToWorldRotationScale.ma[1], houLocalToWorldRotationScale.ma[2],
						houLocalToWorldRotationScale.ma[4], houLocalToWorldRotationScale.ma[5], houLocalToWorldRotationScale.ma[6],
						houLocalToWorldRotationScale.ma[8], houLocalToWorldRotationScale.ma[9], houLocalToWorldRotationScale.ma[10]);
			g_writer->jsonUniformArray<real32>( transform.ma, 9 );

			g_writer->jsonString("res");
			g_writer->jsonUniformArray<sint32>( &res.x, 3 );

			g_writer->jsonString("border");
			g_writer->jsonBeginMap();
				g_writer->jsonKey("type");
				g_writer->jsonString("constant");
				g_writer->jsonKey("value");
				g_writer->jsonReal32(0.0f);
			g_writer->jsonEndMap();

			g_writer->jsonString("compression");
			g_writer->jsonBeginMap();
				g_writer->jsonKey("tolerance");
				g_writer->jsonReal32(0.0f);
			g_writer->jsonEndMap();

			g_writer->jsonString("voxels");
			g_writer->jsonBeginArray();
				g_writer->jsonString("tiledarray");
				g_writer->jsonBeginArray();
					g_writer->jsonString("version");
					g_writer->jsonInt32( 1 );

					g_writer->jsonString("compressiontype");
					// !!! uniform string array?
					g_writer->jsonBeginArray();
						g_writer->jsonString("raw");
						g_writer->jsonString("rawfull");
						g_writer->jsonString("constant");
						g_writer->jsonString("fpreal16");
						g_writer->jsonString("FP32Range");
					g_writer->jsonEndArray();

					g_writer->jsonString("tiles");
					g_writer->jsonBeginArray();

						// get first invalid tileindex in each dimension
						math::Vec3i tileEnd;
						tileEnd.x = res.x / 16;
						tileEnd.y = res.y / 16;
						tileEnd.z = res.z / 16;

						// if there are some voxels remaining, add another tile
						if( res.x%16 )
							++tileEnd.x;
						if( res.y%16 )
							++tileEnd.y;
						if( res.z%16 )
							++tileEnd.z;


						math::Vec3i voxelOffset; // start offset (in voxels) for current tile
						math::Vec3i numVoxels;   // number of voxels for current tile (may differ in each dimension)
						std::vector<real32> data;
						int remainK = res.z;
						for( int tk=0; tk<tileEnd.z;++tk )
						{
							voxelOffset.z = tk*16;
							numVoxels.z = std::min( 16,  remainK );

							int remainJ = res.y;
							for( int tj=0; tj<tileEnd.y;++tj )
							{
								voxelOffset.y = tj*16;
								numVoxels.y = std::min( 16,  remainJ );

								int remainI = res.x;
								for( int ti=0; ti<tileEnd.x;++ti )
								{
									voxelOffset.x = ti*16;
									numVoxels.x = std::min( 16,  remainI );

									// write tile ---
									g_writer->jsonBeginArray();
										g_writer->jsonString("compression");
										g_writer->jsonInt32(0);
										g_writer->jsonString("data");
										data.clear();
										math::V3i voxelEnd( voxelOffset.x + numVoxels.x, voxelOffset.y + numVoxels.y, voxelOffset.z + numVoxels.z );
										for( int k=voxelOffset.z;k<voxelEnd.z;++k )
											for( int j=voxelOffset.y;j<voxelEnd.y;++j )
												for( int i=voxelOffset.x;i<voxelEnd.x;++i )
													data.push_back(volume->getVoxel( i, j, k ));
										g_writer->jsonUniformArray(data);
									g_writer->jsonEndArray();

									remainI -=16;
								}
								remainJ -=16;
							}
							remainK -=16;
						}

					g_writer->jsonEndArray();

				g_writer->jsonEndArray();

			g_writer->jsonEndArray();

			g_writer->jsonString("visualization");
			g_writer->jsonBeginMap();
				g_writer->jsonKey("mode");
				g_writer->jsonString("smoke");
				g_writer->jsonKey("iso");
				g_writer->jsonReal32(0.0f);
				g_writer->jsonKey("density");
				g_writer->jsonReal32(1.0f);
			g_writer->jsonEndMap();

			g_writer->jsonString("taperx");
			g_writer->jsonReal32(1.0f);

			g_writer->jsonString("tapery");
			g_writer->jsonReal32(1.0f);



		g_writer->jsonEndArray();

		g_writer->jsonEndArray();
		return true;
	}

}










