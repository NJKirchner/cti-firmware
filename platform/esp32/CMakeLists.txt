cmake_minimum_required(VERSION 3.20)

set(CTI_PLATFORM esp32 CACHE STRING "" FORCE)
set(CTI_BOARD wishio-lora-915 CACHE STRING "")

if(NOT DEFINED ENV{IDF_PATH})
    message(FATAL_ERROR "IDF_PATH must point to an ESP-IDF checkout")
endif()

set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(CtiFirmware)
