cmake_minimum_required(VERSION 3.7)
project(Orb)

set(CMAKE_CXX_STANDARD 14)

option(UPDATE_REPOS "Update third party repositories" OFF)

set(NO_UPDATE 1)
if (${UPDATE_REPOS})
    set(NO_UPDATE 0)
endif ()

include(cmake/DownloadProject.cmake)

configure_file(src/ShaderConfig.hpp.in ${CMAKE_BINARY_DIR}/orb/ShaderConfig.hpp)

# Download and unpack simulation driver at configure time
download_project(PROJ sim
        PREFIX thirdparty
        GIT_REPOSITORY https://github.com/LoganBarnes/SimulationDriver.git
        GIT_TAG master
        UPDATE_DISCONNECTED ${NO_UPDATE}
        QUIET
        )

set(SIM_BUILD_CUDA OFF CACHE BOOL "" FORCE)
set(SIM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SIM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# compile sim with current project
add_subdirectory(${sim_SOURCE_DIR} ${sim_BINARY_DIR})

set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS})
if (NOT MSVC)
    set(COMPILER_FLAGS "${COMPILER_FLAGS} -Wall -Wextra -Werror -pedantic")
endif ()

set(SOURCE_FILES
        # shaders
        src/shaders/orb.vert
        # orb
        src/orb/Orb
        src/orb/OrbRenderer
        src/orb/OrbUtil
        src/orb/OrbMain
        )

add_executable(OrbExec ${SOURCE_FILES})
target_include_directories(OrbExec PUBLIC src ${CMAKE_BINARY_DIR})
target_link_libraries(OrbExec SimDriver)
set_target_properties(OrbExec PROPERTIES COMPILE_FLAGS ${COMPILER_FLAGS})
