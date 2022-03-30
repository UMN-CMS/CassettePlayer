add_library(vmath INTERFACE)
target_link_libraries(vmath INTERFACE spdlog::spdlog)
target_link_libraries(vmath INTERFACE cereal::cereal)

target_include_directories( vmath
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
add_library(sub::vmath ALIAS vmath)

add_library(geometry geometry.cpp)

target_include_directories( geometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories( geometry
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/geometry
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

target_link_libraries(geometry PUBLIC sub::core)
target_link_libraries(geometry PRIVATE fmt::fmt)
target_link_libraries(geometry PRIVATE spdlog::spdlog)
target_link_libraries(geometry PRIVATE cereal::cereal)
target_link_libraries(geometry PUBLIC sub::visualization)
target_link_libraries(geometry PUBLIC sub::vmath)

add_library(sub::geometry ALIAS geometry)

