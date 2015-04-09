/*---------------------------------------------------------------------



----------------------------------------------------------------------*/
#pragma once

namespace houio
{


namespace math
{
	///
	/// \brief similar to the math::Vec3f but specialized to usage as color
	///
	class Color
	{
	public:
		Color();
		Color( const float &r, const float &g, const float &b, const float &a = 1.0f );
		~Color();

		// standard colors
		static Color                                                              White();
		static Color                                                              Black();
		static Color                                                               Blue();
		static Color                                                             Yellow();
		static Color                                                              Green();
		static Color                                                                Red();
		static Color From255( const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a = 255 );


        void set( const float &r, const float &g, const float &b, const float &a = 1.0f ); 

		void                                                                clamp( void ); ///< clamp the component values into the range of [0,1]
        void                                                               invert( void ); ///< invert the color -> each component is 1.0f - value
		unsigned long                                                   makeDWORD( void ); ///< returns a RGBA(4x8bit) representation of the color

		// operators
		bool                                               operator==( const Color &rhs );
        bool                                               operator!=( const Color &rhs );
        bool                                               operator+=( const Color &rhs );
        bool                                               operator-=( const Color &rhs );
        bool                                               operator*=( const Color &rhs );

        bool                                               operator+=( const float &rhs );
        bool                                               operator-=( const float &rhs );
        bool                                               operator*=( const float &rhs );
        bool                                               operator/=( const float &rhs );

        const                                            float& operator[]( int i ) const; ///< returns ith component
        float&                                                        operator[]( int i ); ///< returns ith component

		union
		{
			struct
			{
				float	r, g, b, a;
			};
			float v[4];
		};
	};
}

} // namespace houio
