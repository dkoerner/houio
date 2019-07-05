#include <houio/Attribute.h>
#include <houio/types.h>

#include <iostream>
#include <cstring>



namespace houio
{
	Attribute::Attribute( char numComponents, ComponentType componentType ) :
		m_componentType(componentType),
		m_numComponents(numComponents),
		m_numElements(0),
		m_isDirty(true)
	{
		switch(componentType)
		{
		case INT:
			{
				m_componentSize=sizeof(int);
			}break;
		default:
		case FLOAT:
			{
				m_componentSize=sizeof(float);
			}break;
		};

	}

	Attribute::~Attribute()
	{
	}

	Attribute::Ptr Attribute::copy()
	{
		return create( numComponents(), elementComponentType(), (unsigned char *)getRawPointer(), numElements() );
	}

	Attribute::ComponentType Attribute::elementComponentType()
	{
		return m_componentType;
	}


	Attribute::Ptr Attribute::create(char numComponents, ComponentType componentType, unsigned char *raw, int numElements)
	{
		Attribute::Ptr attr = std::make_shared<Attribute>( numComponents, componentType );

		attr->m_numElements = numElements;
		int size = attr->elementComponentSize()*attr->numComponents()*attr->numElements();
		attr->m_data.resize( size );
		memcpy( &attr->m_data[0], raw, size );

		return attr;
	}

	

	Attribute::Ptr Attribute::createM33f()
	{
		return Attribute::Ptr( new Attribute(9, Attribute::FLOAT) );
	}

	Attribute::Ptr Attribute::createM44f()
	{
		return Attribute::Ptr( new Attribute(16, Attribute::FLOAT) );
	}

	Attribute::Ptr Attribute::createV4f( int numElements )
	{
		Attribute::Ptr attr = Attribute::Ptr( new Attribute(4, Attribute::FLOAT) );
		attr->resize(numElements);
		return attr;
	}

	Attribute::Ptr Attribute::createV3f( int numElements )
	{
		Attribute::Ptr attr = Attribute::Ptr( new Attribute(3, Attribute::FLOAT) );
		attr->resize(numElements);
		return attr;
	}

	Attribute::Ptr Attribute::createV2f( int numElements )
	{
		Attribute::Ptr attr = Attribute::Ptr( new Attribute(2, Attribute::FLOAT) );
		attr->resize(numElements);
		return attr;
	}

	Attribute::Ptr Attribute::createFloat( int numElements )
	{
		Attribute::Ptr attr = Attribute::Ptr( new Attribute(1, Attribute::FLOAT) );
		attr->resize(numElements);
		return attr;
	}

	Attribute::Ptr Attribute::createInt( int numElements )
	{
		Attribute::Ptr attr = Attribute::Ptr( new Attribute(1, Attribute::INT) );
		attr->resize(numElements);
		return attr;
	}


	int Attribute::componentSize( ComponentType ct )
	{
		switch(ct)
		{
		case INT:return sizeof(sint32);
		case FLOAT:return sizeof(real32);
		//case REAL64:return sizeof(real64);
		default:
			throw std::runtime_error( "unknown component type" );
		};
		return -1;
	}


	Attribute::ComponentType Attribute::componentType( const std::string& ct )
	{
		if( ct == "fpreal32" )
			return FLOAT;
		else
		if( ct == "float" )
			return FLOAT;
		else
		//if( ct == "fpreal64" )
		//	return REAL64;
		//else
		if( ct == "int32" )
			return INT;
		else
		if( ct == "int" )
			return INT;
		return INVALID;
	}


}
