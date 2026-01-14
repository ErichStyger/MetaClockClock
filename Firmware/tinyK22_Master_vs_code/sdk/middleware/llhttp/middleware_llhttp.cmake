# Add set(CONFIG_USE_middleware_llhttp true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/src/api.c
          ${CMAKE_CURRENT_LIST_DIR}/src/http.c
          ${CMAKE_CURRENT_LIST_DIR}/src/llhttp.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/include
        )

  
