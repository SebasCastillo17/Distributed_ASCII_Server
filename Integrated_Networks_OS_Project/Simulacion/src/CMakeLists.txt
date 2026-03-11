set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)

# Configure cpplint for test files
find_program(CPPLINT cpplint)
if(CPPLINT)
    add_custom_target(run_cpplint ALL
        COMMAND ${CPPLINT} ${INCLUDE_DIR}/*.h ${SOURCE_DIR}/*.cpp ${TEST_DIR}/*.cpp || true
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cpplint on header, source, and test files"
    )
endif()

include_directories(${INCLUDE_DIR})

# Check the operating system
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Add files to compile with the supermarket file for Windows
    add_executable(simulation WIN32
        ${SOURCE_DIR}/main.cpp
    )
    else()
    # Add files to compile with the supermarket file for other systems (e.g., Linux)
    add_executable(simulation
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/Client.cpp
    ${SOURCE_DIR}/DrawingServer.cpp
    ${SOURCE_DIR}/Tenedor.cpp
    ${SOURCE_DIR}/SharedMemory.cpp
    ${SOURCE_DIR}/ColabProtocolHandler.cpp
    ${SOURCE_DIR}/FileSystem.c
    ${SOURCE_DIR}/Structures.c
    
    
    )
    target_link_libraries(simulation PRIVATE pthread)
endif()
