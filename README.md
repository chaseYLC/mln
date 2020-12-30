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


# Prebuilt-Packet Protocol(String-based)
MLN supports custom packet protocols. You can create your own packet by referring to the created packet protocol and packet parser.

Pre-built packet based on strings are prepared in advance. Since json conversion is performed on the string-based protocol, it is recommended to first work on the string-based and then add a separate packet for the performance-sensitive packet.
