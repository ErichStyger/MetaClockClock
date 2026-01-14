# Multiprocessor server application

The “Matrix multiply” eRPC server project for multiprocessor applications is located in the *<MCUXpressoSDK\_install\_dir\>\>/boards/<board\_name\>/multiprocessor\_examples/ erpc\_server\_matrix\_multiply\_<transport\_layer\>* folder.

Most of the multiprocessor application setup is the same as for the multicore application. The multiprocessor server application requires server-related generated files \(server shim code\), server infrastructure files, and the server user code. There is no need for server multicore infrastructure files \(MCMGR and RPMsg-Lite\). The RPMsg-Lite transport layer is replaced either by SPI or UART transports. The following table shows the required transport-related files per each transport type.

|SPI|`<eRPC base directory>/erpc_c/setup/erpc_setup_(d)spi_slave.cpp`

 `<eRPC base directory>/erpc_c/transports/erpc_(d)spi_slave_transport.hpp`

 `<eRPC base directory>/erpc_c/transports/erpc_(d)spi_slave_transport.cpp`

|
|UART|`<eRPC base directory>/erpc_c/setup/erpc_setup_uart_cmsis.cpp`

 `<eRPC base directory>/erpc_c/transports/erpc_uart_cmsis_transport.hpp`

 `<eRPC base directory>/erpc_c/transports/erpc_uart_cmsis_transport.cpp`

|


```{include} ../topics/server_user_code.md
:heading-offset: 3
```

```{include} ../topics/multiprocessor_client_application.md
:heading-offset: 3
```

**Parent topic:**[Create an eRPC application](../topics/create_an_erpc_application.md)

