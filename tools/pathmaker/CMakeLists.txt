add_executable(PathMaker pathmaker.cpp)

target_include_directories(PathMaker PUBLIC ${wxWidgets_USE_FILE})
include(${wxWidgets_USE_FILE})
target_link_libraries(PathMaker PUBLIC fmt::fmt)
target_link_libraries(PathMaker PUBLIC spdlog::spdlog)

target_link_libraries(PathMaker PRIVATE sub::visualization)
target_link_libraries(PathMaker PRIVATE sub::geometry)
target_link_libraries(PathMaker PUBLIC sub::config)

