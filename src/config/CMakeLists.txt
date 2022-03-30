# Set the project name
project(config)

# Add a library with the above sources
add_library(${PROJECT_NAME} config.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC sub::visualization)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal::cereal)


target_include_directories( ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )


add_library(sub::config ALIAS ${PROJECT_NAME})

