#include <houio/Field.h>


namespace houio
{
	// needs to go to Field.cpp for gcc compatibility...
	template<> const int Field<float>::m_dataType = 1;
	template<> const int Field<math::V3f>::m_dataType = 2;
	template<> const int Field<double>::m_dataType = 3;
	template<> const int Field<math::V3d>::m_dataType = 4;
}
