# Add set(CONFIG_USE_middleware_multicore_erpc_arbitrator true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_arbitrated_client_manager.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_client_manager.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_simple_server.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_server.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra/erpc_transport_arbitrator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup/erpc_arbitrated_client_setup.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup/erpc_server_setup.cpp
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/port/erpc_threading_freertos.cpp
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/infra
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/setup
          ${CMAKE_CURRENT_LIST_DIR}/erpc/erpc_c/port
        )

  
