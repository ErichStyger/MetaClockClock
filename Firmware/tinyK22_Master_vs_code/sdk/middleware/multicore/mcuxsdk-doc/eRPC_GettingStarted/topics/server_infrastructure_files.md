# Server infrastructure files

The eRPC infrastructure files are located in the following folder:

*<MCUXpressoSDK\_install\_dir\>/middleware/multicore/erpc/erpc\_c*

The **erpc\_c** folder contains files for creating eRPC client and server applications in the *C/C++* language. These files are distributed into subfolders.

-   The **infra** subfolder contains C++ infrastructure code used to build server and client applications.
    -   Four files, `erpc_server.hpp`, `erpc_server.cpp`, `erpc_simple_server.hpp`, and `erpc_simple_server.cpp`, are used for running the eRPC server on the server-side applications. The simple server is currently the only implementation of the server, and its role is to catch client requests, identify and call requested functions, and send data back when requested.
    -   Three files \(`erpc_codec.hpp`, `erpc_basic_codec.hpp`, and `erpc_basic_codec.cpp`\) are used for codecs. Currently, the basic codec is the initial and only implementation of the codecs.
    -   The `erpc_common.hpp` file is used for common eRPC definitions, typedefs, and enums.
    -   The `erpc_manually_constructed.hpp` file is used for allocating static storage for the used objects.
    -   Message buffer files are used for storing serialized data: `erpc_message_buffer.h` and `erpc_message_buffer.cpp`.
    -   The `erpc_transport.h` file defines the abstract interface for transport layer.
-   The **port** subfolder contains the eRPC porting layer to adapt to different environments.
    -   `erpc_port.h` file contains definition of `erpc_malloc()` and `erpc_free()` functions.
    -   `erpc_port_stdlib.cpp` file ensures adaptation to stdlib.
    -   `erpc_config_internal.h` internal erpc configuration file.
-   The **setup** subfolder contains a set of plain C APIs that wrap the C++ infrastructure, providing client and server init and deinit routines that greatly simplify eRPC usage in C-based projects. No knowledge of C++ is required to use these APIs.
    -   The `erpc_server_setup.h` and `erpc_server_setup.cpp` files needs to be added into the “Matrix multiply” example project to demonstrate the use of C-wrapped functions in this example.
    -   The `erpc_transport_setup.h` and `erpc_setup_rpmsg_lite_remote.cpp` files needs to be added into the project in order to allow the C-wrapped function for transport layer setup.
    -   The `erpc_mbf_setup.h` and `erpc_setup_mbf_rpmsg.cpp` files needs to be added into the project in order to allow message buffer factory usage.
-   The **transports** subfolder contains transport classes for the different methods of communication supported by eRPC. Some transports are applicable only to host PCs, while others are applicable only to embedded or multicore systems. Most transports have corresponding client and server setup functions in the setup folder.
    -   RPMsg-Lite is used as the transport layer for the communication between cores, `erpc_rpmsg_lite_base_transport.hpp`, `erpc_rpmsg_lite_transport.hpp`, and `erpc_rpmsg_lite_transport.cpp` files need to be added into the server project.

|![](../images/server_infrastructure_files.jpg "Server infrastructure files")

|

**Parent topic:**[Multicore server application](../topics/multicore_server_application.md)

