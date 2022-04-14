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

SET(OPENCV_DIR
  "E:/kpdf/pdfreader_story/image/opencv-4.5.3"
)
SET(OPENCL_DIR
  "E:/kSource/pythonx/shader/opengl-3rd/OpenCL-SDK-v2022.04.01-Win-x86"
)
SET(JPEG_DIR
  "E:/kSource/pythonx/shader/opengl-3rd/libjpeg-turbo"
)
SET(CUDA_DIR
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

#https://www.cnblogs.com/zjutzz/p/13340318.html
#https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
#set_property(TARGET resizer PROPERTY
#    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#set_target_properties(resizer PROPERTIES
#    CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
#)

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

if (APPLE)
  target_link_libraries(resizer "-framework OpenCL")
  target_link_libraries(resizer /opt/libjpeg-turbo/lib64/libturbojpeg.dylib)
  target_link_libraries(resizer /usr/local/Frameworks/opencv2.framework)
endif (APPLE)

#windows specific
if (WIN32)
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:Debug>:"
            "${OPENCV_DIR}/lib/Debug/opencv_core453d.lib"
            "${OPENCV_DIR}/lib/Debug/opencv_imgproc453d.lib"
            "${OPENCV_DIR}/lib/Debug/opencv_highgui453d.lib"
            "${OPENCV_DIR}/lib/Debug/opencv_imgcodecs453d.lib"
        ">"
        "$<$<NOT:$<CONFIG:Debug>>:"
            "${OPENCV_DIR}/lib/Release/opencv_core453.lib"
            "${OPENCV_DIR}/lib/Release/opencv_imgproc453.lib"
            "${OPENCV_DIR}/lib/Release/opencv_highgui453.lib"
            "${OPENCV_DIR}/lib/Release/opencv_imgcodecs453.lib"
        ">"
    )

  target_link_libraries(resizer "${JPEG_DIR}/lib/turbojpeg-static.lib")
  #target_link_libraries(resizer "${CUDA_DIR}/lib/x64/OpenCL.lib")
  target_link_libraries(resizer "${OPENCL_DIR}/lib/OpenCL.lib")
  target_link_libraries(resizer "${ADDITIONAL_LIBRARY_DEPENDENCIES}")
endif (WIN32)

SET(RUNDIR
    "$<$<CONFIG:Debug>:" "${PROJECT_BINARY_DIR}/Debug" ">"
    "$<$<NOT:$<CONFIG:Debug>>:" "${PROJECT_BINARY_DIR}/Release" ">"
)
string(REPLACE ";" "" RUNDIR ${RUNDIR})
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${OPENCL_DIR}/bin/OpenCL.dll
        ${RUNDIR}/OpenCL.dll)

#https://thomas.trocha.com/blog/cmake--copy-files-after-build/
foreach(opencvmodule core imgproc highgui imgcodecs)
    set(COPYFROM  
        "$<$<CONFIG:Debug>:" "${OPENCV_DIR}/bin/Debug/opencv_${opencvmodule}453d.dll" ">"
        "$<$<NOT:$<CONFIG:Debug>>:" "${OPENCV_DIR}/bin/Release/opencv_${opencvmodule}453.dll" ">")
    SET(COPYTO   
        "$<$<CONFIG:Debug>:" "${PROJECT_BINARY_DIR}/Debug/opencv_${opencvmodule}453d.dll" ">"        
        "$<$<NOT:$<CONFIG:Debug>>:" "${PROJECT_BINARY_DIR}/Release/opencv_${opencvmodule}453.dll" ">")

    string(REPLACE ";" "" COPYFROM ${COPYFROM})
    string(REPLACE ";" "" COPYTO ${COPYTO})

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${COPYFROM}" "${COPYTO}"
)
endforeach(opencvmodule)
