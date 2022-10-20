add_executable(test-application application.cpp)
target_link_libraries(test-application PRIVATE oocmd)
add_test(application ${CMAKE_CURRENT_BINARY_DIR}/test-application)
