cmake_minimum_required(VERSION 3.10)
project(DocsBackend)

set(CMAKE_CXX_STANDARD 17)

# Download and setup asio using FetchContent
include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG        asio-1-28-0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(asio)

# Create an interface library for asio (header-only)
# Note: asio standalone structure has headers in asio/include/asio/
if(NOT TARGET asio::asio)
    add_library(asio::asio INTERFACE IMPORTED)
    set_target_properties(asio::asio PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${asio_SOURCE_DIR}/asio/include"
    )
endif()

# Download and setup Crow framework using URL (more reliable than git tags)
FetchContent_Declare(
    crow
    URL https://github.com/CrowCpp/Crow/archive/refs/heads/master.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(crow)

# Create an interface library for Crow
if(NOT TARGET crow::crow)
    add_library(crow::crow INTERFACE IMPORTED)
    set_target_properties(crow::crow PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${crow_SOURCE_DIR}/include"
    )
endif()

# Find SQLite3
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# Find OpenSSL for crypto operations
find_package(OpenSSL REQUIRED)

include_directories(include)

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
    crow::crow
    ${SQLITE3_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
)
target_compile_definitions(docs_app PRIVATE ASIO_STANDALONE CROW_ENABLE_WEBSOCKET)
target_include_directories(docs_app PRIVATE 
    ${SQLITE3_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)
