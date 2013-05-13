CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(ParticleExample)

#Projects source files
SET(SRC_FILES
	example/main.cpp
    example/game_window.cpp
	example/game_window.hpp
    src/ParticleEmitter.cpp
    src/ParticleEmitter.hpp
    src/Particle.cpp
    src/Particle.hpp
    src/fast_math.cpp
    src/fast_math.hpp
	)

#Projects headers files
SET(INC_FILES
	)
INCLUDE_DIRECTORIES(example src)

#"Sources" and "Headers" are the group names in Visual Studio.
#They may have other uses too...
SOURCE_GROUP("Sources" FILES ${SRC_FILES})
SOURCE_GROUP("Headers" FILES ${INC_FILES})
SOURCE_GROUP("Build System" FILES CMakeLists.txt)

find_package(Gosu REQUIRED)

INCLUDE_DIRECTORIES(${Gosu_INCLUDE_DIRS})
LINK_DIRECTORIES(${Gosu_LIBRARY_DIRS})

set(CMAKE_CXX_COMPILER g++-4.7)
#set(CPPFLAGS ${CPPFLAGS} -D_GLIBCXX_DEBUG)

#Build
ADD_EXECUTABLE(ParticleExample ${SRC_FILES})
set_target_properties(ParticleExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/example)

IF(MSVC)
	SET_TARGET_PROPERTIES(ParticleExample PROPERTIES COMPILE_FLAGS "/W4 /wd4127")
ENDIF(MSVC)
SET_TARGET_PROPERTIES(ParticleExample PROPERTIES COMPILE_FLAGS "-g -pedantic -Wall -Wextra -O2")
TARGET_LINK_LIBRARIES(ParticleExample ${Gosu_LIBRARIES})
