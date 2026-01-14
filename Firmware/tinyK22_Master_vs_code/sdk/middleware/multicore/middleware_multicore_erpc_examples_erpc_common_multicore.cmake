# Add set(CONFIG_USE_middleware_multicore_erpc_examples_erpc_common_multicore true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_error_handler.cpp
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK
        )

  
