# Add set(CONFIG_USE_middleware_multicore_erpc_eRPC_lpi2c_slave_transport true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/transports/erpc_lpi2c_slave_transport.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup/erpc_setup_mbf_dynamic.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_framed_transport.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup/erpc_setup_lpi2c_slave.cpp
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/transports
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra
        )

  
