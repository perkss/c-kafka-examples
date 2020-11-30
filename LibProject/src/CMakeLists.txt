# Notice name prefix of this variable, set by CMake according
# to value given with "project()" in the root CMakeLists.txt.
include_directories(${CKafkaExamples_SOURCE_DIR}/LibProject/include)
add_library(LibProject consumer.c producer.c UpperCaseTopology.c)

find_package(PkgConfig REQUIRED)

# Resolve Mac issue linking Openssl
set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/openssl/lib/pkgconfig")

pkg_check_modules(KAFKA REQUIRED IMPORTED_TARGET rdkafka)

target_link_libraries(LibProject PRIVATE PkgConfig::KAFKA)