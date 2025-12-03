cmake_minimum_required(VERSION 3.10)
project(DocsBackend)

set(CMAKE_CXX_STANDARD 17)

# Find asio using crow's find script
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/third_party/crow/cmake)
find_package(asio REQUIRED)

# Find SQLite3
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Find OpenSSL for crypto operations
find_package(OpenSSL REQUIRED)

include_directories(include third_party third_party/crow/include)

file(GLOB SRC_FILES
    src/main.cpp
    src/routes/*.cpp
    src/controllers/*.cpp
    src/services/*.cpp
    src/repositories/*.cpp
    src/models/*.cpp
    src/db/*.cpp
    src/utils/*.cpp
)

add_executable(docs_app ${SRC_FILES})
target_link_libraries(docs_app PRIVATE 
    asio::asio
    ${SQLITE3_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)
target_compile_definitions(docs_app PRIVATE ASIO_STANDALONE CROW_ENABLE_WEBSOCKET)
target_include_directories(docs_app PRIVATE ${SQLITE3_INCLUDE_DIRS})
