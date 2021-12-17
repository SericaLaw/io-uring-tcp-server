file(GLOB_RECURSE LIB_SOURCES "*.h" "*.cpp")

add_library(uring_server SHARED ${LIB_SOURCES})
target_link_libraries(uring_server uring)