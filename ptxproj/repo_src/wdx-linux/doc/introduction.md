# Introduction to WAGO Parameter Service Communication Library C/C++ API
This is an introduction to the C/C++ APIs. These APIs can be used to request or set different parameters,
provide parameters with one or multiple parameter providers, to manage parameter providers and to connect a
parameter service daemon.
The C++ API is preferred to use, the C API is for compatibility reasons only and limited in functionality.

## 1 WAGO Parameter Service C++ API
The C++ API is preferred to use and has to complete API functionality range.

### 1.1 Request or set parameters (frontend interface)
To request or set parameters the frontend interface is used.

**The frontend interface is not usable/published, yet.**

### 1.2 Invoke methods (frontend interface)
To invoke methods (e. g. service functionality) the frontend interface is used.

**The frontend interface is not usable/published, yet.**

### 1.3 Provide parameters (parameter provider interface)
To provide parameters (or to offer methods) a class has to implement the parameter provider interface
(header file @ref wago/wdx/parameter_provider_i.hpp).

To allow future extensions of this interface it is highly recommended not to derive this interface directly but to use
the base class @ref ::wago::wdx::base_parameter_provider which delivers default implementations for all methods
required by the parameter provider interface.

As an example how to create a parameter provider you may take a look to the *example_parameter_provider* in this project
(see [examples page](examples.html "Examples")).

To support advanced scenarios of parameter providing some additional provider types are available:\n 
- @ref ::wago::wdx::model_provider_i (providing WDM information)
- @ref ::wago::wdx::device_description_provider_i (providing WDD information)
- @ref ::wago::wdx::device_extension_provider_i (providing additional features on runtime)
- @ref ::wago::wdx::file_transfer::file_provider_i (providing file content for file API)

### 1.4 Manage providers and device topology (backend interface)
To manage providers (e. g. register/unregister) the backend interface is used
(header file @ref wago/wdx/parameter_service_backend_i.hpp).
The backend interface can be obtained from backend proxy component
(see @ref ::wago::wdx::linuxos::com::parameter_service_backend_proxy).


Providing will only be possible for registered devices (key word: topology). See methods
@ref ::wago::wdx::parameter_service_backend_i::register_devices()
and
@ref ::wago::wdx::parameter_service_backend_i::unregister_all_devices() there for.


Attention:\n 
Use of backend interface is only possible, when backend proxy is connected to parameter service.
Otherwise an exception of type @ref ::wago::wdx::linuxos::com::exception is thrown.\n 
To get a trigger when proxy component is connected and backend interface ready for use a notifier is available to be set
on notification manager: @ref ::wago::wdx::linuxos::com::backend_notification_manager_i::add_notifier()\n 
A reference to notification manager can be obtained from proxy component: @ref ::wago::wdx::linuxos::com::parameter_service_backend_proxy::get_notification_manager()

### 1.5 Create a parameter service backend proxy instance
To create a parameter service backend proxy instance (e. g. to communicate with parameter service backend) use an instance of class
@ref ::wago::wdx::linuxos::com::parameter_service_backend_proxy (header file @ref wago/wdx/linuxos/com/parameter_service_backend_proxy.hpp).

The proxy component is a passiv component without own thread. To get them working this component have to be driven.
For that reason the proxy component offers a @ref ::wago::wdx::linuxos::com::driver_i interface
(header file @ref wago/wdx/linuxos/com/driver_i.hpp).
The driver interface offers two methods to drive the component.
The method @ref ::wago::wdx::linuxos::com::driver_i::run_once() executes at most one single task.
Alternatively the method @ref ::wago::wdx::linuxos::com::driver_i::run() executes all currently waiting tasks.
Calls of this two methods may be mixed as needed.


Often it is a good idea to allow log output for failure analysis or debugging purposes. With help of function
@ref ::wago::paramlog::set_log_function() you can set a log output function where all log output is written to.
As log output function you may use any of the pre-defined functions\n 
@ref ::wago::paramlog::log_stdout()\n 
@ref ::wago::paramlog::log_stderr()\n 
@ref ::wago::paramlog::log_outstream()\n 
@ref ::wago::paramlog::log_syslog()\n 
or an own function.

Hint:
If you want to use the pre-defined function\n 
@ref ::wago::paramlog::log_syslog()\n 
you have to call\n 
@ref ::wago::paramlog::init_syslog()\n 
before first log output call and\n 
@ref ::wago::paramlog::close_syslog()\n 
at the end.

With help of function
@ref ::wago::paramlog::set_log_level() you can set a log level to avoid unwanted log output.
You may also call this method at any later time again to increase or decrease the wanted log level (e. g. to increase
verbosity for debugging).


As an example how to integrate a parameter service backend proxy into a process you may take a look to the
*example_backend_client* in this project (see [examples page](examples.html "Examples")).

### 1.6 Create a parameter service frontend proxy instance
**Currently the there is no frontend proxy published, yet.**

## 2 WAGO Parameter Service C API
**Currently the C API is prepared but not usable/published, yet.**
