cmake_minimum_required(VERSION 3.14)

project(MyQtProject VERSION 1.0 LANGUAGES CXX)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_PREFIX_PATH "/home/hans/Qt/6.8.1/gcc_64/lib/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network OpenGL DataVisualization)

# Collect source and header files
set(SOURCES
    main.cpp
    mainwindow.cpp
)

set(HEADERS
    carrierdetection.hpp
    dsp.hpp
    fft.hpp
    filesink.hpp
    mainwindow.h
    mousegui.hpp
    sonarview.hpp
    libmouse.hpp
    udpsink.hpp
    tools.hpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include Qt headers
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::OpenGL
    Qt6::DataVisualization
)

# Link external libraries
target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/libusb-1.0 /usr/include/volk)
target_link_libraries(${PROJECT_NAME} usb-1.0 volk)

# Optional install rules
install(TARGETS ${PROJECT_NAME} DESTINATION /opt/${PROJECT_NAME}/bin)

