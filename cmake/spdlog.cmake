find_program(GIT_EXECUTABLE git)

ExternalProject_Add(
    spdlog
    PREFIX ${CMAKE_BINARY_DIR}/vendor/spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG "v1.7.0"
    TIMEOUT 10
    CONFIGURE_COMMAND ""
    CMAKE_ARGS "-DSPDLOG_FMT_EXTERNAL_HO=ON"
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

ExternalProject_Get_Property(spdlog source_dir)
set(SPDLOG_INCLUDE_DIR ${source_dir}/include)
