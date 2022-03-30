cmake_minimum_required(VERSION 3.2)
project(App VERSION 0.1) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()


set(JUST_INSTALL_CEREAL True)


if (MSVC)
  add_compile_options(/W4 /WX)
  add_compile_options(
    $<$<CONFIG:RELEASE>:/O3>
    $<$<CONFIG:DEBUG>:/O0>
    $<$<CONFIG:DEBUG>:/g>
    )
else()
  add_compile_options(-Wall -Wextra -pedantic)
  add_compile_options(
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:DEBUG>:-g>
    )
endif()

configure_file(AppInfo.h.in ${PROJECT_SOURCE_DIR}/src/AppInfo.h)

find_package(wxWidgets REQUIRED COMPONENTS core base html adv)
include(${wxWidgets_USE_FILE})

add_subdirectory(external)

add_subdirectory(src)

add_subdirectory(tools)

unset(CMAKE_BUILD_TYPE CACHE)
