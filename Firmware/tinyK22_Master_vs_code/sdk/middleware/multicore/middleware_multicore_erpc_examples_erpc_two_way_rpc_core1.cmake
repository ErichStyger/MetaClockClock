# Add set(CONFIG_USE_middleware_multicore_erpc_examples_erpc_two_way_rpc_core1 true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_client.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core1Interface_client.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/erpc_two_way_rpc_Core1Interface_interface.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_server.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/c_erpc_two_way_rpc_Core0Interface_server.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service/erpc_two_way_rpc_Core0Interface_interface.cpp
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/erpc/examples/MCUXPRESSO_SDK/erpc_two_way_rpc/service
        )

  
