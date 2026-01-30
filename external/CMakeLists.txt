set(CMAKE_C_CLANG_TIDY "")
set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "")
set(CMAKE_CXX_CPPCHECK "")

add_subdirectory(IXWebSocket)
add_subdirectory(jwt-cpp)
add_subdirectory(SimpleAmqpClient)
add_subdirectory(prometheus-cpp)
