add_executable(App app.cpp casdm.cpp)


add_library(datamanager casdm.cpp)



add_subdirectory(geometry)
add_subdirectory(gui)
add_subdirectory(visualization)
add_subdirectory(core)
add_subdirectory(config)

target_link_libraries(datamanager PUBLIC sub::visualization)
target_link_libraries(datamanager PUBLIC sub::core)
target_link_libraries(datamanager PUBLIC sub::geometry)

target_include_directories(datamanager PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(App PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(App PUBLIC ${wxWidgets_LIBRARIES}) 
target_link_libraries(App PUBLIC datamanager)
target_link_libraries(App PUBLIC sub::gui)
target_link_libraries(App PUBLIC sub::config)
