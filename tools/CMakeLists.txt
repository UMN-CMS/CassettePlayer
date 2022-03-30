
add_library(common common.cpp)
target_link_libraries(common rapidcsv sub::core sub::vmath sub::visualization  structopt)

add_executable(item_point item_point.cpp)
target_link_libraries(item_point rapidcsv sub::core sub::vmath sub::visualization  structopt common)

add_executable(instruct_maker instruct.cpp)
target_link_libraries(instruct_maker rapidcsv sub::core sub::vmath structopt common)

add_subdirectory(pathmaker)

