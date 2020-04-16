cmake_minimum_required(VERSION 3.5)

project(ImageAnalysis_Proj)

set(SRCS
    main.cpp
    
    analysis/blobAnalysis.cpp
    analysis/conversion.cpp
    analysis/doubleThreshold.cpp
    analysis/filtering.cpp
    analysis/morphology.cpp
    analysis/posterization.cpp
    )

set(HEADERS
    analysis/blobAnalysis.hpp
    analysis/conversion.hpp
    analysis/doubleThreshold.hpp
    analysis/filtering.hpp
    analysis/morphology.hpp
    analysis/posterization.hpp
    )

add_executable(ImageAnalysisTest ${SRCS} ${HEADERS})

## If you want to link SFML statically
# set(SFML_STATIC_LIBRARIES TRUE)

## In most cases better set in the CMake cache
# set(SFML_DIR "<sfml root prefix>/lib/cmake/SFML")
set(SFML_DIR "C:/SFML-2.5.1/lib/cmake/SFML")

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

target_link_libraries(ImageAnalysisTest sfml-graphics)
