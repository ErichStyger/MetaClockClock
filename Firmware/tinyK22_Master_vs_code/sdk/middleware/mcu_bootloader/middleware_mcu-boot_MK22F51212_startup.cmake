# Add set(CONFIG_USE_middleware_mcu-boot_MK22F51212_startup true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/../../devices/Kinetis/K/MK22F51212/system_MK22F51212.c
        )

        if(CONFIG_TOOLCHAIN STREQUAL iar)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/startup/crt0.s
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/startup/iar/startup_MK22F51212.s
        )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL mcux OR CONFIG_TOOLCHAIN STREQUAL armgcc))
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/startup/crt0_gcc.S
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/startup/gcc/startup_MK22F51212.S
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL mdk)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/targets/MK22F51212/src/startup/arm/startup_MK22F51212.S
        )
    endif()

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/../../devices/MK22F51212
        )

  
