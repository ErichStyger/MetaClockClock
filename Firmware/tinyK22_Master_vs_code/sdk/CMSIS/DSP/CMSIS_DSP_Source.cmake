# Add set(CONFIG_USE_CMSIS_DSP_Source true) in config.cmake to use this component

include_guard(GLOBAL)
message("${CMAKE_CURRENT_LIST_FILE} component is included.")

      target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/Source/BasicMathFunctions/BasicMathFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/BasicMathFunctions/BasicMathFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/BayesFunctions/BayesFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/BayesFunctions/BayesFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/CommonTables/CommonTables.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/CommonTables/CommonTablesF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/ComplexMathFunctions/ComplexMathFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/ComplexMathFunctions/ComplexMathFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/ControllerFunctions/ControllerFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/DistanceFunctions/DistanceFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/DistanceFunctions/DistanceFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/FastMathFunctions/FastMathFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/FastMathFunctions/FastMathFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/FilteringFunctions/FilteringFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/FilteringFunctions/FilteringFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/InterpolationFunctions/InterpolationFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/InterpolationFunctions/InterpolationFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/MatrixFunctions/MatrixFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/MatrixFunctions/MatrixFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/QuaternionMathFunctions/QuaternionMathFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/SVMFunctions/SVMFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/SVMFunctions/SVMFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/StatisticsFunctions/StatisticsFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/StatisticsFunctions/StatisticsFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/SupportFunctions/SupportFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/SupportFunctions/SupportFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/TransformFunctions/TransformFunctions.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/TransformFunctions/TransformFunctionsF16.c
          ${CMAKE_CURRENT_LIST_DIR}/Source/WindowFunctions/WindowFunctions.c
        )

  
      target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
          ${CMAKE_CURRENT_LIST_DIR}/Include
          ${CMAKE_CURRENT_LIST_DIR}/PrivateInclude
          ${CMAKE_CURRENT_LIST_DIR}/Source/DistanceFunctions
        )

    if(CONFIG_USE_COMPONENT_CONFIGURATION)
  message("===>Import configuration from ${CMAKE_CURRENT_LIST_FILE}")

      target_compile_definitions(${MCUX_SDK_PROJECT_NAME} PUBLIC
                  -DDISABLEFLOAT16
              )
  
  
  endif()

