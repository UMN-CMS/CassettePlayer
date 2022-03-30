# Set the project name
project(visualization)

# Add a library with the above sources
add_library(${PROJECT_NAME} drawables.cpp visualization.cpp color.cpp)

include(${wxWidgets_USE_FILE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES}) 


target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC sub::core)
target_link_libraries(${PROJECT_NAME} PRIVATE sub::geometry)
target_link_libraries(${PROJECT_NAME} PUBLIC sub::vmath)
target_link_libraries(${PROJECT_NAME} PRIVATE sub::config)


target_include_directories( ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )


add_library(sub::visualization ALIAS ${PROJECT_NAME})

