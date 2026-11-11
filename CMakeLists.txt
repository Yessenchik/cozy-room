cmake_minimum_required(VERSION 3.16)
project(CozyRoom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW (window + input) and GLM (math) are downloaded automatically by CMake
# the first time you configure, so nobody has to install them by hand.
include(FetchContent)

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1)

FetchContent_MakeAvailable(glfw glm)

# GLAD (OpenGL function loader) is generated once and kept in external/glad.
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(CozyRoom ${SOURCES})
# stb_easy_font (one header, public domain) draws the on-screen text.
target_include_directories(CozyRoom PRIVATE src external/stb)
target_link_libraries(CozyRoom PRIVATE glfw glm::glm glad)

# Path to the project folder, so the program finds shaders/ and textures/
# no matter where it is started from (VS Code, Xcode or terminal).
target_compile_definitions(CozyRoom PRIVATE PROJECT_DIR="${CMAKE_SOURCE_DIR}/")
