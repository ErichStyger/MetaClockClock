# Add set(CONFIG_USE_component_serial_manager_spi true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/fsl_component_serial_port_spi.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DSERIAL_PORT_TYPE_SPI=1
                        -DSERIAL_PORT_TYPE_SPI_MASTER=1
                        -DSERIAL_PORT_TYPE_SPI_SLAVE=1
                        -DSERIAL_MANAGER_NON_BLOCKING_MODE=1
              )
  
  
  endif()

