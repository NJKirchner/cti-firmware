file(GLOB CTI_SOURCE_FILES
    "${CMAKE_CURRENT_LIST_DIR}/../../../../source/platform/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../source/scpi/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../source/visa/*.cpp"
)

idf_component_register(
    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/../../main.cpp"
        ${CTI_SOURCE_FILES}
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_platform.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_io.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_comms.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_pwm.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_timer.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../platform/esp32_visa.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../../boards/wishio-lora-915/wishio_board.cpp"
    INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/../../../../include"
        "${CMAKE_CURRENT_LIST_DIR}/../../../../include/cti"
        "${CMAKE_CURRENT_LIST_DIR}/../../../../include/visa"
        "${CMAKE_CURRENT_LIST_DIR}/../../../../include/scpi"
        "${CMAKE_CURRENT_LIST_DIR}/../../../../platform/common/include"
        "${CMAKE_CURRENT_LIST_DIR}/../../../../source"
        "${CMAKE_CURRENT_LIST_DIR}/../../boards/wishio-lora-915"
    REQUIRES
        driver
        esp_adc
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_ledc
        esp_driver_spi
        esp_driver_uart
        esp_timer
        esp_system
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    CTI_VISA=1
    _CTI_PLATFORM=esp32
    _CTI_BOARD=wishio_lora_915
    CTI_VER_MAJOR=0
    CTI_VER_MINOR=9
    CTI_VER_PATCH=4
)
