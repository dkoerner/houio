#pragma once
#include <memory>
#include <vector>
#include <houio/math/Math.h>



namespace houio
{

	// basicly a list manager
	struct Attribute
	{
		typedef std::shared_ptr<Attribute> Ptr;
		enum ComponentType
		{
			INVALID,
			INT,
			FLOAT
		};

		Attribute( char numComponents=3, ComponentType componentType = FLOAT );
		~Attribute();

		Attribute::Ptr copy();

		template<typename T>
		unsigned int appendElement( const T &value );
		template<typename T>
		unsigned int appendElement( const T &v0, const T &v1 );
		template<typename T>
		unsigned int appendElement( const T &v0, const T &v1, const T &v2 );
		template<typename T>
		unsigned int appendElement( const T &v0, const T &v1, const T &v2, const T &v3 );

		template<typename T>
		T &get( unsigned int index );

		template<typename T>
		void set( unsigned int index, T value );
		template<typename T>
		void set( unsigned int index, T v0, T v1 );
		template<typename T>
		void set( unsigned int index, T v0, T v1, T v2 );
		template<typename T>
		void set( unsigned int index, T v0, T v1, T v2, T v3 );


		void clear()
		{
			m_data.clear();
			m_numElements = 0;
			m_isDirty = true;
		}

		void resize( int numElements )
		{
			m_data.resize( numElements*numComponents()*elementComponentSize() );
			m_numElements = numElements;
			m_isDirty = true;
		}


		int numElements()
		{
			return m_numElements;
		}

		int numComponents()
		{
			return m_numComponents;
		}

		ComponentType elementComponentType();

		int elementComponentSize()
		{
			return m_componentSize;
		}

		void *getRawPointer()
		{
			return (void *)&m_data[0];
		}
		void *getRawPointer( int index )
		{
			// should change on e per type basis
			return (void *)&m_data[index*numComponents()*elementComponentSize()];
		}





		std::vector<unsigned char> m_data;
		char              m_componentSize; // size in memory of a component of an element in byte
		ComponentType     m_componentType;
		char              m_numComponents; // number of components per element
		int                 m_numElements;

		//
		// static creators
		//
		static Attribute::Ptr create(char numComponents, ComponentType componentType, unsigned char *raw, int numElements );
		static Attribute::Ptr createM33f();
		static Attribute::Ptr createM44f();
		static Attribute::Ptr createV4f( int numElements = 0 );
		static Attribute::Ptr createV3f( int numElements = 0 );
		static Attribute::Ptr createV2f( int numElements = 0 );
		static Attribute::Ptr createFloat(int numElements = 0);
		static Attribute::Ptr createInt(int numElements = 0);

		//
		// static utilities
		//
		static int             componentSize( ComponentType ct );
		static ComponentType   componentType(const std::string& ct );

		bool                   m_isDirty; // indicates update on gpu required
	};


	template<typename T>
	unsigned int Attribute::appendElement( const T &value )
	{
		unsigned int pos = (unsigned int) m_data.size();
		m_data.resize( pos + sizeof(T) );
		*((T *)&m_data[pos]) = value;
		m_isDirty = true;
		return m_numElements++;
	}

	template<typename T>
	unsigned int Attribute::appendElement( const T &v0, const T &v1 )
	{
		unsigned int pos = (unsigned int) m_data.size();
		m_data.resize( pos + sizeof(T)*2 );
		T *data = (T*)&m_data[pos];
		*data = v0;++data;
		*data = v1;++data;
		m_isDirty = true;
		return m_numElements++;
	}

	template<typename T>
	unsigned int Attribute::appendElement( const T &v0, const T &v1, const T &v2 )
	{
		unsigned int pos = (unsigned int) m_data.size();
		m_data.resize( pos + sizeof(T)*3 );
		T *data = (T*)&m_data[pos];
		*data = v0;++data;
		*data = v1;++data;
		*data = v2;++data;
		m_isDirty = true;
		return m_numElements++;
	}

	template<typename T>
	unsigned int Attribute::appendElement( const T &v0, const T &v1, const T &v2, const T &v3 )
	{
		unsigned int pos = (unsigned int) m_data.size();
		m_data.resize( pos + sizeof(T)*4 );
		T *data = (T*)&m_data[pos];
		*data = v0;++data;
		*data = v1;++data;
		*data = v2;++data;
		*data = v3;++data;
		m_isDirty = true;
		return m_numElements++;
	}

	template<typename T>
	T &Attribute::get( unsigned int index )
	{
		T *data = (T*)&m_data[index * sizeof(T)];
		return *data;
	}

	template<typename T>
	void Attribute::set( unsigned int index, T value )
	{
		T *data = (T*)&m_data[index * sizeof(T)];
		*data = value;
		m_isDirty = true;
	}

	template<typename T>
	void Attribute::set( unsigned int index, T v0, T v1 )
	{
		T *data = (T*)&m_data[index * sizeof(T) * 2];
		*data++ = v0;
		*data++ = v1;
		m_isDirty = true;
	}

	template<typename T>
	void Attribute::set( unsigned int index, T v0, T v1, T v2 )
	{
		T *data = (T*)&m_data[index * sizeof(T) * 3];
		*data++ = v0;
		*data++ = v1;
		*data++ = v2;
		m_isDirty = true;
	}

	template<typename T>
	void Attribute::set( unsigned int index, T v0, T v1, T v2, T v3 )
	{
		T *data = (T*)&m_data[index * sizeof(T) * 4];
		*data++ = v0;
		*data++ = v1;
		*data++ = v2;
		*data++ = v3;
		m_isDirty = true;
	}

} // namespace houio
