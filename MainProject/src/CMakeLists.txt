# If header file exists in here
include_directories(${CKafkaExamples_SOURCE_DIR}/MainProject/inc)
# Include the library header files
include_directories(${CKafkaExamples_SOURCE_DIR}/LibProject/include)
link_directories(${CKafkaExamples_SOURCE_DIR}/LibProject/src)
add_executable(MainProject main.c)
# Add the library we build project
target_link_libraries(MainProject LibProject)