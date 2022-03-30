#add_subdirectory(cereal-1.3.1)
#add_subdirectory(docopt)
include(FetchContent)

FetchContent_Declare(
  structopt
  GIT_REPOSITORY https://github.com/p-ranav/structopt
)

FetchContent_Declare(
  cereal
  GIT_REPOSITORY https://github.com/USCiLab/cereal
)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9
)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG eb3220622e73a4889eee355ffa37972b3cac3df5
)

FetchContent_MakeAvailable(structopt cereal fmt spdlog)
add_library(spdlog::spdlog ALIAS spdlog)

add_library(rapidcsv INTERFACE)
target_include_directories(rapidcsv INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidcsv)

