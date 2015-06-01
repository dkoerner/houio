#include <houio/HouGeo.h>

#include <cstring>




namespace houio
{


	HouGeo::HouGeo() : HouGeoAdapter()
	{
	}

	sint64 HouGeo::pointcount()const
	{
		auto it = m_pointAttributes.cbegin();
		if( it != m_pointAttributes.cend() )
			return it->second->getNumElements();
		return 0;
	}

	sint64 HouGeo::vertexcount()const
	{
		if( m_topology )
			return m_topology->getNumIndices();
		return 0;
	}

	sint64 HouGeo::primitivecount()const
	{
		return m_primitives.size();
	}

	void HouGeo::getPointAttributeNames( std::vector<std::string> &names )const
	{
		for( auto it = m_pointAttributes.cbegin(); it != m_pointAttributes.cend(); ++it )
			names.push_back( it->second->getName() );
	}

	HouGeoAdapter::AttributeAdapter::Ptr HouGeo::getPointAttribute( const std::string &name )
	{
		auto it = m_pointAttributes.find(name);
		if(it != m_pointAttributes.end())
			return it->second;
		return HouGeoAdapter::AttributeAdapter::Ptr();
	}

	void HouGeo::getVertexAttributeNames( std::vector<std::string> &names )const
	{
		for( auto it = m_vertexAttributes.cbegin(); it != m_vertexAttributes.cend(); ++it )
			names.push_back( it->second->getName() );
	}

	HouGeoAdapter::AttributeAdapter::Ptr HouGeo::getVertexAttribute( const std::string &name )
	{
		auto it = m_vertexAttributes.find(name);
		if(it != m_vertexAttributes.end())
			return it->second;
		return HouGeoAdapter::AttributeAdapter::Ptr();
	}


	void HouGeo::getGlobalAttributeNames( std::vector<std::string> &names )const
	{
		for( auto it = m_globalAttributes.cbegin(); it != m_globalAttributes.cend(); ++it )
			names.push_back( it->second->getName() );
	}

	HouGeoAdapter::AttributeAdapter::Ptr HouGeo::getGlobalAttribute( const std::string &name )
	{
		auto it = m_globalAttributes.find(name);
		if(it != m_globalAttributes.end())
			return it->second;
		return HouGeoAdapter::AttributeAdapter::Ptr();
	}


	bool HouGeo::hasPrimitiveAttribute( const std::string &name )const
	{
		auto it = m_primitiveAttributes.find( name );
		if( it != m_primitiveAttributes.end() )
			return true;
		return false;
	}

	void HouGeo::getPrimitiveAttributeNames( std::vector<std::string> &names )const
	{
		for( auto it = m_primitiveAttributes.cbegin(); it != m_primitiveAttributes.cend(); ++it )
			names.push_back( it->second->getName() );
	}

	HouGeoAdapter::AttributeAdapter::Ptr HouGeo::getPrimitiveAttribute( const std::string &name )
	{
		auto it = m_primitiveAttributes.find(name);
		if(it != m_primitiveAttributes.end())
			return it->second;
		return HouGeoAdapter::AttributeAdapter::Ptr();
	}



	HouGeo::Primitive::Ptr HouGeo::getPrimitive( int index )
	{
		if( index < m_primitives.size() )
			return m_primitives[index];
		return HouGeo::Primitive::Ptr();
	}

	HouGeo::Topology::Ptr HouGeo::getTopology()
	{
		return m_topology;
	}


	HouGeo::Ptr HouGeo::create()
	{
		return HouGeo::Ptr( new HouGeo() );
	}

	void HouGeo::addPrimitive( ScalarField::Ptr field )
	{
		// add point which will encode the translation of the volume
		HouAttribute::Ptr pAttr = std::dynamic_pointer_cast<HouAttribute>(getPointAttribute( "P" ));

		// no point attribute yet?
		if( !pAttr )
		{
			// add point attribute
			pAttr = std::make_shared<HouAttribute>();
			pAttr->m_name = "P";
			pAttr->tupleSize = 4;
			pAttr->m_storage = HouAttribute::ATTR_STORAGE_FPREAL32;
			pAttr->m_type = HouAttribute::ATTR_TYPE_NUMERIC;
			pAttr->m_attr = Attribute::createV4f();
			setPointAttribute( pAttr );
		}

		math::V3f center = field->localToWorld( math::V3f(0.5f) );
		int index = pAttr->m_attr->appendElement<math::V4f>(math::V4f(center.x, center.y, center.z, 1.0f));

		if( !m_topology )
			m_topology = std::make_shared<HouTopology>();

		HouVolume::Ptr hvol = std::make_shared<HouVolume>();
		hvol->field = field;

		std::vector<int> indexList;
		indexList.push_back( index );

		hvol->vertex = m_topology->getNumIndices();
		m_topology->addIndices(indexList);


		m_primitives.push_back( hvol );
	}


	void HouGeo::setPointAttribute( HouAttribute::Ptr attr )
	{
		m_pointAttributes[attr->getName()] = attr;
	}

	void HouGeo::setPrimitiveAttribute( const std::string &name, HouAttribute::Ptr attr )
	{
		attr->m_name = name;
		m_primitiveAttributes[name] = attr;
	}

	// Attribute ==============================

	HouGeo::HouAttribute::HouAttribute() : AttributeAdapter(), m_type(HouGeoAdapter::AttributeAdapter::ATTR_TYPE_NUMERIC)
	{
		m_name = "unnamed";
		numElements = 0;
	}

	HouGeo::HouAttribute::HouAttribute( const std::string &name, Attribute::Ptr attr ) : AttributeAdapter(),
		m_attr(attr),
		m_name(name),
		m_type(HouGeoAdapter::AttributeAdapter::ATTR_TYPE_NUMERIC),
		m_storage(ATTR_STORAGE_FPREAL32),
		tupleSize(attr->numComponents()),
		numElements(attr->numElements())
	{
		switch( m_attr->elementComponentType() )
		{
		case Attribute::FLOAT:
			m_storage = ATTR_STORAGE_FPREAL32;break;
		//case ::Attribute::REAL64:
		//	m_storage = ATTR_STORAGE_FPREAL64;break;
		case Attribute::INT:
			m_storage = ATTR_STORAGE_INT32;break;
		default:
			throw std::runtime_error("HouGeo::HouAttribute::HouAttribute - unsupported attribute type");
		}
	}

	std::string HouGeo::HouAttribute::getName()const
	{
		return m_name;
	}

	HouGeoAdapter::AttributeAdapter::Type HouGeo::HouAttribute::getType()const
	{
		return m_type;
	}

	int HouGeo::HouAttribute::getTupleSize()const
	{
		if( m_attr )
			return m_attr->numComponents();
		return tupleSize;
	}

	HouGeoAdapter::AttributeAdapter::Storage HouGeo::HouAttribute::getStorage()const
	{
		return m_storage;
	}

	void HouGeo::HouAttribute::getPacking( std::vector<int> &packing )const
	{
	}

	HouGeoAdapter::RawPointer::Ptr HouGeo::HouAttribute::getRawPointer()
	{
		if( m_attr )
			return HouGeoAdapter::RawPointer::create( m_attr->getRawPointer() );
		//if( !data.empty() )
		//	return HouGeoAdapter::RawPointer::create( &data[0] );
		return HouGeoAdapter::RawPointer::create( 0 );
	}

	int HouGeo::HouAttribute::getNumElements()const
	{
		if( m_attr )
			return m_attr->numElements();
		return numElements;
	}


	/*
	int HouGeo::HouAttribute::addV4f( math::V4f value )
	{
		// TODO: check storage
		// TODO: check type

		if( tupleSize != 4 )
			qCritical() << "tupleSize does not match!";

		int elementSize = storageSize( m_storage )*tupleSize;
		data.resize( data.size() + elementSize );

		*((math::V4f *)(&data[ numElements * elementSize ])) = value;

		return numElements++;
	}
	*/

	int HouGeo::HouAttribute::addString(const std::string &value)
	{
		// TODO: check storage
		// TODO: check type
		strings.push_back(value);
		m_type = ATTR_TYPE_STRING;
		//attr->storage = attrStorage;
		tupleSize = 1;
		return numElements++;
	}

	std::string HouGeo::HouAttribute::getString( int index )const
	{
		return strings[index];
	}




	// Topology ==============================

	void HouGeo::HouTopology::getIndices( std::vector<int> &indices )const
	{
		indices.clear();
		indices.insert( indices.begin(), indexBuffer.begin(), indexBuffer.end() );
	}

	void HouGeo::HouTopology::addIndices( std::vector<int> &indices )
	{
		indexBuffer.insert( indexBuffer.end(), indices.begin(), indices.end() );
	}

	sint64 HouGeo::HouTopology::getNumIndices()const
	{
		return indexBuffer.size();
	}




	// a has to be the root of the array from hou geo
	void HouGeo::load( json::ObjectPtr o )
	{
		SharedPrimitiveData sharedPrimitiveData;

		sint64 numVertices = 0;
		sint64 numPoints = 0;
		sint64 numPrimitives = 0;
		if( o->hasKey("pointcount") )
			numPoints = o->get<int>("pointcount", 0);
		if( o->hasKey("vertexcount") )
			numVertices = o->get<int>("vertexcount", 0);
		if( o->hasKey("primitivecount") )
			numPrimitives = o->get<int>("primitivecount", 0);
		if( o->hasKey("attributes") )
		{
			json::ObjectPtr attributes = toObject(o->getArray("attributes"));
			if( attributes->hasKey("pointattributes") )
			{
				json::ArrayPtr pointAttributes = attributes->getArray("pointattributes");
				sint64 numPointAttributes = pointAttributes->size();
				for(int i=0;i<numPointAttributes;++i)
				{
					json::ArrayPtr pointAttribute = pointAttributes->getArray(i);
					HouAttribute::Ptr attr = loadAttribute( pointAttribute, numPoints );
					m_pointAttributes.insert( std::make_pair(attr->getName(), attr) );
				}
			}
			if( attributes->hasKey("vertexattributes") )
			{
				json::ArrayPtr vertexAttributes = attributes->getArray("vertexattributes");
				sint64 numVertexAttributes = vertexAttributes->size();
				for(int i=0;i<numVertexAttributes;++i)
				{
					json::ArrayPtr vertexAttribute = vertexAttributes->getArray(i);
					HouAttribute::Ptr attr = loadAttribute( vertexAttribute, numVertices );
					m_vertexAttributes.insert( std::make_pair(attr->getName(), attr) );
				}
			}
			if( attributes->hasKey("primitiveattributes") )
			{
				json::ArrayPtr primitiveAttributes = attributes->getArray("primitiveattributes");
				sint64 numPrimitiveAttributes = primitiveAttributes->size();
				for(int i=0;i<numPrimitiveAttributes;++i)
				{
					json::ArrayPtr primitiveAttribute = primitiveAttributes->getArray(i);
					HouAttribute::Ptr attr = loadAttribute( primitiveAttribute, numPrimitives );
					m_primitiveAttributes.insert( std::make_pair(attr->getName(), attr) );
				}
			}
			if( attributes->hasKey("globalattributes") )
			{
				json::ArrayPtr globalAttributes = attributes->getArray("globalattributes");
				sint64 numGlobalAttributes = globalAttributes->size();
				for(int i=0;i<numGlobalAttributes;++i)
				{
					json::ArrayPtr globalAttribute = globalAttributes->getArray(i);
					// TODO: element count argument, how many?!
					HouAttribute::Ptr attr = loadAttribute( globalAttribute, 1 );
					m_globalAttributes.insert( std::make_pair(attr->getName(), attr) );
				}
			}
		}
		if( o->hasKey("topology") )
		{
			loadTopology( toObject(o->getArray("topology")) );
		}
		if( o->hasKey("sharedprimitivedata") )
		{
			json::ArrayPtr entries = o->getArray("sharedprimitivedata");

			int numEntries = (int)entries->size()/2;
			for( int i=0;i<numEntries;++i )
			{
				int index = i*2;
				std::string type_questionmark = entries->get<std::string>(index);
				json::ArrayPtr entry = entries->getArray(index+1);

				std::string type2_questionmark = entry->get<std::string>(0);
				std::string id = entry->get<std::string>(1);
				json::ArrayPtr data = entry->getArray(2);

				sharedPrimitiveData.sharedVoxelData[id] = toObject(data);
			}
		}
		if( o->hasKey("primitives") )
		{
			json::ArrayPtr primitives = o->getArray("primitives");
			int numPrimitives = (int)primitives->size();
			for( int j=0;j<numPrimitives;++j )
			{
				json::ArrayPtr primitive = primitives->getArray(j);
				loadPrimitive( primitive, sharedPrimitiveData );
			}
		}
	}


	HouGeo::HouAttribute::Ptr HouGeo::loadAttribute( json::ArrayPtr attribute, sint64 elementCount )
	{
		json::ObjectPtr attrDef = toObject(attribute->getArray(0));
		json::ObjectPtr attrData = toObject(attribute->getArray(1));

		HouGeo::HouAttribute::Ptr attr = std::make_shared<HouGeo::HouAttribute>();

		std::string attrName = attrDef->get<std::string>("name");
		AttributeAdapter::Type attrType = AttributeAdapter::type(attrDef->get<std::string>("type"));

		if( attrType == AttributeAdapter::ATTR_TYPE_NUMERIC )
		{
			AttributeAdapter::Storage attrStorage = AttributeAdapter::storage(attrData->get<std::string>("storage"));
			int attrTupleSize = attrData->get<int>("size");

			Attribute::ComponentType attrComponentType = Attribute::componentType(attrData->get<std::string>("storage"));
			int attrNumComponents = attrData->get<int>("size");
			attr->m_attr = std::make_shared<Attribute>( attrNumComponents, attrComponentType );
			attr->m_attr->resize(elementCount);
			char *data = (char*)attr->m_attr->getRawPointer();

			int attrComponentSize = AttributeAdapter::storageSize( attrStorage );
			int dstTupleSize = attrTupleSize;
			int dstComponentSize = attrComponentSize;
			//attr->data.resize( elementCount*dstTupleSize*dstComponentSize );

			if( attrData->hasKey("values") )
			{
				json::ObjectPtr values = toObject( attrData->getArray("values") );
				if( values->hasKey("rawpagedata") )
				{
					int elementsPerPage = values->get<int>("pagesize");

					// one pack is a sequence of components
					// packing is used to describe in which sequence components are written to the file
					// packing allows to store vectors as list of structs or struct of lists.
					std::vector<ubyte> attrPacking;
					if( values->hasKey("packing") )
					{
						json::ArrayPtr packingArray = values->getArray("packing");
						int psize = (int)packingArray->size();
						for( int i=0;i<psize;++i )
						{
							attrPacking.push_back( packingArray->get<ubyte>(i) );
						}
					}else
						attrPacking.push_back( attrTupleSize );

					// constantpageflags is an array which
					// contains an array for each pack
					// each of those per pack arrays contains flags for each page
					// which tell us wether the pack is constant for this page
					std::vector<std::vector<bool>> constantPageFlagsPerPack;

					// to make things even more fun, some packs can be constant
					// and this may be different per page - oh boy
					if( values->hasKey("constantpageflags") )
					{
						json::ArrayPtr constantPageFlags = values->getArray("constantpageflags");

						// for each pack
						int i=0;
						for( auto it = attrPacking.begin(); it != attrPacking.end();++it,++i )
						{
							constantPageFlagsPerPack.push_back(std::vector<bool>());

							// get array which tells us for each page if the pack is constant
							json::ArrayPtr packConstantFlags = constantPageFlags->getArray(i);

							for( int j=0;j<packConstantFlags->size();++j )
								constantPageFlagsPerPack.back().push_back( packConstantFlags->get<bool>(j) );
						}
					}else
					{
						for( int j=0;j<attrTupleSize;++j )
							constantPageFlagsPerPack.push_back(std::vector<bool>());
					}

					json::ArrayPtr rawPageData = values->getArray("rawpagedata");

					// we need to repack - which when done in a generic way looks like a pain in the butt ======

					attr->numElements = elementCount;
					int elementsRemaining = attr->numElements;
					//qDebug() << "numElements " << attr->numElements;
					//qDebug() << "rawPageData->size() " << (int)rawPageData->size();
					//qDebug() << "attrTupleSize " << attrTupleSize;

					// process each page
					int pageIndex = 0;
					int pageStartIndex = 0;
					while( elementsRemaining>0 )
					{
						int pageStartElement = pageIndex*elementsPerPage;
						int numElements = std::min( elementsRemaining, elementsPerPage );

						// process each pack
						int packIndex = 0;
						ubyte startComponentIndex = 0;
						for( std::vector<ubyte>::iterator it = attrPacking.begin(); it != attrPacking.end();++it, ++packIndex )
						{
							ubyte pack = *it;
							int maxPack = std::min( (int)pack, std::max(0, dstTupleSize-startComponentIndex) );

							if( maxPack == 0 )
								break;

							// is pack for current page constant?
							bool isConstant = constantPageFlagsPerPack[packIndex].empty() ? false : constantPageFlagsPerPack[packIndex][pageIndex];
							//qDebug() << "constant? " << isConstant;


							// if pack is constant only the first element is given, this is the reference
							// find element index where the new page starts
							int elementIndex = pageStartIndex;

							// now iterate over all elements of current page and get values from current pack
							for( int i=0;i<numElements;++i )
							{
								// we update elementIndex only if pack is varying within current page
								// otherwise we will just keep pointing to the reference element
								if( !isConstant )
									// get page element index into rawpagedata for current pack
									// we can do pageStartElement*attrTupleSize because packing doesnt matter for past pages
									elementIndex = pageStartIndex + i*pack;
									//qDebug() << "elementIndex " << elementIndex;
									//qDebug() << "pageStartElement " << pageStartElement;
									//qDebug() << "attrTupleSize " << attrTupleSize;
									//qDebug() << "i " << i;
									//qDebug() << "pack " << pack;

								// get global element index for writing into our dense array
								int destElementIndex = (pageStartElement+i)*dstTupleSize;

								// for each component of current pack
								for( int component=0;component<maxPack;++component )
									// get component value from current rawpagedata
									// and copy that component to the location of that component in dense array
									// TODO: uniform arrays!
									rawPageData->getValue(elementIndex+component).cpyTo( (char *)&(data[(destElementIndex + startComponentIndex + component)*dstComponentSize]) );
							}


							startComponentIndex += pack;
							if( !isConstant )
								pageStartIndex += numElements*pack;
							else
								pageStartIndex += pack;
						}


						elementsRemaining -= numElements;
						pageStartElement += numElements;

						// proceed next page
						++pageIndex;
					}

					attr->m_name = attrName;
					attr->m_type = attrType;
					attr->m_storage = attrStorage;
					attr->tupleSize = dstTupleSize;
				}
			}
		}else
		if( attrType == AttributeAdapter::ATTR_TYPE_STRING )
		{
			if( attrData->hasKey("strings") )
			{
				json::ArrayPtr stringsArray = attrData->getArray("strings");
				int numElements = stringsArray->size();
				for( int i=0;i<numElements;++i )
				{
					std::string string = stringsArray->get<std::string>( i );
					attr->strings.push_back(string);
					//qDebug() << QString::fromStdString(string);
				}
				attr->numElements = numElements;

				attr->m_name = attrName;
				attr->m_type = attrType;
				//attr->storage = attrStorage;
				attr->tupleSize = 1;
			}
		}

		return attr;
	}


	void HouGeo::loadTopology( json::ObjectPtr o )
	{
		HouTopology::Ptr top = std::make_shared<HouTopology>();
		if( o->hasKey("pointref") )
		{
			json::ObjectPtr pointref = toObject( o->getArray("pointref") );
			if( pointref->hasKey("indices") )
			{
				json::ArrayPtr indices = pointref->getArray("indices");
				sint64 numElements = indices->size();
				for(int i=0;i<numElements;++i)
					top->indexBuffer.push_back( indices->get<int>(i) );
			}
		}
		m_topology = top;
	}

	void HouGeo::loadPrimitive( json::ArrayPtr primitive, SharedPrimitiveData& sharedPrimitiveData )
	{
		// we follow the scheme from houdini...

		// primitives have 2 arrays:
		//primitive definition
		json::ObjectPtr primdef = toObject(primitive->getArray(0));
		std::string primitiveType ="";
		if( primdef->hasKey("type") )
			primitiveType = primdef->get<std::string>("type", "");


		// primitive
		if( primitiveType=="Volume" )
			loadVolumePrimitive( toObject(primitive->getArray(1)), sharedPrimitiveData );
		else
		if( primitiveType=="Poly" )
			loadPolyPrimitive( toObject(primitive->getArray(1)) );
		else
		if( (primitiveType=="run")&&(primdef->hasKey("runtype")) )
		{
			if( primdef->get<std::string>( "runtype" ) == "Poly" )
				loadPolyPrimitiveRun( primdef, primitive->getArray(1) );
		}

	}

	// HouGeo::HouVolume ==================================================

	void HouGeo::loadVolumePrimitive( json::ObjectPtr volume, SharedPrimitiveData& sharedPrimitiveData )
	{
		HouVolume::Ptr vol = std::make_shared<HouVolume>();
		vol->field = std::make_shared<ScalarField>();

		if( volume->hasKey("res") )
		{
			json::ArrayPtr res = volume->getArray("res");
			vol->field->resize(res->get<int>(0), res->get<int>(1), res->get<int>(2));
		}
		if( volume->hasKey("vertex") && volume->hasKey("transform") )
		{
			json::ArrayPtr xform = volume->getArray("transform");

			// vertex indexes the indexbuffer (which lives with the topology)
			// the index buffer references the point (which lives with the point attribute)
			// from the point we can work out the translation of the volume

			// transform encodes the rotation and scale in an array of 9 values
			math::Matrix44d houLocalToWorldRotationScale = math::Matrix44d( xform->get<float>(0), xform->get<float>(1), xform->get<float>(2), 0.0,
																			xform->get<float>(3), xform->get<float>(4), xform->get<float>(5), 0.0,
																			xform->get<float>(6), xform->get<float>(7), xform->get<float>(8), 0.0,
																			0.0, 0.0, 0.0, 1.0);


			int v = m_topology->indexBuffer[volume->get<int>("vertex")];
			math::V3f p(0.0f);

			// TODO!!! - make this more generic - kind of hardcode
			{
				HouAttribute::Ptr pAttr = std::dynamic_pointer_cast<HouAttribute>( getPointAttribute("P") );
				switch(pAttr->m_storage)
				{
				// in case of 4 component vector, w component will be ignored...
				case AttributeAdapter::ATTR_STORAGE_FPREAL32:
					{
						//p = *(math::V3f *)&m_pointAttributes.find("P")->second->data[ v*pAttr->tupleSize*sizeof(float) ];
						p = m_pointAttributes.find("P")->second->m_attr->get<math::V3f>( v );
					}break;
				case AttributeAdapter::ATTR_STORAGE_FPREAL64:
					{
						//p = *(math::V3d *)&m_pointAttributes.find("P")->second->data[ v*pAttr->tupleSize*sizeof(double) ];
						p = m_pointAttributes.find("P")->second->m_attr->get<math::V3d>( v );
					}break;
				}
			}
			math::Matrix44d houLocalToWorldTranslation = math::Matrix44d::TranslationMatrix(p);

			math::Matrix44d localToWorld = math::Matrix44d::ScaleMatrix(2.0)*math::Matrix44d::TranslationMatrix(-1.0,-1.0,-1.0)*houLocalToWorldRotationScale*houLocalToWorldTranslation;

			vol->field->setLocalToWorld( localToWorld );
		}

		if( volume->hasKey("sharedvoxels") )
		{
			std::string dataid = volume->get<std::string>("sharedvoxels");
			auto it = sharedPrimitiveData.sharedVoxelData.find(dataid);
			if( it != sharedPrimitiveData.sharedVoxelData.end() )
			{
				json::ObjectPtr voxels = it->second;
				loadVoxelData( voxels, vol->field->getResolution(), vol->field->getRawPointer() );
			}else
				throw std::runtime_error( "HouGeo::loadVolumePrimitive: error shared voxel data not found\n" );
		}

		if( volume->hasKey("voxels") )
		{
			loadVoxelData( toObject(volume->getArray("voxels")), vol->field->getResolution(), vol->field->getRawPointer() );
		}

		m_primitives.push_back( vol );
	}

	void HouGeo::loadVoxelData( json::ObjectPtr voxels, const math::V3i& res, float* volData )
	{
		if( voxels->hasKey("tiledarray") )
		{
			json::ObjectPtr tiledarray = toObject(voxels->getArray("tiledarray"));

			std::vector<int> compressionTypes;
			// 1 = rawfull
			// 2 = constant

			if( tiledarray->hasKey("compressiontypes") )
			{
				json::ArrayPtr ct = tiledarray->getArray("compressiontypes");
				for( int cti=0;cti<ct->size();++cti )
				{
					//std::cout << ct->get<std::string>(cti) << std::endl;
					if( ct->get<std::string>(cti) == "raw" )
						compressionTypes.push_back( 0 );
					else
					if( ct->get<std::string>(cti) == "rawfull" )
						compressionTypes.push_back( 1 );
					else
					if( ct->get<std::string>(cti) == "constant" )
						compressionTypes.push_back( 2 );
					else
						compressionTypes.push_back( -1 );
				}

			}
			if( tiledarray->hasKey("tiles") )
			{
				json::ArrayPtr tiles = tiledarray->getArray("tiles");
				sint64 numTiles = tiles->size();

				// looks like houdini uses some spatial tiling where each tile has
				// a resolution of 16x16x16 (or less on boundary tiles)
				// the whole resolution domain is split into such tiles
				// the first tile starts at 0,0,0 and the ordering is identical
				// to the ordering of voxel values (x being the fastest and z slowest)

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

				// sanity check - number of tiles has to match
				if( (tileEnd.x*tileEnd.y*tileEnd.z)!=numTiles )
					throw std::runtime_error("HouGeo::loadVolumePrimitive problem");

				int currentTileIndex = 0;
				math::Vec3i voxelOffset; // start offset (in voxels) for current tile
				math::Vec3i numVoxels;   // number of voxels for current tile (may differ in each dimension)

				// we iterate all tiles starting from slowest to fastest (inner loop)
				// in each iteration we compute the amount of remaining voxels which is either 16 or smaller for boundary tiles
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
						for( int ti=0; ti<tileEnd.x;++ti, ++currentTileIndex )
						{
							voxelOffset.x = ti*16;
							numVoxels.x = std::min( 16,  remainI );

							json::ObjectPtr tile = toObject(tiles->getArray(currentTileIndex));
							int tileCompression = 1;
							if( tile->hasKey("compression") )
							{
								tileCompression = tile->get<sint32>("compression");
							}
							if( tile->hasKey("data") )
							{
								switch( tileCompression )
								{
								case 0: // raw
								case 1: // rawfull
									{
										json::ArrayPtr data = tile->getArray("data");
										int numElements = (int)data->size();

										if( (numVoxels.x*numVoxels.y*numVoxels.z)!=numElements )
											throw std::runtime_error("HouGeo::loadVolumePrimitive problem");

										if( data->isUniform() && (data->m_uniformType == 2) )
										{
											int v = 0;
											for( int k=0;k<numVoxels.z;++k )
												for( int j=0;j<numVoxels.y;++j, v+=numVoxels.x )
													// copy a complete scanline directly
													memcpy( &volData[(tk*16+k)*res.x*res.y + (tj*16+j)*res.x + (ti*16)], &data->m_uniformdata[v*sizeof(float)], numVoxels.x*sizeof(float) );
										}else
										{
											int v = 0;
											for( int k=0;k<numVoxels.z;++k )
												for( int j=0;j<numVoxels.y;++j )
													for( int i=0;i<numVoxels.x;++i,++v )
														volData[(tk*16+k)*res.x*res.y + (tj*16+j)*res.x + (ti*16+i)] = data->get<float>(v);
										}
									}break;
								case 2: // constant
									{
										float data = tile->get<float>("data");
										int v = 0;
										for( int k=0;k<numVoxels.z;++k )
											for( int j=0;j<numVoxels.y;++j )
												for( int i=0;i<numVoxels.x;++i,++v )
													volData[(tk*16+k)*res.x*res.y + (tj*16+j)*res.x + (ti*16+i)] = data;

									}break;
								case -1:
								default:
									throw std::runtime_error("HouGeo::loadVolumePrimitive unknown compressiontype");
									break;
								};
							}
							remainI -=16;
						}
						remainJ -=16;
					}
					remainK -=16;
				}
			}
		}else // /tiledarray
		if( voxels->hasKey("constantarray") )
		{
			float constantValue = voxels->get<float>( "constantarray" );
			std::fill( volData, volData + res.x*res.y*res.z, constantValue );
		}
	}

	int HouGeo::HouVolume::getVertex()const
	{
		return vertex;
	}

	real32 HouGeo::HouVolume::getVoxel( int i, int j, int k )const
	{
		return field->sample(i, j, k);
	}
	
	math::Vec3i HouGeo::HouVolume::getResolution()const
	{
		return field->getResolution();
	}

	math::M44f HouGeo::HouVolume::getTransform()const
	{
		return field->m_localToWorld;
	}

	// returns raw pointer to the data
	HouGeoAdapter::RawPointer::Ptr HouGeo::HouVolume::getRawPointer()
	{
		return HouGeoAdapter::RawPointer::create(field->getRawPointer());
	}


	// HouGeo::HouPoly ==================================================
	void HouGeo::loadPolyPrimitive( json::ObjectPtr poly )
	{
		HouPoly::Ptr pol = std::make_shared<HouPoly>();

		if( !m_topology )
			throw std::runtime_error( "HouGeo::loadPolyPrimitive expects topology to be loaded already!" );

		if( poly->hasKey( "vertex" ) )
		{
			json::ArrayPtr vertex = poly->getArray("vertex");
			// these are indices into, well, indices...
			int numVertices = (int) vertex->size();
			pol->m_numPolys = 1;
			pol->m_perPolyVertexCount.push_back(numVertices);
			for(int i=0;i<numVertices;++i)
				pol->m_vertices.push_back( m_topology->indexBuffer[vertex->get<sint32>(i)] );
		}

		m_primitives.push_back( pol );
	}

	void HouGeo::loadPolyPrimitiveRun( json::ObjectPtr def, json::ArrayPtr run )
	{
		HouPoly::Ptr pol = std::make_shared<HouPoly>();
		pol->m_numPolys = (int) run->size();
		int vertex = 0;
		for( int i=0;i<pol->m_numPolys;++i )
		{
			json::ArrayPtr c = run->getArray(i)->getArray(0);
			int numVertices = (int)c->size();
			pol->m_perPolyVertexCount.push_back(numVertices);
			for( int j=0;j<numVertices; ++j, ++vertex )
				pol->m_vertices.push_back(m_topology->indexBuffer[c->get<sint32>(j)]);
		}
		m_primitives.push_back( pol );
	}

	int HouGeo::HouPoly::numPolys()const
	{
		return m_numPolys;
	}

	int HouGeo::HouPoly::numVertices( int poly )const
	{
		return m_perPolyVertexCount[poly];
	}

	int const *HouGeo::HouPoly::vertices()const
	{
		return &m_vertices[0];
	}



	// MISC =======================================================

	// turns json array into jsonObject (every first entry is key, every second is value)
	json::ObjectPtr HouGeo::toObject( json::ArrayPtr a )
	{
		json::ObjectPtr o = json::Object::create();

		int numElements = (int)a->size();
		for( int i=0;i<numElements;i+=2 )
		{
			if( a->getValue(i).isString() )
			{
				std::string key = a->get<std::string>(i);
				json::Value value = a->getValue(i+1);
				o->append( key, value );
			}
		}

		return o;
	}








	




}  // namespace houio
