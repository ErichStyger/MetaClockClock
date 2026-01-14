# Designing the eRPC application

The matrix multiply application is based on calling single eRPC function that takes 2 two-dimensional arrays as input and returns matrix multiplication results as another 2 two-dimensional array. The IDL file syntax supports arrays with the dimension length set by the number only \(in the current eRPC implementation\). Because of this, a variable is declared in the IDL dedicated to store information about matrix dimension length, and to allow easy maintenance of the user and server code.

For a simple use of the two-dimensional array, the alias name \(new type definition\) for this data type has is declared in the IDL. Declaring this alias name ensures that the same data type can be used across the client and server applications.

**Parent topic:**[eRPC example](../topics/erpc_example.md)

