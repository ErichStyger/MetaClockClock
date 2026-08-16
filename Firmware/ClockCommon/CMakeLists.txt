set(THIS_LIBRARY_NAME ClockCommonLib)

file(GLOB FILES
  *.c
)

# add_library: With this declaration, you express the intent to build a library. 
# The first argument is the name of the library, 
# the second argument are the files that will be compiled to create your library.
add_library(${THIS_LIBRARY_NAME} ${FILES})

# target_link_libraries: If you link with other libraries, list them here
target_link_libraries(
  ${THIS_LIBRARY_NAME}
  PRIVATE McuLib
  PRIVATE wifiLib
  PRIVATE tinyusb_device
  
  # PL_CONFIG_USE_HTTPD_SERVER:
  pico_lwip_http
  pico_httpd_content
)

# PL_CONFIG_USE_HTTPD_SERVER:
# generates file containing HTTP declaration
# file is configured in lwipopts.h with HTTPD_FSDATA_FILE
# file is generated as build\Debug\srcLib\ClockCommonLib\generated
# but on Windows the Python script builds it with '\' instead of '_' in variables, thus it is fixed and copied locally
pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(pico_httpd_content INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/content/404.html
  ${CMAKE_CURRENT_LIST_DIR}/content/index.shtml
  ${CMAKE_CURRENT_LIST_DIR}/content/test.shtml
  ${CMAKE_CURRENT_LIST_DIR}/content/ledpass.shtml
  ${CMAKE_CURRENT_LIST_DIR}/content/ledfail.shtml
  ${CMAKE_CURRENT_LIST_DIR}/content/img/rpi.png
)
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/generated/pico_fsdata.inc)
  file(COPY ${CMAKE_CURRENT_BINARY_DIR}/generated/pico_fsdata.inc DESTINATION ${CMAKE_CURRENT_LIST_DIR})  # copy file from the build folder to the src folder
  message(STATUS "copy pico_fsdata.inc to " ${CMAKE_CURRENT_LIST_DIR})
endif()

# target_include_directories: Libraries need to publish their header files 
# so that you can import them in source code. This statement expresses where to find the files 
# - typically in an include directory of your projects.
target_include_directories(
  ${THIS_LIBRARY_NAME}
  PUBLIC
  .
)
