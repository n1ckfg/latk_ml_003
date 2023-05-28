cmake_minimum_required(VERSION 3.1)
project(SynDraw)

set(CMAKE_BUILD_TYPE Release)
set(OPENGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# libigl
option(LIBIGL_WITH_EMBREE            "Use Embree"         ON)
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)
option(LIBIGL_WITH_OPENGL_GLFW_IMGUI "Use ImGui"          ON)  
option(LIBIGL_WITH_PNG               "Use PNG"            ON)        
option(LIBIGL_WITH_VIEWER            "Use OpenGL viewer"  ON)
OPTION(EMBREE_ISA_SSE2               "Enables SSE2 ISA."  ON)

find_package(LIBIGL REQUIRED QUIET)
find_package(OpenMP)
        
if(APPLE)
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.14")
endif()

# Add project files
file(GLOB BASEFILES src/Common/*.cpp)
file(GLOB VIEWGRAPHFILES src/ViewGraph/*.cpp)
file(GLOB CHAINERFILES src/Chainer/*)
file(GLOB CONTOUREXTRACTORFILES src/ContourExtractor/*.cpp)
file(GLOB TOOLSFILES src/Tools/*.cpp)
file(GLOB LIBFILES src/Lib/*.cpp)

set(SOURCES ${BASEFILES} ${CHAINERFILES} ${TOOLSFILES} ${VIEWGRAPHFILES} ${CONTOUREXTRACTORFILES} ${LIBFILES})
set(EMBREE_INCLUDE ${LIBIGL_EXTERNAL}/embree/include)

# create common lib
add_library(app_lib ${SOURCES})
target_include_directories(app_lib PUBLIC ${EMBREE_INCLUDE})
if(OpenMP_FOUND)
    target_link_libraries(app_lib PUBLIC igl::core igl::opengl_glfw embree OpenMP::OpenMP_CXX)
else()
    target_link_libraries(app_lib PUBLIC igl::core igl::opengl_glfw embree)
endif()

# build offline target
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE app_lib)

# build interactive target
if(APPLE)
    add_executable(${PROJECT_NAME}_Viewer src/interactive_main.cpp src/LineViewer.cpp src/FileDialog.mm)
    target_link_libraries(${PROJECT_NAME}_Viewer INTERFACE "-framework Carbon -framework Cocoa")
else()
    add_executable(${PROJECT_NAME}_Viewer src/interactive_main.cpp src/LineViewer.cpp)
endif()

target_link_libraries(${PROJECT_NAME}_Viewer PUBLIC igl::core igl::opengl_glfw igl::opengl_glfw_imgui)
target_link_libraries(${PROJECT_NAME}_Viewer PRIVATE app_lib)

# build batch target
add_executable(${PROJECT_NAME}_Batch src/batch_main.cpp)
target_link_libraries(${PROJECT_NAME}_Batch PRIVATE app_lib)

# build normal target
add_executable(${PROJECT_NAME}_Normal src/normal_main.cpp)
target_link_libraries(${PROJECT_NAME}_Normal PRIVATE app_lib igl_stb_image)

# build shadow target
add_executable(${PROJECT_NAME}_Shadow src/shadow_main.cpp)
target_link_libraries(${PROJECT_NAME}_Shadow PRIVATE app_lib igl_stb_image)