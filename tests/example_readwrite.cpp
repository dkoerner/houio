
// this example demonstrates how you would go about reading and writing geometry data from and to houdini files.
// For this it is actually necessary to know the layout/schema of the json structure of houdini files.
// Unfortunately this schema is not documented so you will have to work out the layout from looking
// at the structure of existing houdini files. For this the logger is quite handy. Note that the structure
// differs in some places between the ascii and the binary version of the format. 
// 
// NOTE: the code just gives you an idea how you would import and export houdini geometry, it is not complete
// as the hougeo library has no geometry representation. This is something which is left to the client code.
// This is why in some places functions are commented out which do not exist in this example. These comments
// just give you a hint where you need to extend this example to make it practical.

#include <hougeo/json.h>

#include <iostream>
#include <fstream>


// utility function: turns json array into jsonObject (every first entry is key, every second is value)
hougeo::json::ObjectPtr toObject( hougeo::json::ArrayPtr a )
{
	hougeo::json::ObjectPtr o = hougeo::json::Object::create();

	int numElements = (int)a->size();
	for( int i=0;i<numElements;i+=2 )
	{
		if( a->getValue(i).isString() )
		{
			std::string key = a->get<std::string>(i);
			hougeo::json::Value value = a->getValue(i+1);
			o->append( key, value );
		}
	}

	return o;
}

void import( std::istream *in )
{
		hougeo::json::JSONReader reader;
		hougeo::json::Parser p;
			
		if(!p.parse( in, &reader ))
			throw std::runtime_error("parse failed");
			
		// now the reader contains the json data from the complete file
		// we access it via the root json object.
		
		// For houdini files, the root json object is an array.
		// The reason for that is that the order of items needs
		// to be contained which does not happen for json objects.
		hougeo::json::ArrayPtr root = reader.getRoot().asArray();

		// the houdini root array is a flattened json object
		// in order to work with it more conveniently (e.g. be able
		// to query keys for existance etc.), we unflatten the array
		// into an object again
		hougeo::json::ObjectPtr o = toObject( root );

			
		// now we can start to query the json data for its content. This requires
		// to know the schema of the layout which unfortunately is not documented.
		// The logger can be used to learn the schema from any given file.
		hougeo::sint64 numVertices = 0;
		hougeo::sint64 numPoints = 0;
		hougeo::sint64 numPrimitives = 0;
		if( o->hasKey("pointcount") )
			numPoints = o->get<int>("pointcount", 0);
		if( o->hasKey("vertexcount") )
			numVertices = o->get<int>("vertexcount", 0);
		if( o->hasKey("primitivecount") )
			numPrimitives = o->get<int>("primitivecount", 0);

		std::cout << "#points:" << numPoints << std::endl;
		std::cout << "#verts:" << numVertices << std::endl;
		std::cout << "#prims:" << numPrimitives << std::endl;
				

		if( o->hasKey("attributes") )
		{
			hougeo::json::ObjectPtr attributes = toObject(o->getArray("attributes"));
			if( attributes->hasKey("pointattributes") )
			{
				hougeo::json::ArrayPtr pointAttributes = attributes->getArray("pointattributes");
				hougeo::sint64 numPointAttributes = pointAttributes->size();
				for(int i=0;i<numPointAttributes;++i)
				{
					hougeo::json::ArrayPtr pointAttribute = pointAttributes->getArray(i);
					// here we pass loading the attribute from the json object to a seperate function
					//loadAttribute( pointAttribute, numPoints );
				}
			}
			if( attributes->hasKey("vertexattributes") )
			{
				hougeo::json::ArrayPtr vertexAttributes = attributes->getArray("vertexattributes");
				hougeo::sint64 numVertexAttributes = vertexAttributes->size();
				for(int i=0;i<numVertexAttributes;++i)
				{
					hougeo::json::ArrayPtr vertexAttribute = vertexAttributes->getArray(i);
					// here we pass loading the attribute from the json object to a seperate function
					//loadAttribute( pointAttribute, numPoints );
				}
			}
			if( attributes->hasKey("primitiveattributes") )
			{
				hougeo::json::ArrayPtr primitiveAttributes = attributes->getArray("primitiveattributes");
				hougeo::sint64 numPrimitiveAttributes = primitiveAttributes->size();
				for(int i=0;i<numPrimitiveAttributes;++i)
				{
					hougeo::json::ArrayPtr primitiveAttribute = primitiveAttributes->getArray(i);
					// here we pass loading the attribute from the json object to a seperate function
					//loadAttribute( pointAttribute, numPoints );
				}
			}
			if( attributes->hasKey("globalattributes") )
			{
				hougeo::json::ArrayPtr globalAttributes = attributes->getArray("globalattributes");
				hougeo::sint64 numGlobalAttributes = globalAttributes->size();
				for(int i=0;i<numGlobalAttributes;++i)
				{
					hougeo::json::ArrayPtr globalAttribute = globalAttributes->getArray(i);
					// here we pass loading the attribute from the json object to a seperate function
					//loadAttribute( pointAttribute, numPoints );
				}
			}
		}
		if( o->hasKey("topology") )
		{
			//loadTopology( toObject(o->getArray("topology")) );
		}
		if( o->hasKey("primitives") )
		{
			hougeo::json::ArrayPtr primitives = o->getArray("primitives");
			int numPrimitives = (int)primitives->size();
			for( int j=0;j<numPrimitives;++j )
			{
				hougeo::json::ArrayPtr primitive = primitives->getArray(j);
				//loadPrimitive( primitive );
			}
		}


}


// this is just an example for a geometry interface. In practice this will be replaced
// by the internal geometry representaiton of the client code
struct Geometry
{
	int pointcount(){return 0;}
	int vertexcount(){return 0;}
	int primitivecount(){return 0;}
};

// this functions exports given geometry to houdini format
bool xport( std::ostream *out, Geometry* geo, bool binary )
{
	// we first instatiate a writer depending on wether we
	// want geo (ascii) or bgeo (binary) houdini files
	hougeo::json::Writer* writer=0;
	if( binary )
		writer = new hougeo::json::BinaryWriter( out );
	else
		writer = new hougeo::json::ASCIIWriter( out );

	// we now can write our data using the json events provided
	// by the writer. For this we need to know the schema/layout
	// of the houdini file which unfortunately is not documented.
	writer->jsonBeginArray();

	writer->jsonString( "pointcount" );
	writer->jsonInt( geo->pointcount() );

	writer->jsonString( "vertexcount" );
	writer->jsonInt( geo->vertexcount() );

	writer->jsonString( "primitivecount" );
	writer->jsonInt( geo->primitivecount() );

	// -- topology (required)
	writer->jsonString( "topology" );
	writer->jsonBeginArray();
		//if( geo->getTopology() )
		//	exportTopology( geo->getTopology() );
	writer->jsonEndArray();


	// -- attributes
	writer->jsonString( "attributes" );
	writer->jsonBeginArray();

		// -- point attributes
		writer->jsonString( "pointattributes" );
		writer->jsonBeginArray();
			//std::vector<std::string> pointAttrNames;
			//geo->getPointAttributeNames(pointAttrNames);
			//for( std::vector<std::string>::iterator it = pointAttrNames.begin(); it != pointAttrNames.end(); ++it )
			//	exportAttribute( geo->getPointAttribute(*it) );
		writer->jsonEndArray(); // pointattributes


		// -- primitive attributes
		writer->jsonString( "primitiveattributes" );
		writer->jsonBeginArray();
			//std::vector<std::string> primitiveAttrNames;
			//geo->getPrimitiveAttributeNames(primitiveAttrNames);
			//for( std::vector<std::string>::iterator it = primitiveAttrNames.begin(); it != primitiveAttrNames.end(); ++it )
			//	exportAttribute( geo->getPrimitiveAttribute(*it) );
		writer->jsonEndArray(); // primitiveattributes


		// -- global attributes
		writer->jsonString( "globalattributes" );
		writer->jsonBeginArray();
			//std::vector<std::string> globalAttrNames;
			//geo->getGlobalAttributeNames(globalAttrNames);
			//for( std::vector<std::string>::iterator it = globalAttrNames.begin(); it != globalAttrNames.end(); ++it )
			//	exportAttribute( geo->getGlobalAttribute(*it) );
		writer->jsonEndArray(); // globalattributes

	writer->jsonEndArray(); // attributes


	// -- primitives
	if( geo->primitivecount() > 0 )
	{
		writer->jsonString( "primitives" );
		writer->jsonBeginArray();
		for( int i=0;i<geo->primitivecount();++i )
		{
			//HouGeoAdapter::Primitive::Ptr prim = geo->getPrimitive(i);
			//if( std::dynamic_pointer_cast<HouGeoAdapter::VolumePrimitive>(prim) )
			//	exportPrimitive(std::dynamic_pointer_cast<HouGeoAdapter::VolumePrimitive>(prim));

		}
		writer->jsonEndArray(); // primitives
	}


	writer->jsonEndArray(); // /root

	// done
	delete writer;

	return true;
}


int main(void)
{
	// example for reading houdini geometry
	{
		std::string path( std::string(TESTS_FILE_PATH)+"/test_box.bgeo" );
		std::ifstream in( path.c_str(), std::ios_base::in | std::ios_base::binary );
		import(&in);
	}

	// example for exporting houdini geometry
	{
		std::string path( "example_write.bgeo" );
		std::ofstream out( path.c_str(), std::ios_base::out | std::ios_base::binary );
		Geometry geo;
		xport( &out, &geo, true );
	}

	return 0;
}
