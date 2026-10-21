add_executable(just-test test.cpp)
target_link_libraries(just-test PRIVATE justio)
add_test(NAME just-parser-tests COMMAND just-test)

add_executable(just-example main.cpp)
target_link_libraries(just-example PRIVATE justio)
add_custom_command(TARGET just-example POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_SOURCE_DIR}/syntax.just"
          "$<TARGET_FILE_DIR:just-example>/syntax.just"
     VERBATIM
)
