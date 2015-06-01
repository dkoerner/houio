

#include <houio/json.h>
#include <algorithm>







namespace houio
{
	namespace json
	{
		// Token ==================================================
		Token::Token() : type(JID_NULL), value(0), uaType(JID_NULL)
		{
		}

		void Token::event( Parser *p, int key )
		{
			if (type == JID_ARRAY_BEGIN)
				p->handler->jsonBeginArray();
			else if (type == JID_ARRAY_END)
				p->handler->jsonEndArray();
			if (type == JID_MAP_BEGIN)
				p->handler->jsonBeginMap();
			else if (type == JID_MAP_END)
				p->handler->jsonEndMap();
			else if (type == JID_STRING)
			{
				if( key )
					p->handler->jsonKey( ttl::var::get<std::string>( value ) );
				else
					p->handler->jsonString( ttl::var::get<std::string>( value ) );
			}
			else if (type == JID_BOOL)
				p->handler->jsonBool( ttl::var::get<bool>( value ) );
			else if (type == JID_INT8)
				p->handler->jsonInt32( ttl::var::get<sbyte>( value ) );
			else if (type == JID_INT16)
				p->handler->jsonInt32( ttl::var::get<sword>( value ) );
			else if (type == JID_INT32)
				p->handler->jsonInt32( ttl::var::get<sint32>( value ) );
			else if (type == JID_INT64)
				p->handler->jsonInt32( (int)ttl::var::get<sint64>( value ) );
			else if (type == JID_REAL32)
				p->handler->jsonReal32( ttl::var::get<real32>( value ) );
			else if (type == JID_REAL64)
				p->handler->jsonReal32( (real32)ttl::var::get<real64>( value ) );
			else if (type == JID_UINT8)
				p->handler->jsonInt32( ttl::var::get<ubyte>( value ) );
			else if (type == JID_UINT16)
				p->handler->jsonInt32( ttl::var::get<uword>( value ) );
			else if (type == JID_UNIFORM_ARRAY)
			{
				sint64 numElements = ttl::var::get<sint64>( value );
				switch( uaType )
				{
				case Token::JID_BOOL:p->handler->uaBool( numElements, p );break;
				case Token::JID_INT16:p->handler->uaInt16( numElements, p );break;
				case Token::JID_INT32:p->handler->uaInt32( numElements, p );break;
				case Token::JID_INT64:p->handler->uaInt64( numElements, p );break;
				case Token::JID_REAL32:p->handler->uaReal32( numElements, p );break;
				case Token::JID_REAL64:p->handler->uaReal64( numElements, p );break;
				case Token::JID_UINT8:p->handler->uaUInt8( numElements, p );break;
				case Token::JID_STRING:p->handler->uaString( numElements, p );break;
				case Token::JID_NULL:
				case Token::JID_MAP_BEGIN:
				case Token::JID_MAP_END:
				case Token::JID_ARRAY_BEGIN:
				case Token::JID_ARRAY_END:
				case Token::JID_FALSE:
				case Token::JID_TRUE:
				case Token::JID_TOKENDEF:
				case Token::JID_TOKENREF:
				case Token::JID_TOKENUNDEF:
				case Token::JID_UNIFORM_ARRAY:
				case Token::JID_KEY_SEPARATOR:
				case Token::JID_VALUE_SEPARATOR:
				case Token::JID_MAGIC:
				case Token::JID_INT8:
				case Token::JID_REAL16:
				case Token::JID_UINT16:
				default:
					{
						throw std::runtime_error( "json.cpp Token::event: error unsupported uniform array type" );
						break;
					}
				};
			}




		}





		// Parser ==================================================

		bool Parser::parse( std::istream *in,  Handler *h )
		{
			if( !in->good() )
				return false;

			// make sure length is > 0

			// (re)initialize
			state = STATE_START;
			binary = false;
			handler = h;
			stream = in;

			if( !parseStream() )
			{
				std::cout << "error occured\n";
				return false;
			}
			return true;
		}

		bool Parser::parseStream()
		{
			Token t;

			while(state != STATE_COMPLETE)
			{
				bool popped = false;

				if( !readToken( t ) )
					return false;

				// expecting values ---------------------
				if( (state == STATE_START)||
					(state == STATE_ARRAY_START)||
					(state == STATE_ARRAY_NEED_VALUE)||
					(state == STATE_MAP_NEED_VALUE))
				{
					State newState = STATE_INVALID;

					if( t.type == Token::JID_ARRAY_BEGIN )
						newState = STATE_ARRAY_START;
					else if( t.type == Token::JID_MAP_BEGIN )
						newState = STATE_MAP_START;
					else if( (t.type == Token::JID_ARRAY_END)||(t.type == Token::JID_MAP_END) )
					{
						popState();
						popped = true;
					}

					if( newState != STATE_INVALID )
						pushState( newState );
					else if( !popped )
					{
						if( state == STATE_MAP_NEED_VALUE )
							setState( STATE_MAP_GOT_VALUE );
						else
							setState( STATE_ARRAY_GOT_VALUE );
					}

					// call event handler for current token
					t.event( this );
				}else
				// expecting keys ---------------------
				if( (state == STATE_MAP_START)||
					(state == STATE_MAP_NEED_KEY) )
				{
					// if we got a key
					if( t.type == Token::JID_STRING )
					{
						// we will expect a key value seperator next
						setState( STATE_MAP_SEPERATOR );
						t.event( this, 1 );
					}else
					if( t.type == Token::JID_MAP_END )
					{
						popState();
						t.event( this );
						if( stateStack.empty() )
							return true;
					}
				}else
				if( state == STATE_MAP_SEPERATOR )
				{
					if( t.type == Token::JID_KEY_SEPARATOR )
						setState( STATE_MAP_NEED_VALUE );
				}else
				if( state == STATE_MAP_GOT_VALUE )
				{
					if( t.type == Token::JID_MAP_END )
					{
						popState();
						t.event( this );
						if( stateStack.empty() )
							return true;
					}else
					if( t.type == Token::JID_VALUE_SEPARATOR )
						setState( STATE_MAP_NEED_KEY );
				}else
				if( state == STATE_ARRAY_GOT_VALUE )
				{
					if( t.type == Token::JID_ARRAY_END )
					{
						popState();
						t.event( this );
						if( stateStack.empty() )
							return true;
					}else
					if( t.type == Token::JID_VALUE_SEPARATOR )
						setState( STATE_ARRAY_NEED_VALUE );
				}

			}


			return true;
		}

		bool Parser::readToken( Token &t )
		{
			ubyte c = read<ubyte>();

			if(!stream->good())
				return false;
			
			// binary?
			if( (Token::Type)c == Token::JID_MAGIC )
			{
				// check
				uint32 magic = read<uint32>();
				if( magic == 0x624a534e )  // BINARY_MAGIC = 0x624a534e
					binary = true;
				else
					// todo: error message
					return false;
				return readToken(t);
			}else
				if( binary )
				{
					t.type = (Token::Type)c;
					return readBinaryToken( t, c );
				}
				else
					return readASCIIToken( t, c );

			return true;
		}

		bool Parser::readBinaryToken( Token &t, ubyte test )
		{
			while( (t.type == Token::JID_TOKENDEF) || (t.type == Token::JID_TOKENUNDEF) )
			{
				if(t.type == Token::JID_TOKENDEF)
					readBinaryStringDefinition();
				else
					undefineString();
				t.type = (Token::Type)read<ubyte>();
			};

			switch( t.type )
			{
			case Token::JID_ARRAY_BEGIN:
			case Token::JID_ARRAY_END:
			case Token::JID_MAP_BEGIN:
			case Token::JID_MAP_END:
				return true;
			case Token::JID_TOKENREF:
				{
					//Common strings may be encoded using a shared string table.  Each
					//string token defined (defineToken()) is given an integer "handle"
					//which can be used to reference the token.  Reating a token involves
					//reading the integer handle and looking up the value in the Tokens map.
					sint64 stringId = readLength();
					t.type = Token::JID_STRING;
					t.value = strings[stringId];
				}return true;
			case Token::JID_TRUE: t.value = true;t.type = Token::JID_BOOL; return true;
			case Token::JID_FALSE: t.value = false;t.type = Token::JID_BOOL; return true;
			case Token::JID_BOOL: t.value = read<sbyte>() != 0;t.type = Token::JID_BOOL; return true;
			case Token::JID_INT8: t.value = read<sbyte>();return true;
			case Token::JID_INT16: t.value = read<sword>();return true;
			case Token::JID_INT32: t.value = read<sint32>();return true;
			case Token::JID_INT64: t.value = read<sint64>();return true;
			case Token::JID_REAL32: t.value = read<real32>();return true;
			case Token::JID_REAL64: t.value = read<real64>();return true;
			case Token::JID_UINT8: t.value = read<ubyte>();return true;
			case Token::JID_UINT16: t.value = read<uword>();return true;
			case Token::JID_STRING: t.value = readBinaryString();return true;
			case Token::JID_UNIFORM_ARRAY:
				{
					// Read the type information which will be saved in seperate member
					t.uaType = (Token::Type) read<sbyte>();
					// value will be number of items
					t.value = readLength();
					return true;
				}
			default:
				throw std::runtime_error( "Parser::readToken - unknown id" );
			};


			return false;
		}

		bool Parser::readASCIIToken( Token &t, char c )
		{
			// Read an ASCII token (returns a _jValue or None)
			while( (c == ' ')||(c == '\r')||(c == '\n')||(c == '\t') )
			{
				c = read<char>();
			};

			// We support // style comments
			if( c == '/' )
			{
				c = read<char>();
				if( c == '/' )
				{
					while(true)
					{
						c = read<char>();
						if( (c == '\n')||(c == '\r') )
							return this->readToken( t );
					};
				}
			}

			if( c == '{' ) t.type = Token::JID_MAP_BEGIN;
			else
			if( c == '}' ) t.type = Token::JID_MAP_END;
			else
			if( c == '[' ) t.type = Token::JID_ARRAY_BEGIN;
			else
			if( c == ']' ) t.type = Token::JID_ARRAY_END;
			else
			if( c == ',' ) t.type = Token::JID_VALUE_SEPARATOR;
			else
			if( c == ':' ) t.type = Token::JID_KEY_SEPARATOR;
			else
			if( c == '"' )
			{
				std::string s = readASCIIString();
				t.type = Token::JID_STRING;
				t.value = s;
			}
			else
			{
				// read number -----------------

				//Catch-all parsing method which handles
				//	- null
				//	- true
				//	- false
				//	- numbers
				//Any unquoted string which doesn't match the null, true, false
				//tokens is considered to be a number.  We throw warnings on
				//non-number values, but set the value to 0.  This is not ideal since
				//a NAN or infinity is not preserved.

				std::string string = "";
				bool isReal = false;

				while(true)
				{
					string.append( &c, 1 );
					c = read<char>();
					if( (c == ' ')||(c == '\r')||(c == '\n')||(c == '\t')||(c == '/')||(c == '{')||(c == '}')||(c == '[')||(c == ']')||(c == ',')||(c == ':')||(c == '"') )
					{
						stream->unget();
						break;
					}else
					if( (c == '.')||(c == 'e')||(c == 'E') )
						isReal = true;
				}

				if( string.empty() )
					return true;

				std::transform(string.begin(), string.end(), string.begin(), tolower);

				if( string == "null" ) t.type = Token::JID_NULL;
				else
				if( string == "false" ){ t.type = Token::JID_BOOL; t.value = false;}
				else
				if( string == "true" ){ t.type = Token::JID_BOOL; t.value = true;}
				else
				if( isReal )
				{
					t.type = Token::JID_REAL32;
					t.value = fromString<float>( string );
				}else
				{
					t.type = Token::JID_INT64;
					t.value = fromString<sint64>( string );
				}
			}

			return true;
		}

		void Parser::pushState( State s )
		{
			// if we just started we dont need to track the current state
			//if( state != STATE_START )
			// otherwise we will need to remember
			stateStack.push( s );

			// set new state
			setState( s );
		}

		void Parser::popState()
		{
			if( stateStack.empty() )
				// error
				return;

			int n = (int)stateStack.size();
			stateStack.pop();

			if( n == 1 )
				state = STATE_COMPLETE;
			else
				state = stateStack.top();

						
			if( (state == STATE_MAP_NEED_VALUE)||(state == STATE_MAP_START))
				setState( STATE_MAP_GOT_VALUE );
			else if( (state == STATE_ARRAY_NEED_VALUE)||(state == STATE_ARRAY_START))
				setState( STATE_ARRAY_GOT_VALUE );

			// return true
		}

		void Parser::setState( State s )
		{
			State newState = s;

			if( this->binary )
			{
				// here we will do some built in direct state transitions
				if( newState == STATE_ARRAY_GOT_VALUE )
					newState = STATE_ARRAY_NEED_VALUE;
				else
				if( newState == STATE_MAP_SEPERATOR )
					newState = STATE_MAP_NEED_VALUE;
				else
				if( newState == STATE_MAP_GOT_VALUE )
					newState = STATE_MAP_NEED_KEY;
			}
			// set new state
			state = newState;
			// update stack
			stateStack.pop();
			stateStack.push(newState);
		}


		// Read an id followed by an encoded string.  There is no handle
		// callback, but rather, the string is stored in the shared string
		// Token map.
		bool Parser::readBinaryStringDefinition()
		{
			sint64 l = readLength();
			std::string s = readBinaryString();
			strings[l]  = s;
			return true;
		}

		bool Parser::undefineString()
		{
			throw std::runtime_error("undefineString not implemented");
			return false;
		}

		sint64 Parser::readLength()
		{
			//In the binary format, length is encoded in a multi-byte format.
			//For lenthgs < 0xf1 (240) the lenths are stored as a single byte.
			//If the length is longer than 240 bytes, the value of the first byte
			//determines the number of bytes that follow used to store the
			//length.  Currently, the only supported values for the binary byte
			//0xf2 = 2 bytes (16 bit unsigned length)
			//0xf4 = 4 bytes (32 bit unsigned length)
			//0xf8 = 8 bytes (64 bit signed length)
			//Other values (i.e. 0xf1 or 0xfa) are considered errors.
			ubyte n = read<ubyte>();
			switch( n )
			{
			case 0xf2:return read<uword>();
			case 0xf4:return read<uint32>();
			case 0xf8:return read<sint64>();
			default:
				if (n < 0xf1)
					return n;
				else
				{
					// kind of an error
					std::ostringstream stringStream;
					stringStream << "unknown length id " << n;
					throw std::runtime_error(stringStream.str());
				}
			};
			return 0;
		}

		std::string Parser::readBinaryString()
		{
			// A binary string is encoded by storing it's length (encoded)
			// followed by the string data.  The trailing <null> character is not
			// stored in the file.
			sint64 l = readLength();
			std::string s;
			if( l>0 )
			{
				s.resize( l );
				stream->read( (char *)&s[0], l );
			}

			if( s == "constantpageflags" )
			{
				int debug = 0;
				debug++;
			}

			return s;
		}

		// Read a quoted string one character at a time
		std::string Parser::readASCIIString()
		{
			std::string result = "";

			while(true)
			{
				char c = read<char>();
				if( c == '\\' )
				{
					c =  read<char>();
					if( (c=='"')||(c=='\\')||(c=='/') )result.append(&c, 1);
					else
					if( c=='b' ) result.push_back('\b');
					else
					if( c=='f' ) result.push_back('\f');
					else
					if( c=='n' ) result.push_back('\n');
					else
					if( c=='r' ) result.push_back('\r');
					else
					if( c=='t' ) result.push_back('\t');
					else
					if( c=='u' )
					{
						throw std::runtime_error( "error " );
					}else
					{
						result.push_back('\\');
						result.push_back(c);
					}
				}else
				if( c == '"' )
				{
					return result;
				}else
					result.append(&c, 1);

			};



		/*
        ''' Read a quoted string one character at a time '''
        word = []
        while True:
            c = self._readBytes(1)
            if c == None:
                self.error('Missing end-quote for string')
                return None
            if c == '\\':
                c = self._readBytes(1)
                if c in '"\\/': word.append(c)
                elif c == 'b':  word.append('\b')
                elif c == 'f':  word.append('\f')
                elif c == 'n':  word.append('\n')
                elif c == 'r':  word.append('\r')
                elif c == 't':  word.append('\t')
                elif c == 'u':
                    if self._readBytes(4) == None:
                        return False
                    self.error('UNICODE string escape not supported')
                else:
                    word.append('\\')
                    word.append(c)
            elif c == '"':
                return ''.join(word)
            else:
                word.append(c)
		*/










			// TODO:
			return "";
		}







		// Writer ==================================================


		BinaryWriter::BinaryWriter( std::ostream *out )
		{
			stream = out;
			jsonMagic();
		}

		bool BinaryWriter::writeId( Token::Type id )
		{
			return write<ubyte>( (ubyte)id );
		}

		bool BinaryWriter::writeLength( const sint64 &length )
		{
			if( length < 0xf1 )
				return write<ubyte>( (ubyte)length );
			else if( length < 0xffff )
			{
				write<ubyte>( 0xf2 );
				return write<uword>( (uword)length );
			}else if( length < 0xffffffff )
			{
				write<ubyte>( 0xf4 );
				return write<uint32>( (uint32)length );
			}else
			{
				write<ubyte>( 0xf8 );
				return write<sint64>( length );
			}
			return false;
		}

		bool BinaryWriter::jsonMagic()
		{
			return writeId( Token::JID_MAGIC ) && write<uint32>( 0x624a534e );// BINARY_MAGIC = 0x624a534e
		}

		void BinaryWriter::jsonBeginArray()
		{
			writeId( Token::JID_ARRAY_BEGIN);
		}

		void BinaryWriter::jsonEndArray()
		{
			writeId( Token::JID_ARRAY_END);
		}

		void BinaryWriter::jsonBeginMap()
		{
			writeId( Token::JID_MAP_BEGIN);
		}

		void BinaryWriter::jsonEndMap()
		{
			writeId( Token::JID_MAP_END);
		}

		void BinaryWriter::jsonString( const std::string &value )
		{
			writeId( Token::JID_STRING );
			writeLength( value.size() );
			write<sbyte>( (const sbyte *)&value[0], (sint64)value.size() );
		}

		void BinaryWriter::jsonKey( const std::string &key )
		{
			jsonString(key);
		}

		void BinaryWriter::jsonInt( const sint64 &value )
		{
			if( (value >= -0x80) && (value < 0x80) )
			{
				writeId( Token::JID_INT8 );
				write<sbyte>( (sbyte)value );
			}else if( (value >= -0x8000) && (value < 0x8000) )
			{
				writeId( Token::JID_INT16 );
				write<sword>( (sword)value );
			}else if( (value >= -(sint64)0x80000000) && (value < (sint64)0x80000000) )
			{
				writeId( Token::JID_INT32 );
				write<sint32>( (sint32)value );
			}else
			{
				writeId( Token::JID_INT64 );
				write<sint64>( value );
			}
		}

		void BinaryWriter::jsonUInt8( const ubyte &value )
		{
			writeId( Token::JID_UINT8 );
			write<sbyte>( (ubyte)value );
		}

		void BinaryWriter::jsonInt8( const sbyte &value )
		{
			writeId( Token::JID_INT8 );
			write<sbyte>( (sbyte)value );
		}

		void BinaryWriter::jsonInt32( const sint32 &value )
		{
			writeId( Token::JID_INT32 );
			write<sint32>( (sint32)value );
		}

		void BinaryWriter::jsonInt64( const sint64 &value )
		{
			writeId( Token::JID_INT64 );
			write<sint64>( value );
		}

		void BinaryWriter::jsonReal32( const real32 &value )
		{
			writeId( Token::JID_REAL32 );
			write<real32>(value );
		}

		void BinaryWriter::jsonReal64( const real64 &value )
		{
			writeId( Token::JID_REAL64 );
			write<real64>(value );
		}

		void BinaryWriter::jsonBool( const bool &value )
		{
			if( value )
				writeId( Token::JID_TRUE );
			else
				writeId( Token::JID_FALSE );
		}



		// ASCIIWriter =============================================

		ASCIIWriter::ASCIIWriter( std::ostream *out )
		{
			stream = out;
			gotKey = false;
			firstItem = false;
			indentLevel = 0;
		}

		void ASCIIWriter::write( const std::string &text )
		{
			stream->write( text.c_str(), text.size() );
		}

		void ASCIIWriter::jsonBeginArray()
		{
			writePrefix();
			stack.push( Token::JID_ARRAY_BEGIN );
			writeNewline();
			write( "[" );
			++indentLevel;
			writeNewline();
			firstItem = true;
		}

		void ASCIIWriter::jsonEndArray()
		{
			firstItem = false;
			stack.pop();
			--indentLevel;
			writeNewline();
			write( "]" );
		}

		void ASCIIWriter::jsonBeginMap()
		{
			writePrefix();
			stack.push( Token::JID_MAP_BEGIN );
			writeNewline();
			write( "{" );
			++indentLevel;
			writeNewline();
			firstItem = true;
		}

		void ASCIIWriter::jsonEndMap()
		{
			firstItem = false;
			stack.pop();
			--indentLevel;
			writeNewline();
			write( "}" );
		}

		void ASCIIWriter::jsonString( const std::string &value )
		{
			writePrefix();
			write( "\"" + value + "\"" );
		}

		void ASCIIWriter::jsonKey( const std::string &key )
		{
			writePrefix();
			write( "\"" + key + "\"" );
			gotKey = true;
		}

		void ASCIIWriter::jsonInt( const sint64 &value )
		{
			writePrefix();
			write( toString<sint64>(value) );
		}

		void ASCIIWriter::jsonUInt8( const ubyte &value )
		{
			writePrefix();
			write( toString<ubyte>(value) );
		}

		void ASCIIWriter::jsonInt8( const sbyte &value )
		{
			writePrefix();
			write( toString<sbyte>(value) );
		}

		void ASCIIWriter::jsonInt32( const sint32 &value )
		{
			writePrefix();
			write( toString<sint32>(value) );
		}

		void ASCIIWriter::jsonInt64( const sint64 &value )
		{
			writePrefix();
			write( toString<sint64>(value) );
		}

		void ASCIIWriter::jsonReal32( const real32 &value )
		{
			writePrefix();
			std::string str = toString<real32>(value);
			write( str );
			// if string is not in scientific notation and doesnt contain a point
			// we will append one to make sure the value is loaded as float back in
			if( (str.find( 'e' ) == std::string::npos)&&(str.find( '.' ) == std::string::npos) )
				write( ".0" );
		}

		void ASCIIWriter::jsonReal64( const real64 &value )
		{
			writePrefix();
			write( toString<real64>(value) );
		}

		void ASCIIWriter::jsonBool( const bool &value )
		{
			writePrefix();
			if( value )
				write( "true" );
			else
				write( "false" );
		}

		void ASCIIWriter::writePrefix()
		{
			if( firstItem )
			{
				firstItem = false;
			}else
			if( gotKey )
			{
				gotKey = false;
				write( ":" );
			}else
			if( !stack.empty() )
			{
				if( (stack.top() == Token::JID_MAP_BEGIN)||
					(stack.top() == Token::JID_ARRAY_BEGIN))
				{
					write( ", " );
					writeNewline();
				}
				else
				{
					write( ", " );
				}
			}
		}

		void ASCIIWriter::writeNewline()
		{
			write( "\n" );
			for( int i=0;i<indentLevel;++i )
				write( "\t" );
		}



		// JSONCPP ==================================================

		
		// Value ----
		Value::Value() : m_type(Value::TYPE_NULL)
		{
		}

		void Value::cpyTo( char *dst )const
		{
			switch( m_value.which() )
			{
				//bool
				case 0: memcpy( dst, &ttl::var::get<bool>(m_value), sizeof(bool));break;
				//sint32
				case 1: memcpy( dst, &ttl::var::get<sint32>(m_value), sizeof(sint32));break;
				//real32
				case 2:	memcpy( dst, &ttl::var::get<real32>(m_value), sizeof(real32));break;
				//real64
				case 3: memcpy( dst, &ttl::var::get<real64>(m_value), sizeof(real64));break;
				//ubyte
				case 4: memcpy( dst, &ttl::var::get<ubyte>(m_value), sizeof(ubyte));break;
				//sint64
				case 5: memcpy( dst, &ttl::var::get<sint64>(m_value), sizeof(sint64));break;
			}
		}

		ArrayPtr Value::asArray()
		{
			return m_array;
		}

		ObjectPtr Value::asObject()
		{
			return m_object;
		}

		Value::Variant &Value::getVariant()
		{
			return m_value;
		}

		bool Value::isNull()const
		{
			return m_type==TYPE_NULL;
		}

		bool Value::isArray()const
		{
			return m_type==TYPE_ARRAY;
		}

		bool Value::isObject()const
		{
			return m_type==TYPE_OBJECT;
		}

		bool Value::isString()const
		{
			return m_value.which()==4;
		}

		Value Value::createArray()
		{
			Value v;
			v.m_type = TYPE_ARRAY;
			v.m_array = ArrayPtr( new Array() );
			return v;
		}
		Value Value::createArray(ArrayPtr array)
		{
			Value v;
			v.m_type = TYPE_ARRAY;
			v.m_array = array;
			return v;
		}

		Value Value::createObject()
		{
			Value v;
			v.m_type = TYPE_OBJECT;
			v.m_object = Object::create();
			return v;
		}

		Value Value::createObject(ObjectPtr obj)
		{
			Value v;
			v.m_type = TYPE_OBJECT;
			v.m_object = obj;
			return v;
		}

		// Array ----
		Array::Array() : m_isUniform(false), m_uniformdata(0), m_numUniformElements(0)
		{
		}

		Array::~Array()
		{
			if( m_isUniform && m_uniformdata )
				free(m_uniformdata);
		}

		ArrayPtr Array::create()
		{
			return std::make_shared<Array>();
		}

		bool Array::isUniform()const
		{
			return m_isUniform;
		}

		void Array::append( Value &value )
		{
			m_values.push_back( value );
		}

		void Array::append(ObjectPtr &object)
		{
			Value v;
			v.m_type = Value::TYPE_OBJECT;
			v.m_object = object;
			append(v);
		}

		void Array::append(ArrayPtr &array)
		{
			Value v;
			v.m_type = Value::TYPE_ARRAY;
			v.m_array = array;
			append(v);
		}

		sint64 Array::size()const
		{
			if( m_isUniform )
				return m_numUniformElements;
			return m_values.size();
		}

		Value Array::getValue( const int index )
		{
			if( m_isUniform )
			{
				switch( m_uniformType )
				{
					//bool
				case 0: return Value::create<bool>( *((bool *)(&m_uniformdata[sizeof(bool)*index])) );break;
					//sint32
				case 1: return Value::create<sint32>( *((sint32 *)(&m_uniformdata[sizeof(sint32)*index])) );break;
					//real32
				case 2: return Value::create<real32>( *((real32 *)(&m_uniformdata[sizeof(real32)*index])) );break;
					//real64
				case 3: return Value::create<real64>( *((real64 *)(&m_uniformdata[sizeof(real64)*index])) );break;
				case 4: return Value::create<std::string>("error in Array::getValue - uniform string arrays not supported");break;
					//ubyte
				case 5: return Value::create<ubyte>( *((ubyte *)(&m_uniformdata[sizeof(ubyte)*index])) );break;
					//sint64
				case 6: return Value::create<sint64>( *((sint64 *)(&m_uniformdata[sizeof(sint64)*index])) );break;
				}
			}
			return m_values[index];
		}


		ObjectPtr                getObject( int index );
		ObjectPtr Array::getObject( int index )
		{
			Value v = getValue(index);
			if(v.isObject())
				return v.asObject();
			return ObjectPtr();
		}


		ArrayPtr Array::getArray( int index )
		{
			Value v = getValue(index);
			if(v.isArray())
				return v.asArray();
			return ArrayPtr();
		}

		// Object ----
		ObjectPtr Object::create()
		{
			return std::make_shared<Object>();
		}

		bool Object::hasKey( const std::string &key )
		{
			return m_values.find(key) != m_values.end();
		}

		Value Object::getValue( const std::string &key )
		{
			std::map<std::string, Value>::iterator it = m_values.find( key );
			if( it != m_values.end())
				return it->second;
			return Value();
		}

		void Object::getKeys( std::vector<std::string> &keys )
		{
			keys.clear();
			for( std::map<std::string, Value>::iterator it = m_values.begin(), end = m_values.end(); it != end; ++it )
				keys.push_back(it->first);
		}

		ObjectPtr Object::getObject( const std::string &key )
		{
			Value v = getValue(key);
			if(v.isObject())
				return v.asObject();
			return ObjectPtr();
		}

		ArrayPtr Object::getArray( const std::string &key )
		{
			Value v = getValue(key);
			if(v.isArray())
				return v.asArray();
			return ArrayPtr();
		}
		
		sint64 Object::size()const
		{
			return m_values.size();
		}

		void Object::append( const std::string &key, Value &value )
		{
			m_values.insert( std::make_pair(key, value) );
		}

		void Object::append( const std::string &key, ObjectPtr object )
		{
			Value v;
			v.m_type = Value::TYPE_OBJECT;
			v.m_object = object;
			m_values.insert( std::make_pair(key, v) );
		}

		void Object::append(const std::string &key, ArrayPtr array)
		{
			Value v;
			v.m_type = Value::TYPE_ARRAY;
			v.m_array = array;
			m_values.insert( std::make_pair(key, v) );
		}

		// JSONReader ===============================================

		JSONReader::JSONReader()
		{

		}

		Value JSONReader::getRoot()
		{
			return m_root;
		}

		void JSONReader::push()
		{
			if( !m_root.isNull() )
				m_stack.push( std::make_pair(m_root, nextKey) );
		}

		void JSONReader::pop()
		{
			if( !m_stack.empty() )
			{
				Value v = m_root;
				StackItem si = m_stack.top();
				m_root = si.first;
				m_stack.pop();
				if( m_root.isArray() )
					m_root.asArray()->append(v);
				else
				if( m_root.isObject() )
					m_root.asObject()->append(si.second, v);
			}
		}

		void JSONReader::jsonBeginArray()
		{
			push();
			m_root = Value::createArray();
		}

		void JSONReader::jsonEndArray()
		{
			pop();
		}

		void JSONReader::jsonBeginMap()
		{
			push();
			m_root = Value::createObject();
		}

		void JSONReader::jsonEndMap()
		{
			pop();
		}

		void JSONReader::jsonKey( const std::string &key )
		{
			nextKey = key;
		}

		void JSONReader::jsonString( const std::string &value )
		{
			jsonValue<std::string>(value);
		}

		void JSONReader::jsonBool( const bool &value )
		{
			jsonValue<bool>(value);
		}

		void JSONReader::jsonInt32( const sint32 &value )
		{
			jsonValue<sint32>(value);
		}

		void JSONReader::jsonReal32( const real32 &value )
		{
			jsonValue<real32>(value);
		}


		void JSONReader::uaBool( sint64 numElements, Parser *parser )
		{
			//In binary JSON files, uniform bool arrays are stored as bit
			//streams.  This method decodes the bit-stream, calling jsonBool()
			//for each element of the bit array.

			jsonBeginArray();
			int count = (int) numElements;
			while( count > 0 )
			{
				uint32 bits;
				parser->read<uint32>( &bits, 1 );
				int nbits = std::min( count, 32 );
				count -= nbits;
				for( int i=0;i<nbits;++i )
					jsonBool( (bits & (1 << i)) != 0 );
			}
			jsonEndArray();

		}

		void JSONReader::uaReal32( sint64 numElements, Parser *parser )
		{
			jsonUA<real32, real32>(numElements, parser);
		}

		void JSONReader::uaReal64( sint64 numElements, Parser *parser )
		{
			jsonUA<real64, real64>(numElements, parser);
		}

		void JSONReader::uaInt16( sint64 numElements, Parser *parser )
		{
			jsonUA<sint32, sword>(numElements, parser);
		}

		void JSONReader::uaInt32( sint64 numElements, Parser *parser )
		{
			jsonUA<sint32, sint32>(numElements, parser);
		}

		void JSONReader::uaInt64( sint64 numElements, Parser *parser )
		{
			jsonUA<sint64, sint64>(numElements, parser);
		}

		void JSONReader::uaUInt8( sint64 numElements, Parser *parser )
		{
			jsonUA<ubyte, ubyte>(numElements, parser);
		}

		
		void JSONReader::uaString( sint64 numElements, Parser *parser )
		{
			jsonBeginArray();
			for(sint64 i=0;i<numElements;++i)
				jsonString( parser->readBinaryString() );
			jsonEndArray();
		}


		// JSONWriter =========================================

		bool JSONWriter::write( ObjectPtr object )
		{
			m_writer->jsonBeginMap();
			for( std::map<std::string, Value>::iterator it = object->m_values.begin(), end = object->m_values.end(); it != end; ++it )
			{
				m_writer->jsonKey( it->first );
				write( it->second );
			}
			m_writer->jsonEndMap();
			return true;
		}

		bool JSONWriter::write( ArrayPtr array )
		{
			m_writer->jsonBeginArray();
			for( std::vector<Value>::iterator it = array->m_values.begin(), end = array->m_values.end(); it != end; ++it )
				write( *it );
			m_writer->jsonEndArray();
			return true;
		}

		bool JSONWriter::write( Value &value )
		{
			if( value.isArray() )
				return write( value.asArray() );
			else
			if( value.isObject() )
				return write( value.asObject() );
			else
			if( !value.isNull() )
			{
				ttl::var::apply_visitor( *this, value.getVariant() );
			}
			return false;
		}

	}
}
