cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(CMocka REQUIRED)

file(GLOB SOURCES *.c)

include_directories("${PROJECT_SOURCE_DIR}/Lib/public")
add_executable(MallocTest ${SOURCES})
add_dependencies(MallocTest MallocTest)
add_test(MallocTest MallocTest)

find_package(Valgrind)
if(VALGRIND_FOUND)
    message(STATUS "Tests will be checked with valgrind.")
    add_test(MallocTest_valgrind valgrind
         --error-exitcode=1 --read-var-info=yes
         --leak-check=full --show-leak-kinds=all
         ./MallocTest)
else()
    message(STATUS "Tests won't be checked with valgrind.")
endif()

target_include_directories(MallocTest PRIVATE ${CMOCKA_INCLUDE_DIR})
target_link_libraries(MallocTest ${CMOCKA_LIBRARIES} ${PROJECT_NAME})
