add_executable(labwork6 main.cpp)

target_link_libraries(labwork6 PRIVATE ApiLib)
target_link_libraries(labwork6 PRIVATE ParseCmdLib)
target_link_libraries(labwork6 PRIVATE JsonParserLib)

target_include_directories(labwork6 PRIVATE ${PROJECT_SOURCE_DIR})