# Operating systems project
A multithreaded server-client implementation for Operating Systems project 

## Project choices

### Support list and concurrency management

As support data structure, the server uses a linked list, in which a client is inserted at connection time and removed when the thread is closed or in case of
client disconnection anomaly during operations. The list is being used such that if a client tries to connect with a name already present, the connection is denied.
Each function in the *queue.h* library uses a mutual exclusion variable which is locked at the beginning of each function and unlocked at the end. This ensures that every time a function is called on the list, modification takes place in mutual exclusion. The functions of queue.h are:
* *start_queue*: initialize the list;
* *check_queue*: used by *push_queue* to check the list.
* *push_queue*: call *check_queue* and add a client if and only if this is not already present in the list;
* *pop_queue*: remove a client from the list;
* *destroy_queue*: garbage collection of allocated memory of the list;

### Signal handling

The server uses a handler of the **SIGINT**, **SIGTERM**, **SIGQUIT**, **SIGTSTP** and **SIGUSR1** signals to be able to change default behavior. In case of SIGUSR1 the variable *usr* is set and the server's main thread prints the following information:
* number of clients currently connected (*active_number*);
* number of objects in the objstore (*number_objects_store*);
* total size of the objstore (*size_total_store*);
For the last one, a *get_size* support function is used that recursively visits the directory data and add the size of each file (obtained with *fstat* and *st_size*) to the global variable *size_total_store*. At the end, *size_total_store* and *usr* are reset and server operation continues.

## Server management

### Main behavior

The server's main thread creates the socket, the objstore data folder and the signal mask and puts on hold listening to request. Upon acceptance of a connection, a new thread starts and get assigned that client, if *usr = 1* server info are printed. Each thread takes as argument, the file descriptor returned by accept. Then is called the *pthread_detach*, so that all occupied resources are freed when the thread is closed. Each worker thread that is launched performs the operations requested by the client. The condition of termination of a thread is the variable terminated in the case of the arrival of a signal and the variable *r* which is set to 0 in case of *LEAVE* or -1 in case of errors in the system calls. Each thread performs a byte-by-byte read up to delimeter *\n* to get the client's request. Based on client request, the thread performs the appropriate case:
* *REGISTER*: if the client is already present in the list of connected, the thread sends a *KO* message to the client and they both end. Otherwise the client connects and the global variable *active_number* is incremented. In particular, if it is the first connection, also the client personal directory is created;
* *STORE* case: reads a blank character and then continue reading the data from the client, whose length is contained in the previous message. If saving the data terminate successfully, sends an *OK* message to the client, otherwise it sends a *KO* message;
* *RETRIEVE* case: search for the data requested by the client in his personal directory. When the data is successfully recovered sends a message containing *DATA "length of the data" \n actual data*, otherwise it sends a *KO*;
* *DELETE* case: remove a piece of data from the objstore. Send an *OK* message to the client if the data is
successfully removed, otherwise *KO*;
* *LEAVE* case: in any case it sends an *OK* message to the client and execute the disconnection routine (removing the client from the queue, decreasing the *number_active* variable, closing the file descriptor).
In case of errors during reading or writing the result is 0 if the client has inadvertently disconnected, then the disconnection routine is performed (remove the client from the queue, decrement of the variable *active_number*, closing the file descriptor). The global variables *number_active* and *number_store_objects* are modified in mutual exclusion.

### Server library

The worker thread uses a *server_library.h* library to perform operations on the objstore:
* *register_new_customer*: in *REGISTER* case, is called to create the client directory (passed as parameter) in case of first access. Returns 1 if the directory was created, 0 if the directory already exists;
* *save_customer_data*: is called in *STORE* case, access the client directory (passed as a parameter) to save a new piece of data in the objstore. If data was already present, the old data is deleted and the new one is saved. Returns 1 if the data was saved correctly, -1 if it was not possible to save it;
* *retrieve_customer_data*: is called in *RETRIEVE* case, access the client directory (passed as a parameter) and retrieve the requested data. Returns NULL in case of nonexisting data, otherwise a pointer to it;
* *delete_customer_data*: it is called in *DELETE* case, access the directory client (passed as a parameter) to delete a piece of data. Returns 1 if the data was
deleted successfully, 0 if the data is not present.
Each library function returns -2 on system call errors. The thread in this case *KO* the client and continue to handle subsequent requests.

## Client

Client take as arguments its name and the battery tests to run. In the first battery 20 stores are performed, in the second a retrieve, in the third a delete.
Before running battery of tests, everyone connect to the server and disconnect when operations are finished. Before terminating, print all the information on the performed operations: number of operations performed, number of successful operations, number of failed operations. The client library contains a global integer representing the socket to which it connects. Respecting the protocol described in the text:
* *os_connect*: connect a client via socket and connect, the client sends the request and gets connected with eventually folder creation, returns true (1) on success. If it fails, because already connected, it is immediately terminated by returning false (0);
* *os_store*: send request to save a piece of data, returns true (1) in case of success, 0 otherwise;
* *os_retrieve*: send request to the server, and read response in two separate steps: first
a read up to the *\n* of *DATA "len" \n*, then knowing *len* value read the piece of data. Returns true (1) if the data retrieval was successful, false (0) if it receives a *KO*;
* *os_delete*: send request to the server. Returns true (1) if the deletion terminate successfully, false (0) otherwise;
* *os_disconnect*: sends request to the server and read the response. Returns true (1).
In all functions in case of errors in read or write, if the result is -1 or 0 the client is immediately terminated.

## Further details

The shared library *util.h* is used by both server and client. Contains the *Malloc* function
(malloc with error checking) and various definitions useful for both.

The *test.sh* test battery launch 50 clients instances running type 1 tests (20 stores), after that launch 50 clients instances of which 30 perform type 2 test (retrieve) and 20 that perform type test 3 (delete).
The *testsum.sh* script print the sum of all operations performed on stdout, all successful and unsuccessful operations. Finally, if errors occur in some tests, the test itself and the client name are printed, if connection errors occur only name is printed.

Simply clone the repo by using:
```bash
git clone https://github.com/matteodefra/server
```
and compile and launch test by using *Makefile*
```bash
make all
make test
```
