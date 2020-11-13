cmake_minimum_required(VERSION 3.17)
project(gw2itemwatch)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

find_package(wxWidgets REQUIRED COMPONENTS core base)
find_package(SQLite3 REQUIRED CONFIG)

include(${wxWidgets_USE_FILE})
include_directories(third-party)

add_executable(gw2itemwatch WIN32
        src/main.cpp
        src/frame.cpp
        src/Database.cpp
        src/PricesAPI.cpp src/ListingsModel.cpp src/ListingsModel.h src/listings.cpp src/UpdateTimer.cpp src/UpdateTimer.h src/FetchListingsThread.cpp src/FetchListingsThread.h)

target_link_libraries(gw2itemwatch ${wxWidgets_LIBRARIES} cpr::cpr SQLite::SQLite3 nlohmann_json::nlohmann_json)
