cmake_minimum_required (VERSION 3.3.2)

project(malloc VERSION 1.0.0 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

if(ENABLE_PROT)
    message(STATUS "Enabling mprotect support.")
    add_definitions(-DENABLE_PROT)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(GNUInstallDirs)

if(BUILD_TESTS)
    find_package(Cmocka)
    if (CMOCKA_FOUND)
        enable_testing()
        add_subdirectory(Tests)
        message(STATUS "Tests building enabled.")
    else()
        message(WARNING "Missing cmocka.")
    endif()
endif()

configure_file(Extra/libmalloc.pc.in libmalloc.pc @ONLY)

# Find source files
file(GLOB SOURCES Lib/*.c)

message(STATUS "Building Libmalloc ${PROJECT_VERSION}-${CMAKE_BUILD_TYPE} for ${CMAKE_SYSTEM_NAME}.")

# Find public headers
file(GLOB PUBLIC_H Lib/public/${PROJECT_NAME}/*.h)

# Include private and public headers
include_directories(Lib Lib/public/${PROJECT_NAME})

add_library(${PROJECT_NAME} SHARED ${SOURCES})

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${PUBLIC_H}"
)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wtype-limits -Wstrict-overflow -fno-strict-aliasing -Wformat -Wformat-security") 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

target_include_directories(${PROJECT_NAME} PRIVATE Lib/ Lib/public/${PROJECT_NAME}/)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(FILES ${CMAKE_BINARY_DIR}/libmalloc.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/${PROJECT_NAME}/)
