# Add set(CONFIG_USE_middleware_lvgl_template true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

            add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/lv_conf.h "" middleware_lvgl_template)
          add_config_file(${CMAKE_CURRENT_LIST_DIR}/template/lvgl_support.c "" middleware_lvgl_template)
    
  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/template
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DLV_CONF_INCLUDE_SIMPLE
              )
  
  
  endif()

