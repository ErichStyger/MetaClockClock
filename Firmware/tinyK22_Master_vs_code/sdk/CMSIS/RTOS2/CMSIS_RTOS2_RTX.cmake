# Add set(CONFIG_USE_CMSIS_RTOS2_RTX true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_delay.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_evflags.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_evr.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_kernel.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_memory.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_mempool.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_msgqueue.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_mutex.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_semaphore.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_system.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_thread.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_timer.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_lib.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Config/RTX_Config.c
        )

        if((CONFIG_CORE STREQUAL cm0p OR CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f OR CONFIG_CORE STREQUAL cm33))
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/Source/os_systick.c
        )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL mdk OR CONFIG_TOOLCHAIN STREQUAL armgcc OR CONFIG_TOOLCHAIN STREQUAL mcux) AND CONFIG_CORE STREQUAL cm0p)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/GCC/irq_armv6m.S
        )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL mdk OR CONFIG_TOOLCHAIN STREQUAL armgcc OR CONFIG_TOOLCHAIN STREQUAL mcux) AND (CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f))
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/GCC/irq_armv7m.S
        )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL mdk OR CONFIG_TOOLCHAIN STREQUAL armgcc OR CONFIG_TOOLCHAIN STREQUAL mcux) AND CONFIG_CORE STREQUAL cm33)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/GCC/irq_armv8mml.S
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm0p)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/IAR/irq_armv6m.s
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND (CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f))
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/IAR/irq_armv7m.s
        )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/IAR/irq_armv8mml.s
        )
    endif()

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Include
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Config
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DCMSIS_device_header="fsl_device_registers.h"
                        -DRTE_COMPONENTS_H
              )
  
  
  endif()

