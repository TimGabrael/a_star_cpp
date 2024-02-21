cmake_minimum_required(VERSION 3.21)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

project(
  astar_project
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL ""
  LANGUAGES CXX C)


include(cmake/compiler_warnings.cmake)
include(cmake/sanitizers.cmake)



add_executable(astar src/main.cpp)

set_project_warnings(astar
    True # warnings as errors
	""
	""
	""
	""
	"")


enable_sanitizers(astar
	True
	True
	True
	False
	False
)
