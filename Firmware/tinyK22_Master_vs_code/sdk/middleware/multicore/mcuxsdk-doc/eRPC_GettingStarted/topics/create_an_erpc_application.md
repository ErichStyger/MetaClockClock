# Create an eRPC application

This section describes a generic way to create a client/server eRPC application:

1.  **Design the eRPC application:** Decide which data types are sent between applications, and define functions that send/receive this data.
2.  **Create the IDL file:** The IDL file contains information about data types and functions used in an eRPC application, and is written in the IDL language.
3.  **Use the eRPC generator tool:** This tool takes an IDL file and generates the shim code for the client and the server-side applications.
4.  **Create an eRPC application:**
    1.  Create two projects, where one project is for the client side \(primary core\) and the other project is for the server side \(secondary core\).
    2.  Add generated files for the client application to the client project, and add generated files for the server application to the server project.
    3.  Add infrastructure files.
    4.  Add user code for client and server applications.
    5.  Set the client and server project options.
5.  **Run the eRPC application:** Run both the server and the client applications. Make sure that the server has been run before the client request was sent.

A specific example follows in the next section.


```{include} ../topics/multicore_server_application.md
:heading-offset: 2
```

```{include} ../topics/multicore_client_application.md
:heading-offset: 2
```

```{include} ../topics/multiprocessor_server_application.md
:heading-offset: 2
```

```{include} ../topics/running_the_erpc_application.md
:heading-offset: 2
```

**Parent topic:**[eRPC example](../topics/erpc_example.md)

