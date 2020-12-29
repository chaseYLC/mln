find_program(GIT_EXECUTABLE git)

ExternalProject_Add(
    fmt
    PREFIX ${CMAKE_BINARY_DIR}/vendor/fmt
    GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
    GIT_TAG "7.0.1"
    TIMEOUT 10
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(fmt source_dir)
set(FMT_INCLUDE_DIR ${source_dir}/include)
