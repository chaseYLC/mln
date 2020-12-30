#ADD_DEFINITIONS(-DBoost_USE_STATIC_LIBS=ON)
SET(Boost_ADDITIONAL_VERSIONS "1.72" "1.72.0")

if(WIN32)
    set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
endif()
# set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
# set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
# set(Boost_USE_STATIC_RUNTIME    OFF)

SET ( CMAKE_INSTALL_PREFIX "/usr/local/mln" )

set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
    )

add_compile_definitions(USE_SPDLOG_HEX_SUPPORT) 
add_compile_definitions(USE_SPDLOG_HEX_RINGBUFFER_SINK) 


# SET ( CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" )
# SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE} )
# SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )
# SET ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )


find_package(Boost REQUIRED system date_time serialization filesystem thread chrono regex log)
if(Boost_FOUND)
    message(STATUS "---------------------")
    message(STATUS "Boost_FOUND: ${Boost_FOUND}")
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "---------------------")

    include_directories(${Boost_INCLUDE_DIRS} 
        "${CMAKE_SOURCE_DIR}/mlnserver/"
    )

    include_directories("../protocol/packet_generated/")

    if(WIN32)
        message("win 32 build!")
    elseif(UNIX)
        message("linux build!")
    endif()

    add_executable(mlnserver
        stdafx.cpp
        httpRequestManager.cpp
        keyEventHandler.cpp
        lobbyAcceptorReceiver.cpp
        main.cpp
        packetJsonHandler.cpp
        packetParserJson.cpp
        restServer/restServer.cpp
        restServer/restHandler.cpp
        user/lobbyUser.cpp
        user/lobbyUserManager.cpp
        watchDogHandler.cpp
        xrUtils.cpp
       )

    target_link_libraries(mlnserver
        mln
        ${Boost_LIBRARIES}
        ${LIB_RAKNET}
    )

    set_target_properties( mlnserver
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mlnserver/bin"
    )
    
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/netconfig.json
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin )
    
elseif()
    message("Boost NOT Found!")
endif()

