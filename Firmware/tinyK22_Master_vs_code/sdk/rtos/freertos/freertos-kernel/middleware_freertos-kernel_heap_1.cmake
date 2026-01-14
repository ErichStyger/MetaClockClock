# Add set(CONFIG_USE_middleware_freertos-kernel_heap_1 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/portable/MemMang/heap_1.c
        )

  

