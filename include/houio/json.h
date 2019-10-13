//
// lightweight json parser and writer. special about this one is that it supports binary json as
// specified by sideeffects' bgeo format (houdini12+).
// 
// todo: - transparent recasting of variants (e.g. value is int but queried as float...)
//       - support for uniform arrays
//       - support for ascii (read and write)
//
// bugs: Object::get with std::string is dodgy!
//	the following code crashes:
//		base::json::ObjectPtr o = base::json::Object::create();
//		std::string testShape = "asdasd";
//		o->append( "shape", base::json::Value::create( testShape ) );
//		int test = 0;
//		if( o->hasKey( "settings" ) )
//		{
//			test = o->get<int>("width");
//		}
//		std::string t = o->get<std::string>("shape");
//
#pragma once

#include <iostream>
#include <sstream>
#include <stack>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>

#include <houio/types.h>
#include <ttl/var/variant.hpp>




namespace houio
{
	namespace json
	{
		struct Parser;

		// note: currently all different int types (sint32, sint64 etc. will be routed to jsonInt32)
		struct Handler
		{
			virtual void                               jsonBeginArray() = 0;
			virtual void                                 jsonEndArray() = 0;
			virtual void                                 jsonBeginMap() = 0;
			virtual void                                   jsonEndMap() = 0;
			virtual void         jsonString( const std::string &value ) = 0;
			virtual void              jsonKey( const std::string &key ) = 0;
			virtual void                  jsonBool( const bool &value ) = 0;
			virtual void               jsonInt32( const sint32 &value ) = 0;
			virtual void              jsonReal32( const real32 &value ) = 0;
			virtual void   uaBool( sint64 numElements, Parser *parser ) = 0;
			virtual void uaReal32( sint64 numElements, Parser *parser ) = 0;
			virtual void uaReal64( sint64 numElements, Parser *parser ) = 0;
			virtual void  uaInt16( sint64 numElements, Parser *parser ) = 0;
			virtual void  uaInt32( sint64 numElements, Parser *parser ) = 0;
			virtual void  uaInt64( sint64 numElements, Parser *parser ) = 0;
			virtual void  uaUInt8( sint64 numElements, Parser *parser ) = 0;
			virtual void uaString( sint64 numElements, Parser *parser ) = 0;
		};


		struct Token
		{
			enum Type
			{
				// JSON identifiers used in binary encoding and parsing
				JID_NULL                = 0x00,
				JID_MAP_BEGIN           = 0x7b, // '{'
				JID_MAP_END             = 0x7d, // '}'
				JID_ARRAY_BEGIN         = 0x5b, // '['
				JID_ARRAY_END           = 0x5d, // ']'
				JID_BOOL                = 0x10,
				JID_INT8                = 0x11,
				JID_INT16               = 0x12,
				JID_INT32               = 0x13,
				JID_INT64               = 0x14,
				JID_REAL16              = 0x18,
				JID_REAL32              = 0x19,
				JID_REAL64              = 0x1a,
				JID_UINT8               = 0x21,
				JID_UINT16              = 0x22,
				JID_STRING              = 0x27,
				JID_FALSE               = 0x30,
				JID_TRUE                = 0x31,
				JID_TOKENDEF            = 0x2b, // triggers on-the-fly string definition
				JID_TOKENREF            = 0x26, // references a previous defined string
				JID_TOKENUNDEF          = 0x2d,
				JID_UNIFORM_ARRAY       = 0x40,
				JID_KEY_SEPARATOR       = 0x3a,
				JID_VALUE_SEPARATOR     = 0x2c,
				JID_MAGIC               = 0x7f
			};

			typedef ttl::var::variant<bool,           // bool
									  sbyte,          // int8
									  sword,          // int16
									  sint32,         // int32
									  sint64,         // int64
									  // TODO: real16
									  real32,         // real32
									  real64,         // real64
									  ubyte,          // uint8
									  uword,          // uint16
									  // no uint32?
									  std::string    // string
									  > Value;
			Token();
			void event( Parser *p, int key = false );


			Type                                type; // also encodes value type
			Value                              value;
			Type                              uaType; // additional type 
		};


		// UTILITY FUNCTIONS ======================================

		template<typename T>
		T fromString(const std::string& s)
		{
			std::istringstream stream (s);
			T t;
			try
			{
				stream >> t;
			}catch(...)
			{
				std::cout << "error during string conversion..." << std::endl;
			}
			return t;
		}

		template<class T>
		inline std::string toString(const T& t)
		{
			std::ostringstream stream;
			stream << t;
			return stream.str();
		}
		template<class T>
		inline std::string toString(const T& t, int precision)
		{
			std::ostringstream stream;
			stream.precision( precision );
			stream << t;
			return stream.str();
		}

		// Parser ==================================================
		struct Parser
		{
			enum State
			{
				// Parsing states
				STATE_INVALID           = -1,
				STATE_START             = 0,
				STATE_COMPLETE          = 1,
				STATE_MAP_START         = 2,
				STATE_MAP_SEPERATOR     = 3,
				STATE_MAP_NEED_VALUE    = 4,
				STATE_MAP_GOT_VALUE     = 5,
				STATE_MAP_NEED_KEY      = 6,
				STATE_ARRAY_START       = 7,
				STATE_ARRAY_NEED_VALUE  = 8,
				STATE_ARRAY_GOT_VALUE   = 9
			};

			bool parse( std::istream *in,  Handler *h );
			bool                          parseStream();
			bool                  readToken( Token &t );
			bool            readBinaryToken( Token &t, ubyte test = -1 );
			bool     readASCIIToken( Token &t, char c );
			bool           readBinaryStringDefinition();
			bool                       undefineString();
			void                   pushState( State s );
			void                    setState( State s );
			void                             popState();


			template<typename T>
			T                                    read();
			template<typename T>
			void     read( T *dst, sint64 numElements );
			sint64                         readLength();
			std::string              readBinaryString();
			std::string               readASCIIString();


			// 
			State                                 state;
			std::stack<State>                stateStack;
			Handler                            *handler;
			std::istream                        *stream;
			bool                                 binary;

			std::map<sint64, std::string>       strings; // common strings are references by ids
		};



		template<typename T>
		T Parser::read()
		{
			T v;
			stream->read( (char *)&v, sizeof(T) );
			return v;
		}
		template<typename T>
		void Parser::read( T *dst, sint64 numElements )
		{
			stream->read( (char *)dst, numElements*sizeof(T) );
		}


		// Writer ==================================================
		struct Writer
		{
			virtual                                   ~Writer(){};
			virtual void                       jsonBeginArray() = 0;
			virtual void                         jsonEndArray() = 0;
			virtual void                         jsonBeginMap() = 0;
			virtual void                           jsonEndMap() = 0;
			virtual void jsonString( const std::string &value ) = 0;
			virtual void      jsonKey( const std::string &key ) = 0;
			virtual void          jsonBool( const bool &value ) = 0;
			virtual void         jsonInt( const sint64 &value ) = 0; // specific type written will depend on range in which value lies
			virtual void        jsonUInt8( const ubyte &value ) = 0; // specific type written will depend on range in which value lies
			virtual void         jsonInt8( const sbyte &value ) = 0; // specific type written will depend on range in which value lies
			virtual void       jsonInt32( const sint32 &value ) = 0; // specific type written will depend on range in which value lies
			virtual void       jsonInt64( const sint64 &value ) = 0; // specific type written will depend on range in which value lies
			virtual void      jsonReal32( const real32 &value ) = 0;
			virtual void      jsonReal64( const real64 &value ) = 0;
		};

		struct BinaryWriter : public Writer
		{
			BinaryWriter( std::ostream *out );

			bool jsonMagic();
			virtual void                               jsonBeginArray()override;
			virtual void                                 jsonEndArray()override;
			virtual void                                 jsonBeginMap()override;
			virtual void                                   jsonEndMap()override;
			virtual void         jsonString( const std::string &value )override;
			virtual void              jsonKey( const std::string &key )override;
			virtual void                  jsonBool( const bool &value )override;
			virtual void                 jsonInt( const sint64 &value )override;
			virtual void                jsonUInt8( const ubyte &value )override;
			virtual void                 jsonInt8( const sbyte &value )override;
			virtual void               jsonInt32( const sint32 &value )override;
			virtual void               jsonInt64( const sint64 &value )override;

			virtual void              jsonReal32( const real32 &value )override;
			virtual void              jsonReal64( const real64 &value )override;


			template<typename T>
			bool                 jsonUniformArray( const std::vector<T> &data );
			template<typename T>
			bool          jsonUniformArray( const T *data, sint64 numElements );

			bool                                      writeId( Token::Type id );
			bool                            writeLength( const sint64 &length );


			template<typename T>
			bool                                            write( const T &v );
			template<typename T>
			bool                      write( const T *dst, sint64 numElements );



			// 
			std::ostream                                                *stream;
		};

		template<typename T>
		bool BinaryWriter::write( const T &v )
		{
			stream->write( (const char *)&v, sizeof(T) );
			return true;
		}
		template<typename T>
		bool BinaryWriter::write( const T *dst, sint64 numElements )
		{
			stream->write( (const char *)dst, numElements*sizeof(T) );
			return true;
		}

		template<typename T>
		bool BinaryWriter::jsonUniformArray( const std::vector<T> &data )
		{
			Token::Type type = Token::JID_NULL;
			if( typeid(T) == typeid(real32) )
				type = Token::JID_REAL32;
			else if( typeid(T) == typeid(real64) )
				type = Token::JID_REAL64;
			else if( typeid(T) == typeid(bool) )
				type = Token::JID_BOOL;
			else if( typeid(T) == typeid(sbyte) )
				type = Token::JID_INT8;
			else if( typeid(T) == typeid(sword) )
				type = Token::JID_INT16;
			else if( typeid(T) == typeid(sint32) )
				type = Token::JID_INT32;
			else if( typeid(T) == typeid(sint64) )
				type = Token::JID_INT64;
			else if( typeid(T) == typeid(ubyte) )
				type = Token::JID_UINT8;
			else if( typeid(T) == typeid(uword) )
				type = Token::JID_UINT16;
			else
				throw std::runtime_error("BinaryWriter::jsonUniformArray: unable to handle type");
			
			writeId( Token::JID_UNIFORM_ARRAY );
			write<sbyte>( (sbyte)type );
			writeLength( data.size() );
			if( !data.empty() )
				write<T>( &data[0], data.size() );

			return true;
		}
		template<typename T>
		bool BinaryWriter::jsonUniformArray( const T *data, sint64 numElements )
		{
			Token::Type type = Token::JID_NULL;
			if( typeid(T) == typeid(real32) )
				type = Token::JID_REAL32;
			else if( typeid(T) == typeid(real64) )
				type = Token::JID_REAL64;
			else if( typeid(T) == typeid(bool) )
				type = Token::JID_BOOL;
			else if( typeid(T) == typeid(sbyte) )
				type = Token::JID_INT8;
			else if( typeid(T) == typeid(sword) )
				type = Token::JID_INT16;
			else if( typeid(T) == typeid(sint32) )
				type = Token::JID_INT32;
			else if( typeid(T) == typeid(sint64) )
				type = Token::JID_INT64;
			else if( typeid(T) == typeid(ubyte) )
				type = Token::JID_UINT8;
			else if( typeid(T) == typeid(uword) )
				type = Token::JID_UINT16;
			else
				throw std::runtime_error("BinaryWriter::jsonUniformArray: unable to handle type");
			
			writeId( Token::JID_UNIFORM_ARRAY );
			write<sbyte>( (sbyte)type );
			writeLength( numElements );
			write<T>( data, numElements );

			return true;
		}


		// ASCIIWriter ==================================================
		struct ASCIIWriter : public Writer
		{
			ASCIIWriter( std::ostream *out );

			virtual void                               jsonBeginArray()override;
			virtual void                                 jsonEndArray()override;
			virtual void                                 jsonBeginMap()override;
			virtual void                                   jsonEndMap()override;
			virtual void         jsonString( const std::string &value )override;
			virtual void              jsonKey( const std::string &key )override;
			virtual void                  jsonBool( const bool &value )override;
			virtual void                 jsonInt( const sint64 &value )override;
			virtual void                jsonUInt8( const ubyte &value )override;
			virtual void                 jsonInt8( const sbyte &value )override;
			virtual void               jsonInt32( const sint32 &value )override;
			virtual void               jsonInt64( const sint64 &value )override;

			virtual void              jsonReal32( const real32 &value )override;
			virtual void              jsonReal64( const real64 &value )override;


			void                               write( const std::string &text );

			void                                                 writeNewline();
			void                                                  writePrefix();

			int                                                     indentLevel;
			bool                                                      firstItem;
			bool                                                         gotKey;
			std::stack<Token::Type>                                       stack;
			std::string                                                  prefix;
			// 
			std::ostream                                                *stream;
		};



		// JSONLogger ==================================================
		struct JSONLogger : public Handler
		{
			JSONLogger() : out(std::cout), indentLevel(0)
			{
			}
			JSONLogger( std::ostream &outputStream ) : out(outputStream), indentLevel(0)
			{
			}

			void indent()
			{
				for( int i=0;i<indentLevel;++i )
					out << "\t";std::flush(out);
			}

			virtual void jsonBeginArray()
			{
				indent();
				out << "jsonBeginArray\n";std::flush(out);
				++indentLevel;
			}

			virtual void jsonEndArray()
			{
				--indentLevel;
				indent();
				out << "jsonEndArray\n";std::flush(out);
			}

			virtual void jsonString( const std::string &value )
			{
				indent();
				out << "jsonString " << value << "\n";std::flush(out);
			}

			virtual void jsonKey( const std::string &key )
			{
				indent();
				out << "jsonKey " << key << "\n";std::flush(out);
			}

			virtual void jsonBool( const bool &value )
			{
				indent();
				out << "jsonBool " << value << "\n";std::flush(out);
			}

			virtual void jsonInt32( const sint32 &value )
			{
				indent();
				out << "jsonInt32 " << value << "\n";std::flush(out);
			}

			virtual void jsonReal32( const real32 &value )
			{
				indent();
				out << "jsonReal32 " << value << "\n";std::flush(out);
			}

			virtual void jsonBeginMap()
			{
				indent();
				out << "jsonBeginMap\n";std::flush(out);
				++indentLevel;
			}

			virtual void jsonEndMap()
			{
				--indentLevel;
				indent();
				out << "jsonEndMap\n";std::flush(out);
			}

			virtual void uaBool( sint64 numElements, Parser *parser )
			{
				indent();
				//In binary JSON files, uniform bool arrays are stored as bit
				//streams.  This method decodes the bit-stream, calling jsonBool()
				//for each element of the bit array.
				std::vector<bool> data(numElements);
				if( numElements != 0 )
				{
					int count = (int) numElements;
					while( count > 0 )
					{
						uint32 bits;
						parser->read<uint32>( &bits, 1 );
						int nbits = std::min( count, 32 );
						count -= nbits;
						for( int i=0;i<nbits;++i )
						{
							bool b = (bits & (1 << i)) != 0;
							data[i] = b;
						}
					}
				}
				out << "jsonArray [";std::flush(out);
				for( std::vector<bool>::iterator it = data.begin(); it != data.end();++it )
					out << (int)(*it) << " ";std::flush(out);
				out << "]\n";std::flush(out);
			}

			virtual void uaReal32( sint64 numElements, Parser *parser )
			{
				ua<real32>( numElements, parser, "<real32>" );
			}

			virtual void uaReal64( sint64 numElements, Parser *parser )
			{
				ua<real64>( numElements, parser, "<real64>" );
			}

			virtual void uaInt16( sint64 numElements, Parser *parser )
			{
				ua<sword>( numElements, parser, "<int16>" );
			}

			virtual void uaInt32( sint64 numElements, Parser *parser )
			{
				ua<sint32>( numElements, parser, "<int32>" );
			}

			virtual void uaInt64( sint64 numElements, Parser *parser )
			{
				ua<sint64>( numElements, parser, "<sint64>" );
			}

			virtual void uaUInt8( sint64 numElements, Parser *parser )
			{
				indent();
				std::vector<ubyte> data(numElements);
				if( numElements != 0 )
					parser->read<ubyte>( &data[0], numElements );
				out << "jsonArray<uint8> [";std::flush(out);
				for( std::vector<ubyte>::iterator it = data.begin(); it != data.end();++it )
					out << (int)(*it) << " ";std::flush(out);
				out << "]\n";std::flush(out);
			}

			virtual void uaString( sint64 numElements, Parser *parser )
			{
				indent();
				std::vector<std::string> data;
				data.reserve(numElements);
				for(sint64 i=0;i<numElements;++i)
					data.push_back( parser->readBinaryString() );
				out << "jsonArray<string> [";std::flush(out);
				for( std::vector<std::string>::iterator it = data.begin(); it != data.end();++it )
					out << *it << " ";std::flush(out);
				out << "]\n";std::flush(out);
			}

			template<typename T>
			void ua( sint64 numElements, Parser *parser, std::string type = "" )
			{
				indent();
				std::vector<T> data(numElements);
				if( numElements != 0 )
					parser->read<T>( (T*)&data[0], numElements );
				out << "jsonArray"<<type<<" ("<< numElements << ") [";std::flush(out);
				for( typename std::vector<T>::iterator it = data.begin(); it != data.end();++it )
					out << *it << " ";std::flush(out);
				out << "]---\n";std::flush(out);
			}

			std::ostream &out;
			int indentLevel;
		};

		// JSONCPP ==================================================

		// Value -------------

		struct Array;
		typedef std::shared_ptr<Array> ArrayPtr;
		struct Object;
		typedef std::shared_ptr<Object> ObjectPtr;

		struct Value
		{

			typedef ttl::var::variant<bool,           // order
									  sint32,         // must
									  real32,         // not
									  real64,         // change
									  std::string,    // !!!!!! - because index is used in is* methods
									  ubyte,          // also: if you add something here you need to update Value::cpyTo, JSONWriter::operators
									  sint64
									  > Variant;

			Value();

			bool                           isNull()const;
			bool                          isArray()const;
			bool                         isObject()const;
			bool                         isString()const;

			template<typename T>
			const T                           as() const;

			void                 cpyTo( char *dst )const;

			ArrayPtr                           asArray();
			ObjectPtr                         asObject();

			Variant                        &getVariant();

			static Value                   createArray();
			static Value     createArray(ArrayPtr array);
			static Value                  createObject();
			static Value   createObject( ObjectPtr obj );

			template<typename T>
			static Value        create( const T &value );


		private:
			enum Type
			{
				TYPE_NULL,
				TYPE_VALUE,
				TYPE_OBJECT,
				TYPE_ARRAY
			};
			Type                               m_type; // value, array or object?
			Variant                           m_value;
			ObjectPtr                        m_object;
			ArrayPtr                          m_array;

			friend                             Object;
			friend                             Array;
		};


		// VariantConverter ================================

		// In order to conveniently extract data from variants we need to be able to convert between 
		// all the different variant types. This is done by using the visitor pattern in the as function of
		// the varient class. VariantConverter is the visitor which will its ()operator get called for all the different
		// variant types. The Converter is a templated class which has the destination type as its template argument
		// D. The operator is templated as well with the argument type (the type of the varient) as template argument.
		// By default a simple cast is done. Now to handle special cases where such a cast fails we do the following steps:
		// 1. create a specialized class template which has the destination type as template argument D
		// 2. overload the () operator with the variant type which produces the conflict and resolve it in the
		// definition of that overloaded function

		template<typename D>
		struct VariantConverter
		{
			D &dest;
			VariantConverter( D &_dest ) : dest(_dest){}
			template< typename T >
			void operator()( T d )
			{
				dest = (D)d;
			}
		};

		template<>
		struct VariantConverter<bool>
		{
			typedef bool t_dest;
			t_dest &dest;
			VariantConverter( t_dest &_dest ) : dest(_dest){}

			void operator()(std::string /*x*/)
			{
				dest = false;
			}

			template< typename T >
			void operator()( T d )
			{
				dest = (t_dest)d;
			}
		};

		template<>
		struct VariantConverter<float>
		{
			typedef float t_dest;
			t_dest &dest;
			VariantConverter( t_dest &_dest ) : dest(_dest){}

			void operator()(std::string /*x*/)
			{
				//dest = false;
			}

			template< typename T >
			void operator()( T d )
			{
				dest = (t_dest)d;
			}
		};

		template<>
		struct VariantConverter<int>
		{
			typedef int t_dest;
			t_dest &dest;
			VariantConverter( t_dest &_dest ) : dest(_dest){}

			void operator()(std::string /*x*/)
			{
				//dest = false;
			}

			template< typename T >
			void operator()( T d )
			{
				dest = (t_dest)d;
			}
		};

		template<>
		struct VariantConverter<ubyte>
		{
			typedef ubyte t_dest;
			t_dest &dest;
			VariantConverter( t_dest &_dest ) : dest(_dest){}

			void operator()(std::string /*x*/)
			{
				//dest = false;
			}

			template< typename T >
			void operator()( T d )
			{
				dest = (t_dest)d;
			}
		};

		template<>
		struct VariantConverter<std::string>
		{
			typedef std::string t_dest;
			t_dest &dest;
			VariantConverter( t_dest &_dest ) : dest(_dest){}

			void operator()(std::string x)
			{
				dest = x;
			}

			template< typename T >
			void operator()( T d )
			{
				std::ostringstream stream;
				stream << d;
				dest = stream.str();
			}
		};


		template<typename T>
		const T Value::as()const
		{
			T dest = T();
			VariantConverter<T> conv(dest);
			ttl::var::apply_visitor(conv, m_value);
			return dest;
		}


		template<typename T>
		Value Value::create( const T &value )
		{
			Value v;
			v.m_type = TYPE_VALUE;
			v.m_value = value;
			return v;
		}


		// Array -------------
		struct Array
		{
			Array();
			~Array();

			static ArrayPtr               create();

			template<typename T>
			const T                  get( const int index );
			ObjectPtr                getObject( int index );
			ArrayPtr                  getArray( int index );

			Value               getValue( const int index );

			sint64                              size()const;
			bool                           isUniform()const;



			template<typename T>
			void                     appendValue( T value );
			//void                     append( Value &value );
			void                     append( const Value &value );
			void                     append(ObjectPtr &object );
			void                     append(ArrayPtr &array );

		//private:
			std::vector<Value>                     m_values;
			bool                                m_isUniform;
			unsigned char*                    m_uniformdata;
			sint64                     m_numUniformElements;
			int                               m_uniformType; // integer which equals Variant::which()
		};


		template<typename T>
		const T Array::get( const int index )
		{
			return getValue(index).as<T>();
		}

		template<typename T>
		void Array::appendValue( T value )
		{
			append(Value::create<T>(value));
		}

		// Object -------------
		struct Object
		{
			static ObjectPtr                                           create();
			bool                               hasKey( const std::string &key );

			template<typename T>
			T                        get( const std::string &key, T def = T() );
			ObjectPtr                       getObject( const std::string &key );
			ArrayPtr                         getArray( const std::string &key );


			Value                            getValue( const std::string &key );
			void                      getKeys( std::vector<std::string> &keys );
			sint64                                                  size()const;

			template<typename T>
			void appendValue( const std::string &key, const T& value );
			void                 append( const std::string &key, const Value &value );
			void             append( const std::string &key, ObjectPtr object );
			void             append( const std::string &key, ArrayPtr array );
		//private:
			std::map<std::string, Value>                               m_values;

		};


		template<typename T>
		T Object::get( const std::string &key, T def )
		{
			T result = def;
			std::map<std::string, Value>::iterator it = m_values.find( key );
			if( it != m_values.end())
				result = it->second.as<T>();
			return result;
		}

		template<typename T>
		void Object::appendValue( const std::string &key, const T& value )
		{
			append(key, Value::create<T>(value));
		}

		// JSONReader ========================================================
		// this will read json into cpp json structures (Object,Array,Value)
		struct JSONReader : public Handler
		{
			JSONReader();

			Value                                                getRoot();

			virtual void                                  jsonBeginArray();
			virtual void                                    jsonEndArray();
			virtual void                                    jsonBeginMap();
			virtual void                                      jsonEndMap();
			virtual void            jsonString( const std::string &value );
			virtual void                 jsonKey( const std::string &key );
			virtual void                     jsonBool( const bool &value );
			virtual void                  jsonInt32( const sint32 &value );
			virtual void                 jsonReal32( const real32 &value );
			virtual void      uaBool( sint64 numElements, Parser *parser );
			virtual void    uaReal32( sint64 numElements, Parser *parser );
			virtual void    uaReal64( sint64 numElements, Parser *parser );
			virtual void     uaInt16( sint64 numElements, Parser *parser );
			virtual void     uaInt32( sint64 numElements, Parser *parser );
			virtual void     uaInt64( sint64 numElements, Parser *parser );
			virtual void     uaUInt8( sint64 numElements, Parser *parser );
			virtual void    uaString( sint64 numElements, Parser *parser );


		private:
			typedef std::pair<Value, std::string> StackItem; // holds value and nextKey

			template<typename T>
			void                               jsonValue( const T &value );
			template<typename T, typename S>
			void              jsonUA( sint64 numElements, Parser *parser );

			void                                                    push();
			void                                                     pop();
			Value                                                   m_root;
			std::stack<StackItem>                                  m_stack; // used during sax parsing
			std::string                                            nextKey; // used during sax parsing
		};

		template<typename T>
		void JSONReader::jsonValue( const T &value )
		{
			if( m_root.isArray() )
				m_root.asArray()->append(Value::create<T>(value));
			else
			if( m_root.isObject() )
			{
				m_root.asObject()->append(nextKey, Value::create<T>(value));
				nextKey = "JSONReader::jsonValue:invalid key";
			}else
			{
				throw std::runtime_error("JSONReader::jsonValue: unknown container");
			}
		}

		template<typename T, typename S>
		void JSONReader::jsonUA( sint64 numElements, Parser *parser )
		{
			typedef ttl::meta::find_equivalent_type<const T&, Value::Variant::list> found;


			Value v = Value::createArray();
			ArrayPtr ua = v.asArray();
			ua->m_isUniform = true;
			ua->m_uniformdata = (unsigned char *)malloc( numElements*sizeof(T) );
			ua->m_numUniformElements = numElements;
			ua->m_uniformType = found::index;

			S *data_src = (S *)malloc( numElements*sizeof(S) );
			parser->read<S>( data_src, numElements );

			// convert types
			T *data_dest = (T *)ua->m_uniformdata;
			for( sint64 i=0;i<numElements;++i, ++data_dest )
				*data_dest = (T)data_src[i];

			if( m_root.isArray() )
				m_root.asArray()->append(v);
			else
			if( m_root.isObject() )
				m_root.asObject()->append(nextKey, v);

			free(data_src);
		}


		// JSONWriter =========================================
		struct JSONWriter
		{
			JSONWriter( std::ostream *out, bool binary = false )
			{
				if(binary)
					m_writer = new BinaryWriter(out);
				else
					m_writer = new ASCIIWriter(out);
			}

			~JSONWriter()
			{
				delete m_writer;
			}

			bool                    write( ObjectPtr object );
			bool                        write( ArrayPtr arr );
			bool                        write( Value &value );

			//used in combination with visitor pattern
			void operator()( bool value ){m_writer->jsonBool(value);}
			void operator()( ubyte value ){m_writer->jsonUInt8(value);}
			void operator()( sbyte value ){m_writer->jsonInt8(value);}
			void operator()( sint32 value ){m_writer->jsonInt32(value);}
			void operator()( sint64 value ){m_writer->jsonInt64(value);}
			void operator()( real32 value ){m_writer->jsonReal32(value);}
			void operator()( real64 value ){m_writer->jsonReal64(value);}
			void operator()( const std::string &value ){m_writer->jsonString(value);}
		private:
			Writer                                  *m_writer;
		};

	}
}

