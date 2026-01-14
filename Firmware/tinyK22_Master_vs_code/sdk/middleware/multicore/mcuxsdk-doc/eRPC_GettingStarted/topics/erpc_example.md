# eRPC example

This section shows how to create an example eRPC application called “Matrix multiply”, which implements one eRPC function \(matrix multiply\) with two function parameters \(two matrices\). The client-side application calls this eRPC function, and the server side performs the multiplication of received matrices. The server side then returns the result.

For example, use the NXP MIMXRT1170-EVK board as the target dual-core platform, and the IAR Embedded Workbench for ARM \(EWARM\) as the target IDE for developing the eRPC example.

-   The primary core \(CM7\) runs the eRPC client.
-   The secondary core \(CM4\) runs the eRPC server.
-   RPMsg-Lite \(Remote Processor Messaging Lite\) is used as the eRPC transport layer.

The “Matrix multiply” application can be also run in the multi-processor setup. In other words, the eRPC client running on one SoC comunicates with the eRPC server that runs on anothe SoC, utilizing different transport channels. It is possible to run the board-to-PC example \(PC as the eRPC server and a board as the eRPC client, and vice versa\) and also the board-to-board example. These multiprocessor examples are prepared for selected boards only.

|Multicore application source and project files|*<MCUXpressoSDK\_install\_dir\>/boards/evkmimxrt1170/multicore\_examples/erpc\_matrix\_multiply\_rpmsg/*|
|Multiprocessor application source and project files|*<MCUXpressoSDK\_install\_dir\>/boards/<board\_name\>/multiprocessor\_examples/erpc\_client\_matrix\_multiply\_<transport\_layer\>/*

 *<MCUXpressoSDK\_install\_dir\>/boards/<board\_name\>/multiprocessor\_examples/erpc\_server\_matrix\_multiply\_<transport\_layer\>/*

|
|eRPC source files|*<MCUXpressoSDK\_install\_dir\>/middleware/multicore/erpc/*|
|RPMsg-Lite source files|*<MCUXpressoSDK\_install\_dir\>/middleware/multicore/rpmsg\_lite/*|


```{include} ../topics/designing_the_erpc_application.md
:heading-offset: 1
```

```{include} ../topics/creating_the_idl_file.md
:heading-offset: 1
```

```{include} ../topics/using_the_erpc_generator_tool.md
:heading-offset: 1
```

```{include} ../topics/create_an_erpc_application.md
:heading-offset: 1
```

