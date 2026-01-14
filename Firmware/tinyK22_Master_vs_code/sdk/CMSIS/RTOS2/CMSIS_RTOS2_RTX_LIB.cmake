# Add set(CONFIG_USE_CMSIS_RTOS2_RTX_LIB true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source/rtx_lib.c
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Config/RTX_Config.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Source
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Include
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Config
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

  
            if(CONFIG_TOOLCHAIN STREQUAL mdk)
      target_compile_options(${MCUX_SDK_PROJECT_NAME} PUBLIC
              -fno-short-wchar
              -fno-short-enums
            )
      endif()
      
  endif()

      if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm0p)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V6M.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND (CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f))
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V7MF.a
        -Wl,--end-group
    )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL armgcc OR CONFIG_TOOLCHAIN STREQUAL mcux) AND CONFIG_CORE STREQUAL cm0p)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V6M.a
        -Wl,--end-group
    )
    endif()

        if((CONFIG_TOOLCHAIN STREQUAL armgcc OR CONFIG_TOOLCHAIN STREQUAL mcux) AND (CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f))
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V7MF.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL mdk AND CONFIG_CORE STREQUAL cm0p)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/ARM/RTX_V6M.lib
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL mdk AND (CONFIG_CORE STREQUAL cm4f OR CONFIG_CORE STREQUAL cm7f))
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/ARM/RTX_V7MF.lib
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND CONFIG_DSP STREQUAL NO_DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MB.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MM.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMF.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND CONFIG_DSP STREQUAL NO_DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MB.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MM.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMF.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND CONFIG_DSP STREQUAL NO_DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MB.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MM.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMF.a
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MM.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMF.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_TOOLCHAIN STREQUAL iar AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/IAR/RTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MM.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMF.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL gcc AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/GCC/libRTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MM.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMF.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND (CONFIG_FPU STREQUAL SP_FPU OR CONFIG_FPU STREQUAL DP_FPU) AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMFN.a
        -Wl,--end-group
    )
    endif()

        if(CONFIG_COMPILER STREQUAL armclang AND CONFIG_CORE STREQUAL cm33 AND CONFIG_FPU STREQUAL NO_FPU AND CONFIG_DSP STREQUAL DSP AND CONFIG_TRUSTZONE STREQUAL NO_TZ)
    target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE
    -Wl,--start-group
          ${CMAKE_CURRENT_LIST_DIR}/RTX/Library/CLANG/libRTX_V8MMN.a
        -Wl,--end-group
    )
    endif()

  