# Add set(CONFIG_USE_middleware_fmstr true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_appcmd.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_pipes.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_protocol.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_rec.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_scope.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_tsa.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_ures.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_utils.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_can.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_sha.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_pdbdm.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_serial.c
          ${CMAKE_CURRENT_LIST_DIR}/src/common/freemaster_net.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/src/common
        )

  
