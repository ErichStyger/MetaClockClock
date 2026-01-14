# Add set(CONFIG_USE_middleware_lvgl_demo_benchmark true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/lv_demo_benchmark.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_benchmark_avatar.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_benchmark_cogwheel_alpha256.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_benchmark_cogwheel_argb.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_benchmark_cogwheel_indexed16.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/img_benchmark_cogwheel_rgb.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/lv_font_benchmark_montserrat_12_compr_az.c.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/lv_font_benchmark_montserrat_16_compr_az.c.c
          ${CMAKE_CURRENT_LIST_DIR}/assets/lv_font_benchmark_montserrat_28_compr_az.c.c
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

