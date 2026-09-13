# Add executable
add_executable(${ProjectName}
    main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/ssd1306_i2c.c
)

target_include_directories(${ProjectName} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${FREERTOS_KERNEL_PATH}/include
)

target_link_libraries(${ProjectName}
    pico_stdlib
    hardware_i2c
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

pico_add_extra_outputs(${ProjectName})
