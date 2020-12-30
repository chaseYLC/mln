# Features
* Cross platform(Linux, Windows)
* using [fmt library](https://github.com/fmtlib/fmt)
* using [spdlog](https://github.com/gabime/spdlog)
* supported json(using jsoncpp and [simdjson](https://github.com/simdjson/simdjson))

# Required
* CMake [>=3.12]
* boost 1.72

# Testing with Docker
Included the "dockerfile" setting. The "docker/build_and_run.sh" script configures the build environment and runs the server. (It takes a lot of time because cmake, boost, etc. are installed on ubuntu.)


