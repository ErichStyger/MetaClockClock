# Using the eRPC generator tool

|Windows OS|<MCUXpressoSDK\_install\_dir\>/middleware/multicore/tools/erpcgen/Windows|
|Linux OS|<MCUXpressoSDK\_install\_dir\>/middleware/multicore/tools/erpcgen/Linux\_x64

 <MCUXpressoSDK\_install\_dir\>/middleware/multicore/tools/erpcgen/Linux\_x86

|
|Mac OS|<MCUXpressoSDK\_install\_dir\>/middleware/multicore/tools/erpcgen/Mac|

The files for the “Matrix multiply” example are pre-generated and already a part of the application projects. The following section describes how they have been created.

-   The easiest way to create the shim code is to copy the erpcgen application to the same folder where the IDL file \(`*.erpc`\) is located; then run the following command:

    `erpcgen <IDL_file>.erpc`

-   In the “Matrix multiply” example, the command should look like:

    `erpcgen erpc_matrix_multiply.erpc`


Additionally, another method to create the shim code is to execute the eRPC application using input commands:

-   “-?”/”—help” – Shows supported commands.
-   “-o <filePath\>”/”—output<filePath\>” – Sets the output directory.

For example,

```
<path_to_erpcgen>/erpcgen –o <path_to_output>
<path_to_IDL>/<IDL_file_name>.erpc
```

For the “Matrix multiply” example, when the command is executed from the default `erpcgen` location, it looks like:

`erpcgen –o`

*../../../../../boards/evkmimxrt1170/multicore\_examples/erpc\_common/erpc\_matrix\_multiply/service ../../../../../boards/evkmimxrt1170/multicore\_examples/erpc\_common/erpc\_matrix\_multiply/service/erpc\_matrix\_multiply.erpc*

In both cases, the following four files are generated into the *<MCUXpressoSDK\_install\_dir\>/boards/evkmimxrt1170/multicore\_examples/erpc\_common/erpc\_matrix\_multiply/service folder.*

-   `erpc_matrix_multiply.h`
-   `erpc_matrix_multiply_client.cpp`
-   `erpc_matrix_multiply_server.h`
-   `erpc_matrix_multiply_server.cpp`

For multiprocessor examples, the eRPC file and pre-generated files can be found in the *<MCUXpressoSDK\_install\_dir\>/boards/<board\_name\>/multiprocessor\_examples/erpc\_common/erpc\_matrix\_multiply/service* folder.

**For Linux OS users:**

-   Do not forget to set the permissions for the eRPC generator application.
-   Run the application as `./erpcgen`… instead of as erpcgen ….

**Parent topic:**[eRPC example](../topics/erpc_example.md)

