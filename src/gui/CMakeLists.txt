# Set the project name
project(gui)

add_library(${PROJECT_NAME} instructdv.cpp instruction_frame.cpp oppanel.cpp oprecord.cpp visframe.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${wxWidgets_USE_FILE})
include(${wxWidgets_USE_FILE})

target_include_directories( ${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE datamanager)

add_library(sub::gui ALIAS ${PROJECT_NAME})


