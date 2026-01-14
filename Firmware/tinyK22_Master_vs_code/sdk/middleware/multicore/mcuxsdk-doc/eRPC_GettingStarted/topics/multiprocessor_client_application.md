# Multiprocessor client application

The “Matrix multiply” eRPC client project for multiprocessor applications is located in the *<MCUXpressoSDK\_install\_dir\>/ boards/<board\_name\>/multiprocessor\_examples/erpc\_client\_matrix\_multiply\_<transport\_layer\>/iar/* folder.

Most of the multiprocessor application setup is the same as for the multicore application. The multiprocessor server application requires client-related generated files \(server shim code\), client infrastructure files, and the client user code. There is no need for client multicore infrastructure files \(MCMGR and RPMsg-Lite\). The RPMsg-Lite transport layer is replaced either by SPI or UART transports. The following table shows the required transport-related files per each transport type.

|SPI|`<eRPC base directory>/erpc_c/setup/erpc_setup_(d)spi_master.cpp`

 `<eRPC base directory>/erpc_c/transports/ erpc_(d)spi_master_transport.hpp`

 `<eRPC base directory>/erpc_c/transports/ erpc_(d)spi_master_transport.cpp`

|
|UART|`<eRPC base directory>/erpc_c/setup/erpc_setup_uart_cmsis.cpp`

 `<eRPC base directory>/erpc_c/transports/erpc_uart_cmsis_transport.hpp`

 `<eRPC base directory>/erpc_c/transports/erpc_uart_cmsis_transport.cpp`

|


```{include} ../topics/client_user_code.md
:heading-offset: 4
```

**Parent topic:**[Multiprocessor server application](../topics/multiprocessor_server_application.md)

