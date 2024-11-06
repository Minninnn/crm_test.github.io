cmake_minimum_required(VERSION 3.20)
project(test)

set(PostgreSQL_ADDITIONAL_VERSIONS "17")

add_subdirectory("C:/Programs/libpqxx-7.9.2" libpqxx-build)

set(CMAKE_CXX_STANDARD 17)
find_package(Restbed REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(libpqxx REQUIRED CONFIG)

add_executable(test main.cpp)

target_link_libraries(test PRIVATE unofficial::restbed::restbed nlohmann_json::nlohmann_json pqxx )
