cmake_minimum_required(VERSION 3.14)
project(resizer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )

include_directories(.)
include_directories(OpenCL)
include_directories(OpenCL/kernels)

#cmake .. -G "Visual Studio 16 2019" -A Win32
if (APPLE)
  include_directories(/opt/libjpeg-turbo/include)
  include_directories(/usr/local/Frameworks/opencv2.framework)
endif (APPLE)

SET(
  OPENCV_DIR
  "E:/kpdf/pdfreader_story/image/opencv-4.5.3"
)
SET(
  OPENCL_DIR
  "E:/kSource/pythonx/shader/opengl-3rd/OpenCL-SDK-v2022.04.01-Win-x86"
)
SET(
  JPEG_DIR
  "E:/kSource/pythonx/shader/opengl-3rd/libjpeg-turbo"
)
SET(
  CUDA_DIR
  "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.0"
)

#windows specific
if (WIN32)
  include_directories("${JPEG_DIR}/include")
  #include_directories("${CUDA_DIR}/include")
  include_directories("${OPENCV_DIR}/modules/core/include")
  include_directories("${OPENCV_DIR}/modules/imgproc/include")
  include_directories("${OPENCV_DIR}/modules/highgui/include")
  include_directories("${OPENCV_DIR}/modules/imgcodecs/include")
  include_directories("${OPENCV_DIR}/build")
  include_directories("${OPENCL_DIR}/include")
endif (WIN32)

add_executable(resizer
    OpenCL/oclManager.cpp
    OpenCL/oclManager.h
    Resizer.cpp
    Resizer.h
    Image.cpp
    Image.h
    IO.cpp
    IO.h
    JPEGImage.cpp
    JPEGImage.h
    main.cpp
    Profiler.h
    Utils.cpp
    Utils.h)

#https://www.cnblogs.com/zjutzz/p/13340318.html
set_property(TARGET resizer PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


if (APPLE)
  target_link_libraries(resizer "-framework OpenCL")
  target_link_libraries(resizer /opt/libjpeg-turbo/lib64/libturbojpeg.dylib)
  target_link_libraries(resizer /usr/local/Frameworks/opencv2.framework)
endif (APPLE)

if (NOT CMAKE_BUILD_TYPE) 
   set(CMAKE_BUILD_TYPE "Debug")
endif()

if (CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  set(FTYPE_LIB "d.lib")
  set(FTYPE_DLL "d.dll")
#elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
else()
  set(FTYPE_LIB ".lib")
  set(FTYPE_DLL ".dll")
endif()

#windows specific
if (WIN32)
  target_link_libraries(resizer "${JPEG_DIR}/lib/turbojpeg-static.lib")
  #target_link_libraries(resizer "${CUDA_DIR}/lib/x64/OpenCL.lib")
  target_link_libraries(resizer "${OPENCV_DIR}/lib/${CMAKE_BUILD_TYPE}/opencv_core453${FTYPE_LIB}")
  target_link_libraries(resizer "${OPENCV_DIR}/lib/${CMAKE_BUILD_TYPE}/opencv_imgproc453${FTYPE_LIB}")
  target_link_libraries(resizer "${OPENCV_DIR}/lib/${CMAKE_BUILD_TYPE}/opencv_highgui453${FTYPE_LIB}")
  target_link_libraries(resizer "${OPENCV_DIR}/lib/${CMAKE_BUILD_TYPE}/opencv_imgcodecs453${FTYPE_LIB}")
  target_link_libraries(resizer "${OPENCL_DIR}/lib/OpenCL.lib")
endif (WIN32)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCL_DIR}/bin/OpenCL.dll
        ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/OpenCL.dll)

#https://thomas.trocha.com/blog/cmake--copy-files-after-build/
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_core453${FTYPE_DLL}
        ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/opencv_core453${FTYPE_DLL})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgproc453${FTYPE_DLL}
        ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/opencv_imgproc453${FTYPE_DLL})

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_highgui453${FTYPE_DLL}
        ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/opencv_highgui453${FTYPE_DLL})

 add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCV_DIR}/bin/${CMAKE_BUILD_TYPE}/opencv_imgcodecs453${FTYPE_DLL}
        ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/opencv_imgcodecs453${FTYPE_DLL})
