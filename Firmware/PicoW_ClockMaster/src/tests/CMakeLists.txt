# file: Collect all files that need to be compiled. 
# You can use a GLOB function as shown here, or explicitly mention the specific files
#file(GLOB FILES *.c *.h)

# Need to include CTest on every CMakeLists.txt which is going to use tests
include(CTest)

set(THIS_LIBRARY_NAME srcTestsLib)

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
  PRIVATE unityLib
  PRIVATE srcLib
  PRIVATE pico_cyw43_arch_lwip_sys_freertos 
)

# target_include_directories: Libraries need to publish their header files 
# so that you can import them in source code. This statement expresses where to find the files 
# - typically in an include directory of your projects.
target_include_directories(
  ${THIS_LIBRARY_NAME}
  PUBLIC
  .
)
#################################################################################
if (DEFINED ENV{PICO_SEGGER_IP})
  # Set PICO_SEGGER_IP to something linke "192.168.65.254" or "tunnel:801048148"
  # For using an IP, start on the host 'JLinkRemoteServerCL' (no arguments needed) and use its IP
  # For using a tunnel, you have to ron the server on thost, e.g. for tunneling:
  #   JLinkRemoteServerCL -UseTunnel -TunnelBySN -select="801048148"
  # If using a DevContainer, run the following on the host:
  #   JLinkRemoteServerCLExe (Linux host) or JLinkRemoteServerCL (Windows host)
  # If setting PICO_SEGGER_IP in the DevContainer, make sure you re-configure the build system;
  #   export PICO_SEGGER_IP=192.168.65.254
  #  check with printenv PICO_SEGGER_IP
  #   cmake --preset Test
  #   ctest --verbose --test-dir build/Test -R Led
  message(STATUS "JRun: PICO_SEGGER_IP is set to $ENV{PICO_SEGGER_IP}")
  set (JRUN_CTEST_COMMAND JRun --verbose --device RP2040_M0_0 --rtt -if SWD --pc off --sp off --ip "$ENV{PICO_SEGGER_IP}")
else()
  # Connect directly to the probe with USB
  message(STATUS "JRun: PICO_SEGGER_IP is not set, using USB connection")
  set (JRUN_CTEST_COMMAND JRun --verbose --device RP2040_M0_0 --rtt -if SWD --pc off --sp off)
endif()

add_test(
  NAME Led
  COMMAND ${JRUN_CTEST_COMMAND} --args "led" ${TEST_EXECUTABLE}
)

add_test(
  NAME Sensor 
  COMMAND ${JRUN_CTEST_COMMAND} --args "sensor" ${TEST_EXECUTABLE}
)

add_test(
  NAME DNS
  COMMAND ${JRUN_CTEST_COMMAND} --args "dns" ${TEST_EXECUTABLE}
)

set_tests_properties(
  Led Sensor DNS 
  PROPERTIES 
  TIMEOUT 15
) 
