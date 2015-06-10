CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( houio )

INCLUDE_DIRECTORIES ( include )

ADD_LIBRARY ( houio STATIC
  src/Attribute.cpp
  src/Field.cpp
  src/math/Color.cpp
  src/math/Math.cpp
  src/json.cpp
  src/HouGeoAdapter.cpp
  src/HouGeo.cpp
  src/HouGeoIO.cpp
  src/Geometry.cpp
  )

INSTALL ( TARGETS
  houio
  DESTINATION
  lib
  )

INSTALL ( DIRECTORY
  include
  DESTINATION
  .
  )
