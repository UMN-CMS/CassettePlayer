# Set the project name
project(core)

add_library(${PROJECT_NAME} operation.cpp slots.cpp instruction.cpp)

target_include_directories( ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
  )


target_link_libraries(${PROJECT_NAME} PRIVATE ${wxWidgets_LIBRARIES}) 
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal::cereal)

add_library(sub::core ALIAS ${PROJECT_NAME})

