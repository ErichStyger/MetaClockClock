# Add set(CONFIG_USE_middleware_lvgl_demo_widgets true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_widgets.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_clothes.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_demo_widgets_avatar.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_demo_widgets_needle.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_lvgl_logo.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
          ${CMAKE_CURRENT_LIST_DIR}/assets
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DLV_CONF_INCLUDE_SIMPLE
              )
  
  
  endif()

