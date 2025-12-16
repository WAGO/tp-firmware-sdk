# WDx Core

## Bibliotheken

In diesem Projekt werden plattform-unabhängige Bibliotheken entwickelt und bereitgestellt:

### WDx-Core `libwdxcore`

Die zentrale Implenentierung des WDx-Dienstes. Sie gilt als Ausgangsbasis für Integrationen in Firmware-Plattformen.

Hier sind unter anderem die allgemeinen WDx-Schnittstellen `parameter_service_frontend_i` `parameter_service_backend_i` implementiert, sowie die Schnittstellen `parameter_provider_i` und `file_provider` definiert.

### WDx-Test `libwdxtest`

In dieser Bibliothek sind Hilfsfunktionen, -implementierungen und Mocks enthalten, die bei der
Erstellung von Unittests im Kontext der WDx-Entwicklung (z.B. Parameterprovider) unterstützen können.

### WDx-WDA `libwdxwda`

Bibliothek, welche die Implementierung der WDA REST-API enthält. Sie definiert Schnittstellen, welche
Firmware-spezifisch implementiert werden müssen, zum Beispiel solche um die API an einen tatsächlichen Webserver anzubinden.

## Entwickeln, Bauen und Testen

Das Projekt ist ein Cmake-Projekt, welches eine Umgebung mit folgenden Tools und bibliotheken voraussetzt:

- CMake in der benötigten Version (siehe CMakeLists.txt)
- googletest 1.8 (nur wenn Unittests gebaut werden sollen; kann abgeschaltet werden)
- Weitere Abhängigkeiten werden während des CMake-Builds via FetchContent bezogen, wenn so konfiguriert. Ansonsten werden sie ebenfalls vorausgesetzt:
  - [common-header](https://svgithub01001.wago.local/BU-Automation/common-header/)
  - [nlohmann_json](https://github.com/nlohmann/json)

### Devcontainer

Um in diesem Repository zu arbeiten, sind **Devcontainer** vorgesehen. Die entsprechende Vorbereitung sollte VS Code automatisch erkennen und entsprechend vorschlagen, in einen Devcontainer zu wechseln. Dafür ist die Erweiterung **Dev Containers** notwendig, welche, falls noch nicht installiert, automatisch von VS Code vorgeschlagen werden sollte. Innerhalb des Devcontainers sind alle Abhängigkeiten vorhanden, so dass das Projekt gebaut und getestet werden kann.

#### Vorbedingungen

Im Laufe des Builds versucht dieses Repository Artefakte aus dem Artifactory hertunerzuladen. Dazu benötgigt es Berechtigungen. Diese werden über den Mechanismus einer **.netrc** gegeben. Dazu muss der Nutzer in seinem **Homeverzeichnis** ein File mit dem Namen **.netrc** anlegen, mit folgendem Inhalt:

``` bash
machine artifactory.wago.local password <artifactory-key>
```

Der weiter unten beschriebene Devcontainer reicht diese Datei durch. Somit bekommt auch der Container die Berechtigungen.

#### Änderungen

Sind Änderungen an zum Beispiel der Docker Umgebung notwendig, geschehen diese nicht innerhalb der Dockerfiles. Um weitere apt Pakete hinzuzufügen, editiert man eine Liste in dem Dokument **tools &rarr; setup &rarr; debian &rarr; ubuntu_focal-packages.txt**. Ähnliches gilt für Snaps, PPAs und Zertifikate.

## Workflow

### auf der Konsole

#### Tests ausführen

```text
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug && \
cmake --build build/debug --target check
```

#### Code Coverage bestimmen

```text
cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=yes && \
cmake --build build/coverage --target check
```

Dieser Aufruf weist GCC an, die Binarys mit Code zum Bestimmen der Coverage ([gcov]) zu instrumentieren. Beim Compilieren wird zu jedem Objectfile eine zusätzliche Datei mit der Endung `.gcno`, später bei der Ausführung eine weitere mit der Endung `.gcda` erzeugt.  
Der Inhalt der `.gcda` Dateien kann in VS Code dargestellt werden, z.B. mit der Extension [Gcov Viewer]. Diese Extension wird über die im Ordner `.devcontainer` abgelegte Konfiguration beim Erstellen eines neuen Containers automatisch installiert. Beim erneuten Betreten einer bereits erstellten Umgebung wird diese Erweiterung zur Installation vorgeschlagen (siehe `.vscode/extensions.json`).

Wenn eine Quellcodedatei im Editor geöffnet ist, kann mit `Strg+Umsch+P` und `GCov Viewer: Show` die Coverage im Quellcodeeditor angezeigt werden. Durchlaufene Branches werden grün, nicht durchlaufene rot und nur teilweise durchlaufene Entscheidungen gelb angezeigt.  
Nach einer erneuten Ausführung der Tests muss die Anzeige mit `Gcov Viewer: Reload` manuell aktualisiert werden.  

> [!NOTE] inkrementelle Abdeckung
>
> Das Bestimmen der Abdeckung geschieht inkrementell, d.h. die Zähler für die Zeilendurchläufe steigen bei jedem Programmlauf.  
> Wenn die Codeabdeckung für einen einzelnen Durchlauf bestimmt werden soll, müssen zunächst die alten Abdeckungsdaten gelöscht werden:
> `find -name '*.gcda' -delete`

##### Coverage für einen einzelnen Test bestimmen

Durch die Vorgabe von `GTEST_FILTER` können einzelne Tests ausgeführt werden.

```text
cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=yes && \
GTEST_FILTER=parameter_value_test.create_uint8 cmake --build build/coverage --target check
```

#### Code Coverage Report erstellen

Das zusätzliche Build Target `coverage` kann verwendet werden, um nach dem Ausführen der instrumentierten Tests aus den Coveragedateien einen HTML- sowie einen XML-(Cobertura-)Report zu erstellen:

```text
cmake -B build/coverage -DCMAKE_BUILD_TYPE=Debug -DWITH_COVERAGE=yes && \
cmake --build build/coverage --target coverage
```

> [!NOTE] absolute Abdeckung
>
> Das Bestimmen der Abdeckungsreports geschieht _nicht_ inkrementell, d.h. die Zähler für die Zeilendurchläufe repräsentieren hier einen einzelnen Programmlauf.  
> (Der durch das Target `coverage` definierte Ablauf löscht die gcda-Dateien vor der Ausführung des Testexecutables.)

* Die XML-Datei wird verwendet, um die Gesamtcoverage auf Jenkins anzuzeigen. Der Inhalt dieser Datei kann z.B. mit _[Coverage Gutters]: Display Coverage_ angezeigt werden.
* Die HTML-Datei kann z.B. mit _[Coverage Gutters]: Preview Coverage Report_ angezeigt werden.
* Alternativ kann die Extension [Live Preview] verwendet werden.

### Ausführen der Tests in VS Code

tbc.

## Bauen wie auf dem Buildserver

### Bau

``` bash
docker buildx bake -f docker-bake.hcl -f .env build
```

### Test

``` bash
docker buildx bake -f docker-bake.hcl -f .env test
```

### Coverage

``` bash
docker buildx bake -f docker-bake.hcl -f .env coverage
```

### Dist (nur Erzeugung von source-Package)

``` bash
docker buildx bake -f docker-bake.hcl -f .env dist
```

### Bauen mit lokaler nlohmann, abgelegt unter external_libs

``` bash
docker buildx bake -f docker-bake.hcl -f .env build --set *.args.LOCAL_NLOHMANN=ON
```

### Testen mit lokalen gtest, abgelegt unter external_libs

``` bash
docker buildx bake -f docker-bake.hcl -f .env test --set *.args.LOCAL_GTEST=ON
```

## Jenkins

### Ablauf des Buildjobs

Auch auf dem Jenkins werden die beschriebenen Container zum Bauen und Testen genutzt.

Dafür gibt es in der Jenkins-Bibliothek ein eigenes bakeProject.

Der Buildjobs ist nun Multibranch-fähig: [Jenkins WDx Core Jobs](https://svli01003.wago.local/jenkins/job/firmware/job/wdx/job/core/)

Er baut somit alle Branches, PRs und Tags.

Alles außer Tags wird beim Deployen des Source-Paketes mit einer snapshot-ID versehen, um Eindeutigkeit herzustellen.

Bei Tags wird diese ID entfernt, das Paket wird einmalig in der definierten Version abgelegt.

## Changelog

### 2

* Changelog discontinued, see https://svgithub01001.wago.local/BU-Automation/wdx-core/releases

### 1.7.6

* WAT-37056: Core bugfix: Avoid class searches on matching selected parameters

### 1.7.5

* WAT-37023: Core bugfix: Corrected detection of invalid parameter instance IDs

### 1.7.4

* WAT-37023: Core bugfix: Corrected applied overrides for derived classes

### 1.7.3

* WAT-36950: Core bugfix: Corrected response construction with domain specific status code

### 1.7.2

* WAT-36950: Core bugfix: Copy also value on write status `success_but_value_adjusted`

### 1.7.1

* WAT-36950: Providers should respond adjusted values directly

### 1.7.0

* WAT-36810: Added missing header for GCC 13 compatibility
* PTX-50: Enabled WDx Core to support IO channels:  
          Support WDMM version v1.1.0 (added overrides for class instances)  
          Added logging of applied overrides
* PTX-50: Check for implemented WDMM version on WDM/WDD load
* PTX-50: Fixed parameter ID cut after 16 bits on WDD parsing
* PTX-50: Added new status code `success_but_value_adjusted` for nonessential changes on parameter write
* WAT-36920: Mark instance parameters as user setting if class is a user setting

### 1.6.0

* YBP-3421: Removed deprecated member access for parameter_exception::error_code
* YBP-3419: Do not allow ambiguous device IDs with numerics and text (2-3 vs. rlb-3)
* WAT-36794: Corrected compiler warnings for printf in unit tests
* WAT-36682: Check identity reference values for correct syntax
* Fixed some clang-tidy warnings
* YBP-3195: Set minimum required CMake version to v3.10
* YBP-3365: Lower future unit test sleep times for faster unit test execution
* YBP-3359: Added gdb dependency for debugging support
* YBP-3329: Fixed some issues in Jenkinsfile
* YBP-3195: Added support for code coverage report

### 1.5.3

* Fix error Cross-feature references do not allow dynamic classes

### 1.5.2

* Made status code member of parameter_exception public accessable again (deprecated)
* Introduce const getter for status code member of parameter_exception

### 1.5.1

* Fixed parameter unproviding if serializing provider wrapper is used
* Print error reason to log instead of exception type
* Added warn log message if no parameter instance is unprovided when unregister a parameter provider

### 1.5.0

* `is_deprecated` on `feature_definition`, `class_definition` and `parameter_definition`
* `device::has_deprecated_parameters`
* `parameter_filter::without_deprecated()`, `parameter_filter::only_deprecated()`

### 1.4.5

* Fixed parameter filter IPC serialization
* Added parameter filter comparison operators

### 1.4.4

* Fixed parameter value creation with unset identity reference

### 1.4.3

* Fixed pc file template encoding (UTF8-BOM -> UTF8)
* Fixed linker command options in pc file

### 1.4.2

* Fixed CMake install step for static lib, headers and .pc file

### 1.4.1

* Added checks for instance resets
* Add check or instance identity ref parameters that a referenced base path point to an allowed class
* Corrected several error messages
* Added single argument overload to create instance identy ref parameter values
* Removed obsolete example code
* Refactored project CMake files

### 1.4.0

* First extensions for new value type "identity ref"
* Move tests for parameter_value class into separate test class
* Add "instanceKey" to parameter definition and device model loader
* Add "RefClasses" to parameter definition
* Added checks for instance refs & instance keys
* Use numeric limit instead of number itself
* Fixed typos
* Remove counting in enum definiton
* Added missing logs in case of internal errors
* Ensure instance keys are only writeable in reset case
* Fix: create float array
* Fix: Made parameter path comparision case insensitive

### 1.3.4

* fix feature requests for a specific device

### 1.3.3

* several fixes and source code improvements

### 1.3.2

* updated wstd to version 1.2.5

### 1.3.1

* fixed incomplete preparation of feature classes
* more warnings for issue detection

### 1.3.0

* `parameter_filter::only_subpath` fully implemented
* more filters: `without_usersettings`, `only/without_writeable`
* correctly applying filtering of instances parameters
* removed deprecated `domain_specific_message`
* removed ambiguous `to_string` and `from_string` for `std::string` aliases

### 1.2.41

* `get_enum_definition` and `get_all_enum_definitions`
* `get_all_parameter_definitions` with extended `parameter_filter` (`only_subpath` only working for complete paths)
* `parameter_response` contains correct pointer to `method_definition`

### 1.2.40

* fixed macro for NDEBUG

### 1.2.39

* Different semantics of `has_error` depending on context
* validating ranges for `parameter_instance_path`, in `from_string` and core
* added 64 bit bounds check for `parameter_value`
* `status_value_unavailable_if_not_provided` flag in WDD
* added more debug log output, except when built with `NDEBUG`
* nlohman/json 3.11.2
* improved doc

### 1.2.38

* `status_codes::status_value_unavailable`
* `parameter_value::create_unset_instance_ref()` and `::is_unset_instance_ref()`

### 1.2.37

* using base constructors for file transfer responses
* relaxed nlohmann version requirement

### 1.2.36

* fixed GCC errors/warnings

### 1.2.35

* non-blocking backend interface implementation
* limiting upload_ids to 100

### 1.2.34

* **BREAKING**: async backend
* `is_beta` on `feature_definition`, `class_definition` and `parameter_definition`
* `device::has_beta_parameters`
* `parameter_filter::without_beta()`, `parameter_filter::only_beta()`
* number range violations reported as `::invalid_value` instead of `::wrong_value_representation`

### 1.2.33

* implemented `cleanup` properly
* corrected statuscode for unresponding `create_parameter_upload_id`
* made `response::get_message()` const

### 1.2.32

* fixed missing virtual dtor

### 1.2.31

* every `response` may have a `message`
* cleanup method
* updated nlohmann-json to 3.10.5
* improved file-id handling for error cases
* improved doc

### 1.2.30

* milestone release for `file_id` handling

### 1.2.29

* intermediate release for more file_id functionality

### 1.2.28

* intermediate release for first implementation of create_upload_id

### 1.2.27

* removed deprecated methods of `parameter_value` and `response` types

### 1.2.26

* checking integer bounds of `parameter_value`s
* made `file_id` longer to make collisions very unlikely

**NOTE**: This is the last release which contains the deprecated methods of `parameter_value` and `response` types.

### 1.2.25

* made call mode an enum class

### 1.2.24

* register_parameter_providers with optional call mode `serialized`

### 1.2.23

* fixed methods in class instances
* generating pseudorandom file_id

### 1.2.22

* fixed exception copy
* removed doc that is not valid yet

### 1.2.21

* not logging parameter_provider display_name, which could cause deadlock

### 1.2.20

* fixed internal_error when multiple parameter_providers are involved in set_parameter_values

### 1.2.19

* invalid parameter value results in rejection of all parameters meant for the same parameter_provider
* fixed device_extension_response ctors

### 1.2.18

* improved logging of invalid values
* (re)using ctor with status_code for all responses
* ctor with domain_specific_statuscode for value_response
* explicit status_code for the attempt of reading values of methods. parameter_providers will not be called for reading values for methods anymore
* reporting all invalid values from parameter_provider as internal_error
* validating In&OutArgs with enum type
* exception in PP.invokeMethod results in an internal_error with logging
* correctly integrating pp exceptions into result. Also fixed memory leak in that case.
* removed pfc-demo and fcgi-lib

### 1.2.17

* corrected status_code for invalid device slots in path

### 1.2.16

* fixed paging of get_all_parameters
* get_all_parameters can filter without_file_ids and only_file_ids
* get_instances differentiates between ::unknown_device and ::unknown_device_collection
* improved doc

### 1.2.15

* set_parameter_response::set_deferred
* fixed status code serialization
* fixed linking errors

### 1.2.14

* fixed missing status_codes::wda_connection_changes_deferred

### 1.2.13

* parameter_provider_i with deferred set_parameter_values
* frontend method with defer_wda_web_connection_changes flag
* get_all_parameters can be filtered by only_methods
* made responses more homogenous
* better named methods for signal errorneous responses (deprecated old ones)
* fixed is_valid for floating point numbers
* documented status_codes used by parameter_providers
* fixed memory leaks
* cleaned some warnings

### 1.2.12

* get_all_parameters threadsafe and working with dynamic instances
* overrideables.inactive and all associated behavior:
  * writing ignored
  * using fixed_value from WDD
  * not treated as user_setting in get_all_parameters
* fixed register order issues
* using wstd 1.2.3
* convenience method is_success

### 1.2.11

* fixed deadlocks
* fixed non-threadsafe code

### 1.2.10

* thread safety via mutexes
* serializing domain_specific_message for set_parameter_response

### 1.2.9

* support WDDs with partially missing model information
* model_providers can be registered after device_description_providers, device_extension_providers or devices
* robust behavior when parameter_provider makes a "successful" response without value

### 1.2.8

* improved paging
* domain specific code/message for value_response & parameter_response
* BREAKING: renamed method_specific_status_code -> domain_specific_status_code
* status_codes::ignored for ignored parameters in the set

### 1.2.7

* fixed compilation error (ipc of size_t)

### 1.2.6

* IPC improvements
  * asynchronous retrieval of model and device descriptions
  * fixed cyclic reference issue which prevented futures from being cleaned up
  * method_specific_message serialized
* frontend improvements
  * condensed parameter_service_frontend_i into essential frontend methods, others moved to frontend_extended
  * ipc serialization for parameter_service_frontend_i
  * fixed method specific status_code and message missing from result
* improvements for parameter_providers
  * status_code::other_invalid_value_in_set, for parameters that were withheld because others belonging to the same consistency set were invalid
  * made has_class const
  * deprecated parameter_value::get_bool -> get_boolean should be used instead

### 1.2.5

* fixed provider issue for parameters of dynamic instances

### 1.2.4

* fixed re-registration issue with providers for dynamic instances
* added internal debugging utilities

### 1.2.3

* ipc serialization of parameter_value also holds its type and rank, fixes remote `parameter_provider_i`s
* simplified base_parameter_provider code
* correct status_codes for base_parameter_provider, improved doc

### 1.2.2

* fixed ipc serialization of empty value_response, also simplified for other types

### 1.2.1

* ipc serialization for device_id, file_read_response, file_info_response

### 1.2.0

* BREAKING: All frontend methods are asynchronous
* The following limitations are present:
  * die Methoden zum Verwalten und Abfragen von Monitoringlisten sind nicht threadsafe (der problematische Fall tritt auf wenn 2 Threads parallel versuchen, eine neue Monitoringliste anzulegen, die restlichen Fälle sind in der Praxis unkritisch)
 der Core würde derzeit sein Versprechen an die ParameterProvider nicht halten, dass er ihnen keine weitere Anfrage stellt solange sie noch die vorige Anfrage bearbeiten (es wird also nirgends gequeued)
 get_all_parameters liefert derzeit keine Parameter einer dynamischen Klasse

### 1.1.24

* default constructors from asynchronous branch
* correctly preparing builtin parameters for OrderNumber and FW-Version
* corrected doc

### 1.1.23

* fixed instantiation of polymorphic classes or features

### 1.1.22

* parsing and validating AllowedLength for Arrays
* all backend register/unregister methods have singular and plural version
* fixed monitoring list behaviour with changing values
* fixed memory leak by cyclic references
* build flag for using WSTD future
* BREAKING: renamed monitor_list -> monitoring_list
* warning on duplicate model definitions or wdd features
* documenting status_codes for file_api
* preparation for exceptions in backend/frontend stubs

### 1.1.21

* `UserSetting` attribute, `get_all_parameters` can filter for them
* interpreting and validating `AllowedValues`
* domain specific error messages can be set in responses for `set_parameters` and `invoke_method`
* fixed `json_value` for empty arrays
* explicit `parameter_instance_id` ctor
* fixed overrides

### 1.1.20

* fixed checking of empty array values

### 1.1.19

* get_device(device_id device)
* unregister_devices(vector<device_id> device_ids)
* paging get_all_parameters
* mirroring unknown ids/paths

### 1.1.18

* redesigned get_all_parameters
* == and != for parameter_value
* corrected display_name for parameter_providers derived from base_parameter_provider
* removed file_finish (not needed for now)

### 1.1.17

* improved creation of parameter_values (e.g. `parameter_value::create_instance_ref_array({1, 3});`)
* convenience ctor for base response (e.g. `response(status_codes::success)`)
* fix: 0-0-0-0 addresses instantiation parameter of class without base_id
* correct inclusion of nlohmann/json

### 1.1.15

* improved arguments of `file_provider_i` methods

### 1.1.14

* added `file_provider_i::create`

### 1.1.13

* new frontend methods `get_parameter_definitions` / `get_parameter_definitions_by_path`
* allowing both `/` and `-` for `device_path_t`
* replacing `/` with `-` for http representation of `parameter_instance_path` INCLUDING `device_path_t`

### 1.1.12

* fixed gcc errors

### 1.1.11

* fixed gcc errors

### 1.1.10

* treating dynamic class as non-dynamic for device with instantiations in WDD

### 1.1.9

* supporting valuetype ipv4address
* introducing dynamic instances, instantiation parameter
* validation support, fixes and convenience for parameter rank ::array
* changed file_provider methods from size_t to uint64_t
* parameter_provider_i implementations have status_codes::invalid_value for validating write requests

### 1.1.8

* introducing file_providers and new value_type file_id
* new value_type instance_ref, attribute ref_class
* BREAKING: renamed Version/ModelVersion in WDM/WDD artifacts -> WDMMVersion
* domain specific status code for set_parameter_response
* added null check for parameter_values
* made response constructors explicit
* unknown includes handled more gracefully

### 1.1.7

* logging invalid values from parameter_provider_i
* handling exceptions thrown by parameter_provider_i
* checking device_id at registration
* warning for unknown parameter_value_types

### 1.1.6

* redesigned interfaces for model/device information providers
* separated registering again
* typedefs for wdd/wdm content

### 1.1.5

* redesigned interfaces for model/device information providers
* registering unified for all types of providers

### 1.1.4

* fixed GCC build errors

### 1.1.3

* introduces `model_provider_i`
  * wdm/wdd fragments can be provided (bundle not possible yet)
  * replaces `artifact_repository_i`
* `parameter_response` now contains `parameter_instance_path`
* corrected `to_json` for `parameter_value`s of rank array
* documentation for `parameter_value`

### 1.1.2

* reintroduced parameter_value::create, with the added possibility of explicitly setting the value type
* convenience methods for ParameterProviders setting value / out_args
* added more string representations for basic types
* simplified parameter_response
* fixed more warnings

### 1.1.1

* `DefaultValue` for `MethodInputArguments` supported
* UInt64, Int8, Int16, Int32, Int64, Float64 supported
* creating `parameter_value`s of rank array supported from unknown type
* all `parameter_value` create-methods explicitly say which `parameter_value_type`
* checking if parameter_values are integer/unsigned
* fixed GCC build errors/warnings

### 1.1.0

* 32bits for parameter_id_t
* mocking firmware-version with correct format
* supporting attribute Pattern, validating string values with it
  * for `set_parameter_values`, for values from WDD and from parameter providers
* supporting attribute Writable
* supporting Enums, with validation
* supporting Overrides in WDM and WDD
* supporting value type UInt8, UInt32
* == and != for `device_id` and `parameter_instance_path`
* renamed `empty_parameter_provider` -> `base_parameter_provider`, without override of `get_provided_parameters`
* `device_selector`s, can be combined with `parameter_selector`s
* string representation for basic id types
* `::get_parameters{_by_path}` always return the `parameter_definition` and `parameter_instance_id` along with the value
* `::get_all_devices`
* better management of `monitor_lists`, always retrievable by client

### 1.0.19

* removed unnecessary methods:
  * parameter_provider_i
    * ::start_providing
    * ::stop_providing
  * parameter_service_backend
    * ::unregister_all_parameter_providers
    * ::provide_parameter_ids
    * ::provide_devices

### 1.0.18

* firmware_version can now be passed at ::register_devices (if not, it will be mocked as "1.0.0", but later this will be rejected)
* renamed CMake Flag PFC -> PFC-DEMO
* improved code doc

### 1.0.17

* signal methods start_providing/stop_providing for parameter providers
* removed parameter_provider_i::initialize
* parameter_provider_i::get_provided_parameters returns a response
* new backend method unregister_all_parameter_providers
* improved code doc

### 1.0.16

* better possibilities for parameter_providers to declare what they are providing
  * ::get_provided_parameters method will be called by parameter service, returning parameter_selectors
  * parameter_selectors can select whole devices, features or classes, filtered by device technology (only by device_collection atm)
* improved code doc

### 1.0.15

* integrated fixed future.hpp from wstd
* parameter_service_i::set_log_fn
* logging loaded device descriptions

### 1.0.14

* empty_parameter_provider methods now public
* firmware_version is part of topology information (value mocked for now)

### 1.0.13

* asynchronous parameter_provider_i methods, using wago::future
  * the implementation of parameter_service does not take advantage of this yet
* logging function can be passed to parameter_service_factory
  * either with c-style function pointer or std::function
* cleaned frontend interface
  * removed superfluous const's
  * removed unnecessary or risky call-by-refs
* renamed parameter_instance_identification -> parameter_instance_id
* comparator for parameter_instance_id
* parameter_provider::initialize can now return a status_code to signal if something went wrong.
  * the parameter_service will not call the parameter_provider ever again, no matter if parameters where provided
  * this method will be removed soon, because it is too powerful at the moment. It might be reintroduced when the use cases surrounding the "wiring" of parameter_providers are better understood.
* reduced the usage of shared_ptr's
  * parameter_providers -> plain ptr
  * artifact_repository -> plain ptr
  * artifacts for model and device descriptions -> unique_ptr
* made namespace wda::wdmm inline (all wdmm declarations are now also in namespace wda)
* renamed parameter_provider_i_default_implementation -> empty_parameter_provider
* upgraded to nlohmann/json 3.7.0
* fixed minor memory leak

### 1.0.12

* New Methods for monitor_lists
  * create_monitor_list[_with_paths]: Create a monitor list that prepares a list of parameters to read (multiple times) later with minimal overhead
  * get_values_for_monitor_list: Retrieve the parameter values for the created monitor list
  * delete_monitor_list: Cleanup explicitly. Monitor lists will be cleaned up automatically after timeout
* minor code style changes

### 1.0.11

* Code style changed
* no functional changes

### 1.0.10

* ParameterServiceBackend::[Un]RegisterParameterProviders
* Using value class ParameterValue, instead of JsonParameterValue
  * CreateValue-Methods for creation from native types
  * CreateWithUnknownType for creation from json messages
* Value type "Bytes" fully supported (with base64-encoding for json serialization)
* Json-Schema for device descriptions and device model
* easier ValueDeclarations in device descriptions
* easier and more compact Instantiations in device descriptions
* bracket initializer support for responses
* convenience IParameterProviderDefaultImplementation
* to_string method for StatusCodes
* documented used StatusCodes in Responses

### 1.0.9

* ParameterServiceBackend::UnregisterAllDevices
* ParameterService::SetParameterValues[ByPath] and ParameterService::InvokeMethod[ByPath] will do the following:
  * check the given ParameterValue type against the model
  * supplement the ParameterValue type if unknown by caller
  * test if ParameterValue can be converted to native value

### 1.0.8

* ParameterService::GetSubdevices[ByCollectionName] returns DeviceCollectionResponse
* ParameterService::InvokeMethod[ByName] with named arguments instead of positional arguments
* renamed ParameterPath to ParameterInstancePath (and devicepath_t to device_path_t)
* Default constructors for Response base class
* eliminated all usings from public headers

### 1.0.7

* ParameterService now hidden from public interface, instead create with ParameterServiceFactory
* ParameterServiceFactory will call ParameterService.Initialize for you
* ParameterService::RegisterDevices returns SUCCESS instead of NO_ERROR_YET
* moved public interface to wago::wda
* moved WDMM headers to wago::wda::wdmm
* moved RPC and json conversion functions into wago::wda_ipc
* Keeping the globals clean: STATUSCODESTRINGS now constexpr, DEVICEID_ROOT now DeviceID::HEADSTATION
* eliminated 'using namespace std' from public headers
* fixed lots of warnings
* temporarily removed all printf's, will switch to logging function/class

### 1.0.6

* Binary is linkable as shared lib with codesys runtime
* started better organisation of inc folder
* fixed build issues
* doxygen configuration file

### 1.0.5

* GetSubdevicesByName heißt jetzt GetSubdevicesByCollectionName
* ErrorCodes heißen jetzt StatusCodes
* GetSubdevices[ByCollectionName] liefert jetzt die vollständige DeviceID
* Code-Dokumentation für IParameterServiceFrontend
* verbesserte Code-Dokumentation an diversen Stellen

### 1.0.4

* Parameterpfade case-insensitive
* Neue Methode IParameterServiceFrontend.GetSubdevicesByName (der Name der DeviceCollection ist ebenfalls case-insensitive)
* GetSubdevices[ByName] liefern DeviceResponses mit Slot und Artikelnummer
* Device-Pfad is nun {deviceCollection}-{slot} statt {deviceCollection}/{slot}
* Code-Dokumentation für IParameterServiceBackend und IParameterProvider komplett (als nächstes kommt das IParameterServiceFrontend)

## Dieses Projekt ist cross-kompilierbar

Es verwendet CMake für die Kompatibilität mit verschiedenen Umgebungen.

[//]: <> (List of references)

[Gcov Viewer]: https://marketplace.visualstudio.com/items?itemName=JacquesLucke.gcov-viewer
[Live Preview]: https://marketplace.visualstudio.com/items?itemName=ms-vscode.live-server
[gcov]: https://gcc.gnu.org/onlinedocs/gcc/Gcov-Intro.html
[Coverage Gutters]: https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters
