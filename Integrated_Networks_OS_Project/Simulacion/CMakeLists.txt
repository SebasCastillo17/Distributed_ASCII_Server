cmake_minimum_required(VERSION 3.10)

# project name
project(Entrega1_Cliente)

set(CMAKE_CXX_STANDARD 14)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# add the subdirectories that have a file "CMakeLists.txt"
# you can add unit test for example
add_subdirectory(src)

