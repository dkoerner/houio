CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )

PROJECT ( houio )
set( HOUIO_VERSION 0.1 )

set(CMAKE_DEBUG_POSTFIX "d")
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

# trigger cxx standard (c++11)
set_property(TARGET houio PROPERTY CXX_STANDARD 11)

add_subdirectory ( tests )

# install target (the lib file) and register the target in export set ---
install( TARGETS houio DESTINATION lib EXPORT houio-targets )
# copy header files
install( DIRECTORY include DESTINATION . )


# location of cmake config files in binary tree
set(ConfigPackageLocation lib/cmake/houio)

include(CMakePackageConfigHelpers)

# create version config file ---
write_basic_package_version_file( "${CMAKE_CURRENT_BINARY_DIR}/houio/houio-config-version.cmake" VERSION ${HOUIO_VERSION} COMPATIBILITY AnyNewerVersion )

# export targets (which have been defined during previous calls to install)
export(EXPORT houio-targets FILE "${CMAKE_CURRENT_BINARY_DIR}/houio/houio-targets.cmake" NAMESPACE Upstream:: )


# install houio-targets cmake file ---
install( EXPORT houio-targets DESTINATION ${ConfigPackageLocation} )


# install version and config package file ---
install( FILES cmake/houio-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/houio/houio-config-version.cmake" DESTINATION ${ConfigPackageLocation} COMPONENT Devel )

# for the final config file you can also use configure_file to replace some variables and move it into place
# create final package config file (by copying from source-tree)
#configure_file(cmake/houio-config.cmake "${CMAKE_CURRENT_BINARY_DIR}/houio/houio-config.cmake" COPYONLY )

# tests -----------
add_subdirectory( tests )

