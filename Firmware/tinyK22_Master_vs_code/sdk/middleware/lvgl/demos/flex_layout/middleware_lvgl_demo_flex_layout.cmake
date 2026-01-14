# Add set(CONFIG_USE_middleware_lvgl_demo_flex_layout true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_ctrl_pad.c
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_flex_loader.c
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_main.c
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_view.c
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_view_child_node.c
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_flex_layout_view_ctrl_pad.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/.
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DLV_CONF_INCLUDE_SIMPLE
              )
  
  
  endif()

