cmake_minimum_required(VERSION 3.0)
project(SFML_Oscilloscope)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 

# Add user source files
file(GLOB SOURCES "src/*.cpp")
# Add user header files
file(GLOB HEADERS "include/*.h")

set(CMAKE_MODULE_PATH "" CACHE PATH "Where is FindSFML.cmake?" ${CMAKE_MODULE_PATH})

# Find SFML
find_package(SFML 2.4 COMPONENTS graphics window system)

# Output an error if SFML wasn't found
if(SFML_FOUND)
	message("SFML Found")
else()
	set(SFML_ROOT "" CACHE PATH "SFML top-level directory")	
	message("\n-> SFML directory not found. Set SFML_ROOT to SFML's top-level path (containing \"include\" and \"lib\" directories).")
	message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

# Serial
set(SERIAL_ROOT_DIR "" CACHE PATH "Path to serial root directory")
find_library(SERIAL_LIBRARIES_RELEASE NAMES "serial" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)
find_library(SERIAL_LIBRARIES_DEBUG NAMES "serial-d" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)
set(SERIAL_LIBRARIES debug ${SERIAL_LIBRARIES_DEBUG} optimized ${SERIAL_LIBRARIES_RELEASE})
find_path(SERIAL_INCLUDE_DIR NAME "serial/serial.h" PATHS "${SERIAL_ROOT_DIR}/*" NO_DEFAULT_PATH)

# FFTW 3
set(FFTW3_ROOT_DIR "" CACHE PATH "Path to FFTW3 root directory")
find_library(FFTW3_LIBRARIES NAME "libfftw3-3" PATHS ${FFTW3_ROOT_DIR} NO_DEFAULT_PATH)
find_path(FFTW3_INCLUDE_DIR NAME "fftw3.h" PATHS ${FFTW3_ROOT_DIR} NO_DEFAULT_PATH)

link_directories(${SERIAL_LIB_DIR} ${FFTW3_LIB_DIR})
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include ${SFML_INCLUDE_DIR} ${SERIAL_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR})

if (WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${SERIAL_LIBRARIES}
	${FFTW3_LIBRARIES}
	${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif ()
	
if (UNIX AND NOT APPLE)
target_link_libraries(${PROJECT_NAME} PRIVATE
	${SERIAL_LIBRARIES}
	${FFTW3_LIBRARIES}
	${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif ()
