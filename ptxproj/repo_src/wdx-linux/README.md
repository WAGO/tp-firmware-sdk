# Linux specific libraries for WDx (wdx-linux)

All contents of this project is WDx related but mostly specific to Linux because
it relies on specific technologies such as UNIX domain sockets.

## Libaries

All libaries are meant to be used on PFC like platforms to enable inter-process
communication between the parameter service daemon (paramd) and other processes,
usually containing parameter providers. 

### `libwdxlinuxosfile.a`

This library provides utility functions to develop file providers for arbitrary
files from disk.

### `libwdxlinuxoscom.so`

The WDx **com**munication library is used to connect processes to the frontend
and backend interfaces of the parameter service daemon.

The library aims to provide an easy-to-use API for establishing and maitaining
one or more IPC connections between the parameter service daemon and several
client processes. Use this library from within your project to register parameter
providers or use the parameter service frontend functions by using the
corresponding proxy/stub components.

#### WDx communication library general usage

To use the com library include the header files from "*inc/wago/wdx/linuxos/*"
for the functionality you wish to use. Important headers are:

<pre>
- com/parameter_service_backend_proxy.hpp
	-> Remote backend service component<br/>
- com/exception.hpp
	-> Base exception for communication library<br/>
- com/driver_i.hpp
	-> Driver interface to run library actions<br/>
- com/notification_manager_i.hpp
	-> Management interface to add/remove notifier for established connections<br/>

</pre>

See generated documentation (target "doc") for a more detailed description.

#### WDx communication library security aspects

To use this library a client have to be able to open the (Unix domain) socket,
which is part of the currently used IPC solution, to communicate with a
Parameter Service server process.
The default path for this socket is shipped with the library implementation,
so normally a client does not have to know the currently used socket path.
But for security reasons the client have either to be root or member of a Unix
group corresponding to the socket that should be used:
- "wdxprov" for backend socket
- "wdaadm" for frontend socket

### `libwdxlinuxosclient.a`

This library aims to provide all functionality to create a straight forward
client process giving (parameter) providers a home to run.
The library defines a main function running a parameter service backend proxy
by utilizing libwdxlinuxoscom.so. If you use this library you only have to
implement the public client abstract class by initializing the application with
your provider instances.

### `libwdxlinuxosserv.a`

The **serv**ice library supports creating of a Parameter Service daemon and
connecting it to a webserver on Linux based systems.

### `paramd`

The WAGO Parameter Service daemon for Linux based systems.

## Unit tests

In subdirectories "*test-inc*" and "*test-src*" are unit test sources provided.
To skip compilation of test targets the CMake option WITHOUT_TEST may be set.
