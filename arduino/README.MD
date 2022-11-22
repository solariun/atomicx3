# Arduino procedures

## Install arduino-cli

## Useful commands

```
arduino-cli board details -b arduino:avr:uno --list-programmers

arduino-cli board details "$1"

arduino-cli board listall

arduino-cli board listall esp8266:esp8266:wifi

arduino-cli board listall avr

arduino-cli lib install  "Adafruit AMG88xx Library"

arduino-cli lib search AMG88

```

## Compiling and uploading

```bash

arduino-cli compile -b arduino:samd:mkrzero  --upload -p <serial path>  <project dir>

arduino-cli compile -b arduino:avr:nano:cpu=atmega328  --upload -p "$1" "$2"

arduino-cli compile -b arduino:mbed_rp2040:pico  --upload -p "$1" "$2"

# upload AVR using USB-ASP
arduino-cli compile -b arduino:avr:nano:cpu=atmega328  --upload -P usbasp  "$1"

# upload using avrisp XSP
arduino-cli compile -b arduino:avr:nano:cpu=atmega328  --upload -P avrisp -p "$1" "$2"

arduino-cli compile -u -p "$1"  -b esp32:esp32:heltec_wifi_kit_32  "$2"

arduino-cli compile -u  -p "$1" -b Arduino_STM32:STM32F1:genericSTM32F103C:upload_method=jlinkMethod,opt=o3std "$2"

arduino-cli compile -v -b esp8266:esp8266:wifi_kit_8:baud=921600 -upload -p "<serial>" "<arduino>"

arduino-cli compile -v -b esp8266:esp8266:nodemcuv2:baud=921600 -upload -p "<serial>" "<arduino>"

arduino-cli compile -b arduino:mbed_nano:nano33ble -upload -p "<serial>" "<arduino>"

# Using ST-LINK
arduino-cli compile -v -b STMicroelectronics:stm32:GenF1:pnum=BLUEPILL_F103C8,usb=CDCgen -upload -p "." "<arduino>"

```

## Hints for bluepill STM32F103

Exemple compilation/upload:

`arduino-cli compile -v -b STMicroelectronics:stm32:GenF1:pnum=BLUEPILL_F103C8,usb=CDCgen -upload -p "." .`

If you see the error: `Error during Upload: Property 'upload.tool.serial' is undefined`
please, Add the following line anywhere in the STM32 boards.txt file, otherwise:
`GenF1.menu.upload_method.swdMethod.upload.tool.default=stm32CubeProg`


Despite the lack of feature parity at the moment, Arduino CLI provides many of the features you can find in the Arduino
IDE. Let's see some examples.

### LIST CPUs and other options for a specific board

````
arduino-cli board details -b STMicroelectronics:stm32:GenF1

arduino-cli board details -b arduino:avr:nano
````

## Before you start

`arduino-cli` is a container of commands and each command has its own dedicated help text that can be shown with the
`help` command like this:

```console
$ arduino-cli help core
Arduino Core operations.

Usage:
    arduino-cli core [command]

Examples:
    ./arduino-cli core update-index

Available Commands:
    download     Downloads one or more cores and corresponding tool dependencies.
    install      Installs one or more cores and corresponding tool dependencies.
    list         Shows the list of installed platforms.
    search       Search for a core in Boards Manager.
    uninstall    Uninstalls one or more cores and corresponding tool dependencies if no more used.
    update-index Updates the index of cores.
    upgrade      Upgrades one or all installed platforms to the latest version.

Flags:
    -h, --help   help for core

Global Flags:
        --additional-urls strings   Additional URLs for Boards Manager.
        --config-file string        The custom config file (if not specified the default will be used).
        --format string             The output format, can be [text|json]. (default "text")
        --log-file string           Path to the file where logs will be written.
        --log-format string         The output format for the logs, can be [text|json].
        --log-level string          Messages with this level and above will be logged.
    -v, --verbose                   Print the logs on the standard output.

Use "arduino-cli core [command] --help" for more information about a command.
```

## Create a configuration file

Arduino CLI doesn't strictly require a configuration file to work because the command line interface provides any
possible functionality. However, having one can spare you a lot of typing when issuing a command, so let's go ahead and
create it with:

```sh
$ arduino-cli config init
Config file written: /home/luca/.arduino15/arduino-cli.yaml
```

If you inspect the contents of `arduino-cli.yaml`, you'll find the available options with their respective default
values. For more information, see the [configuration documentation].

## Create a new sketch

To create a new sketch named `MyFirstSketch` in the current directory, run the following command:

```sh
$ arduino-cli sketch new MyFirstSketch
Sketch created in: /home/luca/MyFirstSketch
```

A sketch is a folder containing assets like source files and libraries; the `new` command creates for you a .ino file
called `MyFirstSketch.ino` containing Arduino boilerplate code:

```sh
$ cat $HOME/MyFirstSketch/MyFirstSketch.ino
void setup() {
}

void loop() {
}
```

At this point you can use your favourite file editor or IDE to open the file `$HOME/MyFirstSketch/MyFirstSketch.ino` and
change the code like this:

```c
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
```

## Connect the board to your PC

The first thing to do upon a fresh install is to update the local cache of available platforms and libraries by running:

```sh
$ arduino-cli core update-index
Updating index: package_index.json downloaded
```

After connecting the board to your PC by using the USB cable, you should be able to check whether it's been recognized
by running:

```sh
$ arduino-cli board list
Port         Type              Board Name              FQBN                 Core
/dev/ttyACM1 Serial Port (USB) Arduino/Genuino MKR1000 arduino:samd:mkr1000 arduino:samd
```

In this example, the MKR1000 board was recognized and from the output of the command you see the platform core called
`arduino:samd` is the one that needs to be installed to make it work.

If you see an `Unknown` board listed, uploading should still work as long as you identify the platform core and use the
correct FQBN string. When a board is not detected for whatever reason, you can list all the supported boards and their
FQBN strings by running the following:

```sh
$ arduino-cli board listall mkr
Board Name              FQBN
Arduino MKR FOX 1200    arduino:samd:mkrfox1200
Arduino MKR GSM 1400    arduino:samd:mkrgsm1400
Arduino MKR WAN 1300    arduino:samd:mkrwan1300
Arduino MKR WiFi 1010   arduino:samd:mkrwifi1010
Arduino MKRZERO         arduino:samd:mkrzero
Arduino/Genuino MKR1000 arduino:samd:mkr1000
```

## Install the core for your board

To install the `arduino:samd` platform core, run the following:

```sh
$ arduino-cli core install arduino:samd
Downloading tools...
arduino:arm-none-eabi-gcc@4.8.3-2014q1 downloaded
arduino:bossac@1.7.0 downloaded
arduino:openocd@0.9.0-arduino6-static downloaded
arduino:CMSIS@4.5.0 downloaded
arduino:CMSIS-Atmel@1.1.0 downloaded
arduino:arduinoOTA@1.2.0 downloaded
Downloading cores...
arduino:samd@1.6.19 downloaded
Installing tools...
Installing platforms...
Results:
arduino:samd@1.6.19 - Installed
arduino:arm-none-eabi-gcc@4.8.3-2014q1 - Installed
arduino:bossac@1.7.0 - Installed
arduino:openocd@0.9.0-arduino6-static - Installed
arduino:CMSIS@4.5.0 - Installed
arduino:CMSIS-Atmel@1.1.0 - Installed
arduino:arduinoOTA@1.2.0 - Installed
```

Now verify we have installed the core properly by running:

```sh
$ arduino-cli core list
ID              Installed       Latest  Name
arduino:samd    1.6.19          1.6.19  Arduino SAMD Boards (32-bits ARM Cortex-M0+)
```

Great! Now we are ready to compile and upload the sketch.

## Adding 3rd party cores

If your board requires 3rd party core packages to work, you can list the URLs to additional package indexes in the
Arduino CLI configuration file.

For example, to add the ESP8266 core, edit the configuration file and change the `board_manager` settings as follows:

```yaml
board_manager:
  additional_urls:
    - https://arduino.esp8266.com/stable/package_esp8266com_index.json
```

If you have your package indexes locally installed, you can list their file path in the Arduino CLI configuration file.

For example, to add the NRF52832 core, edit the configuration file and change the `board_manager` settings as follows:

```yaml
board_manager:
  additional_urls:
    - https://arduino.esp8266.com/stable/package_esp8266com_index.json
    - file:///absolute/path/to/your/package_nrf52832_index.json
```

From now on, commands supporting custom cores will automatically use the additional URL from the configuration file:

```sh
$ arduino-cli core update-index
Updating index: package_index.json downloaded
Updating index: package_esp8266com_index.json downloaded
Updating index: package_nrf52832_index.json
Updating index: package_index.json downloaded

$ arduino-cli core search esp8266
ID              Version Name
esp8266:esp8266 2.5.2   esp8266
```

Alternatively, you can pass a link to the additional package index file with the `--additional-urls` option, that has to
be specified every time and for every command that operates on a 3rd party platform core, for example:

```sh
$ arduino-cli  core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
Updating index: package_esp8266com_index.json downloaded

$ arduino-cli core search esp8266 --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
ID              Version Name
esp8266:esp8266 2.5.2   esp8266
```

The same applies to the additional package index file provided by file paths:

```sh
$ arduino-cli  core update-index --additional-urls file:///absolute/path/to/your/package_esp8266com_index.json
Updating index: package_esp8266com_index.json downloaded

$ arduino-cli core search esp8266 --additional-urls file:///absolute/path/to/your/package_esp8266com_index.json
ID              Version Name
esp8266:esp8266 2.5.2   esp8266
```

## Compile and upload the sketch

To compile the sketch you run the `compile` command, passing the proper FQBN string:

```sh
$ arduino-cli compile --fqbn arduino:samd:mkr1000 MyFirstSketch
Sketch uses 9600 bytes (3%) of program storage space. Maximum is 262144 bytes.
```

To upload the sketch to your board, run the following command, using the serial port your board is connected to:

```sh
$ arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:samd:mkr1000 MyFirstSketch
No new serial port detected.
Atmel SMART device 0x10010005 found
Device       : ATSAMD21G18A
Chip ID      : 10010005
Version      : v2.0 [Arduino:XYZ] Dec 20 2016 15:36:43
Address      : 8192
Pages        : 3968
Page Size    : 64 bytes
Total Size   : 248KB
Planes       : 1
Lock Regions : 16
Locked       : none
Security     : false
Boot Flash   : true
BOD          : true
BOR          : true
Arduino      : FAST_CHIP_ERASE
Arduino      : FAST_MULTI_PAGE_WRITE
Arduino      : CAN_CHECKSUM_MEMORY_BUFFER
Erase flash
done in 0.784 seconds

Write 9856 bytes to flash (154 pages)
[==============================] 100% (154/154 pages)
done in 0.069 seconds

Verify 9856 bytes of flash with checksum.
Verify successful
done in 0.009 seconds
CPU reset.
```

## Add libraries

If you need to add more functionalities to your sketch, chances are some of the libraries available in the Arduino
ecosystem already provide what you need. For example, if you need a debouncing strategy to better handle button inputs,
you can try searching for the `debouncer` keyword:

```sh
$ arduino-cli lib search debouncer
Name: "Debouncer"
    Author: hideakitai
    Maintainer: hideakitai
    Sentence: Debounce library for Arduino
    Paragraph: Debounce library for Arduino
    Website: https://github.com/hideakitai
    Category: Timing
    Architecture: *
    Types: Contributed
    Versions: [0.1.0]
Name: "FTDebouncer"
    Author: Ubi de Feo
    Maintainer: Ubi de Feo, Sebastian Hunkeler
    Sentence: An efficient, low footprint, fast pin debouncing library for Arduino
    Paragraph: This pin state supervisor manages debouncing of buttons and handles transitions between LOW and HIGH state, calling a function and notifying your code of which pin has been activated or deactivated.
    Website: https://github.com/ubidefeo/FTDebouncer
    Category: Uncategorized
    Architecture: *
    Types: Contributed
    Versions: [1.3.0]
Name: "SoftTimer"
    Author: Balazs Kelemen <prampec+arduino@gmail.com>
    Maintainer: Balazs Kelemen <prampec+arduino@gmail.com>
    Sentence: SoftTimer is a lightweight pseudo multitasking solution for Arduino.
    Paragraph: SoftTimer enables higher level Arduino programing, yet easy to use, and lightweight. You are often faced with the problem that you need to do multiple tasks at the same time. In SoftTimer, the programmer creates Tasks that runs periodically. This library comes with a collection of handy tools like blinker, pwm, debouncer.
    Website: https://github.com/prampec/arduino-softtimer
    Category: Timing
    Architecture: *
    Types: Contributed
    Versions: [3.0.0, 3.1.0, 3.1.1, 3.1.2, 3.1.3, 3.1.5, 3.2.0]
```

Our favourite is `FTDebouncer`, let's install it by running:

```sh
$ arduino-cli lib install FTDebouncer
FTDebouncer depends on FTDebouncer@1.3.0
Downloading FTDebouncer@1.3.0...
FTDebouncer@1.3.0 downloaded
Installing FTDebouncer@1.3.0...
Installed FTDebouncer@1.3.0
```

## Using the `daemon` mode and the gRPC interface

Arduino CLI can be launched as a gRPC server via the `daemon` command.

The [client_example] folder contains a sample client code that shows how to interact with the gRPC server. Available
services and messages are detailed in the [gRPC reference] pages.

To provide observability for the gRPC server activities besides logs, the `daemon` mode activates and exposes by default
a [Prometheus](https://prometheus.io/) endpoint (http://localhost:9090/metrics) that can be fetched for metrics data
like:

```text
# TYPE daemon_compile counter
daemon_compile{buildProperties="",exportFile="",fqbn="arduino:samd:mkr1000",installationID="ed6f1f22-1fbe-4b1f-84be-84d035b6369c",jobs="0",libraries="",preprocess="false",quiet="false",showProperties="false",sketchPath="5ff767c6fa5a91230f5cb4e267c889aa61489ab2c4f70f35f921f934c1462cb6",success="true",verbose="true",vidPid="",warnings=""} 1 1580385724726

# TYPE daemon_board_list counter
daemon_board_list{installationID="ed6f1f22-1fbe-4b1f-84be-84d035b6369c",success="true"} 1 1580385724833
```

The metrics settings are exposed via the `metrics` section in the CLI configuration:

```yaml
metrics:
  enabled: true
  addr: :9090
```

[configuration documentation]: configuration.md
[client_example]: https://github.com/arduino/arduino-cli/blob/master/client_example
[grpc reference]: rpc/commands.md
[prometheus]: https://prometheus.io/

## Configuration keys

- `board_manager`
  - `additional_urls` - the URLs to any additional Boards Manager package index files needed for your boards platforms.
- `daemon` - options related to running Arduino CLI as a [gRPC] server.
  - `port` - TCP port used for gRPC client connections.
- `directories` - directories used by Arduino CLI.
  - `data` - directory used to store Boards/Library Manager index files and Boards Manager platform installations.
  - `downloads` - directory used to stage downloaded archives during Boards/Library Manager installations.
  - `user` - the equivalent of the Arduino IDE's ["sketchbook" directory][sketchbook directory]. Library Manager
    installations are made to the `libraries` subdirectory of the user directory.
- `library` - configuration options relating to Arduino libraries.
  - `enable_unsafe_install` - set to `true` to enable the use of the `--git-url` and `--zip-file` flags with
    [`arduino-cli lib install`][arduino cli lib install]. These are considered "unsafe" installation methods because
    they allow installing files that have not passed through the Library Manager submission process.
- `logging` - configuration options for Arduino CLI's logs.
  - `file` - path to the file where logs will be written.
  - `format` - output format for the logs. Allowed values are `text` or `json`.
  - `level` - messages with this level and above will be logged. Valid levels are: `trace`, `debug`, `info`, `warn`,
    `error`, `fatal`, `panic`.
- `metrics` - settings related to the collection of data used for continued improvement of Arduino CLI.
  - `addr` - TCP port used for metrics communication.
  - `enabled` - controls the use of metrics.
- `sketch` - configuration options relating to [Arduino sketches][sketch specification].
  - `always_export_binaries` - set to `true` to make [`arduino-cli compile`][arduino-cli compile] always save binaries
    to the sketch folder. This is the equivalent of using the [`--export-binaries`][arduino-cli compile options] flag.
- `updater` - configuration options related to Arduino CLI updates
  - `enable_notification` - set to `false` to disable notifications of new Arduino CLI releases, defaults to `true`

## Configuration methods

Arduino CLI may be configured in three ways:

1. Command line flags
1. Environment variables
1. Configuration file

If a configuration option is configured by multiple methods, the value set by the method highest on the above list
overwrites the ones below it.

If a configuration option is not set, Arduino CLI uses a default value.

[`arduino-cli config dump`][arduino-cli config dump] displays the current configuration values.

### Command line flags

Arduino CLI's command line flags are documented in the command line help and the [Arduino CLI command reference].

#### Example

Setting an additional Boards Manager URL using the [`--additional-urls`][arduino-cli global flags] command line flag:

```shell
$ arduino-cli core update-index --additional-urls https://downloads.arduino.cc/packages/package_staging_index.json
```

### Environment variables

All configuration options can be set via environment variables. The variable names start with `ARDUINO`, followed by the
configuration key names, with each component separated by `_`. For example, the `ARDUINO_DIRECTORIES_USER` environment
variable sets the `directories.user` configuration option.

On Linux or macOS, you can use the [`export` command][export command] to set environment variables. On Windows cmd, you
can use the [`set` command][set command].

#### Example

Setting an additional Boards Manager URL using the `ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS` environment variable:

```sh
$ export ARDUINO_BOARD_MANAGER_ADDITIONAL_URLS=https://downloads.arduino.cc/packages/package_staging_index.json
```

### Configuration file

[`arduino-cli config init`][arduino-cli config init] creates or updates a configuration file with the current
configuration settings.

This allows saving the options set by command line flags or environment variables. For example:

```sh
arduino-cli config init --additional-urls https://downloads.arduino.cc/packages/package_staging_index.json
```

#### File name

The configuration file must be named `arduino-cli`, with the appropriate file extension for the file's format.

#### Supported formats

`arduino-cli config init` creates a YAML file, however a variety of common formats are supported:

- [JSON]
- [TOML]
- [YAML]
- [Java properties file]
- [HCL]
- envfile
- [INI]

#### Locations

Configuration files in the following locations are recognized by Arduino CLI:

1. Location specified by the [`--config-file`][arduino cli command reference] command line flag
1. Current working directory
1. Any parent directory of the current working directory (more immediate parents having higher precedence)
1. Arduino CLI data directory (as configured by `directories.data`)

If multiple configuration files are present, the one highest on the above list is used. Configuration files are not
combined.

The location of the active configuration file can be determined by running the command:

```sh
arduino-cli config dump --verbose
```

#### Example

Setting an additional Boards Manager URL using a YAML format configuration file:

```yaml
board_manager:
  additional_urls:
    - https://downloads.arduino.cc/packages/package_staging_index.json
```

Doing the same using a TOML format file:

```toml
[board_manager]
additional_urls = [ "https://downloads.arduino.cc/packages/package_staging_index.json" ]
```

[grpc]: https://grpc.io
[sketchbook directory]: sketch-specification.md#sketchbook
[arduino cli lib install]: commands/arduino-cli_lib_install.md
[sketch specification]: sketch-specification.md
[arduino-cli compile]: commands/arduino-cli_compile.md
[arduino-cli compile options]: commands/arduino-cli_compile.md#options
[arduino-cli config dump]: commands/arduino-cli_config_dump.md
[arduino cli command reference]: commands/arduino-cli.md
[arduino-cli global flags]: commands/arduino-cli_config.md#options-inherited-from-parent-commands
[export command]: https://ss64.com/bash/export.html
[set command]: https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/set_1
[arduino-cli config init]: commands/arduino-cli_config_init.md
[json]: https://www.json.org
[toml]: https://github.com/toml-lang/toml
[yaml]: https://en.wikipedia.org/wiki/YAML
[java properties file]: https://en.wikipedia.org/wiki/.properties
[hcl]: https://github.com/hashicorp/hcl
[ini]: https://en.wikipedia.org/wiki/INI_file

This is the specification for Arduino sketches.

The programs that run on Arduino boards are called "sketches". This term was inherited from
[Processing](https://processing.org/), upon which the Arduino IDE and the core API were based.

## Sketch folders and files

The sketch root folder name and code file names must start with a basic letter (`A`-`Z` or `a`-`z`) or number (`0`-`9`),
followed by basic letters, numbers, underscores (`_`), dots (`.`) and dashes (`-`). The maximum length is 63 characters.

Support for names starting with a number was added in Arduino IDE 1.8.4.

### Sketch root folder

Because many Arduino sketches only contain a single .ino file, it's easy to think of that file as the sketch. However,
it is the folder that is the sketch. The reason is that sketches may consist of multiple code files and the folder is
what groups those files into a single program.

### Primary sketch file

Every sketch must contain a `.ino` file with a file name matching the sketch root folder name.

`.pde` is also supported but **deprecated** and will be removed in the future, using the `.ino` extension is strongly
recommended.

### Additional code files

Sketches may consist of multiple code files.

The following extensions are supported:

- .ino - [Arduino language](https://www.arduino.cc/reference/en/) files.
- .pde - Alternate extension for Arduino language files. This file extension is also used by Processing sketches. .ino
  is recommended to avoid confusion. **`.pde` extension is deprecated and will be removed in the future.**
- .cpp - C++ files.
- .c - C Files.
- .S - Assembly language files.
- .h - Header files.
- .tpp, .ipp - Header files (available from Arduino CLI 0.19.0).

For information about how each of these files and other parts of the sketch are used during compilation, see the
[Sketch build process documentation](sketch-build-process.md).

### `src` subfolder

The contents of the `src` subfolder are compiled recursively. Unlike the code files in the sketch root folder, these
files are not shown as tabs in the IDEs.

This is useful for files you don't want to expose to the sketch user via the IDE's interface. It can be used to bundle
libraries with the sketch in order to make it a self-contained project.

Arduino language files under the `src` folder are not supported.

- In Arduino IDE 1.6.5-r5 and older, no recursive compilation was done.
- In Arduino IDE 1.6.6 - 1.6.9, recursive compilation was done of all subfolders of the sketch folder.
- In Arduino IDE 1.6.10 and newer, recursive compilation is limited to the `src` subfolder of the sketch folder.

### `data` subfolder

The `data` folder is used to add additional files to the sketch, which will not be compiled.

Files added to the sketch via the Arduino IDE's **Sketch > Add File...** are placed in the `data` folder.

The Arduino IDE's **File > Save As...** only copies the code files in the sketch root folder and the full contents of
the `data` folder, so any non-code files outside the `data` folder are stripped.

### Metadata

Arduino CLI and Arduino Web Editor use a file named sketch.json, located in the sketch root folder, to store sketch
metadata.

The `cpu` key contains the board configuration information. This can be set via
[`arduino-cli board attach`](commands/arduino-cli_board_attach.md) or by selecting a board in the Arduino Web Editor
while the sketch is open. With this configuration set, it is not necessary to specify the `--fqbn` or `--port` flags to
the [`arduino-cli compile`](commands/arduino-cli_compile.md) or [`arduino-cli upload`](commands/arduino-cli_upload.md)
commands when compiling or uploading the sketch.

The `included_libs` key defines the library versions the Arduino Web Editor uses when the sketch is compiled. This is
Arduino Web Editor specific because all versions of all the Library Manager libraries are pre-installed in Arduino Web
Editor, while only one version of each library may be installed when using the other Arduino development software.

### Secrets

Arduino Web Editor has a
["Secret tab" feature](https://create.arduino.cc/projecthub/Arduino_Genuino/store-your-sensitive-data-safely-when-sharing-a-sketch-e7d0f0)
that makes it easy to share sketches without accidentally exposing sensitive data (e.g., passwords or tokens). The
Arduino Web Editor automatically generates macros for any identifier in the sketch which starts with `SECRET_` and
contains all uppercase characters.

When you download a sketch from Arduino Web Editor that contains a Secret tab, the empty `#define` directives for the
secrets are in a file named arduino_secrets.h, with an `#include` directive to that file at the top of the primary
sketch file. This is hidden when viewing the sketch in Arduino Web Editor.

### Documentation

Image and text files in common formats which are present in the sketch root folder are displayed in tabs in the Arduino
Web Editor.

### Sketch file structure example

```
Foo
|_ arduino_secrets.h
|_ Abc.ino
|_ Def.cpp
|_ Def.h
|_ Foo.ino
|_ Ghi.c
|_ Ghi.h
|_ Jkl.h
|_ Jkl.S
|_ sketch.json
|_ data
|  |_ Schematic.pdf
|_ src
   |_ SomeLib
      |_ library.properties
      |_ src
         |_ SomeLib.h
         |_ SomeLib.cpp
```

## Sketchbook

The Arduino IDE provides a "sketchbook" folder (analogous to Arduino CLI's "user directory"). In addition to being the
place where user libraries and manually installed platforms are installed, the sketchbook is a convenient place to store
sketches. Sketches in the sketchbook folder appear under the Arduino IDE's **File > Sketchbook** menu. However, there is
no requirement to store sketches in the sketchbook folder.

## Library/Boards Manager links

A URI in a comment in the form `http://librarymanager#SEARCH_TERM` will open a search for SEARCH_TERM in
[Library Manager](https://www.arduino.cc/en/guide/libraries#toc3) when clicked in the Arduino IDE.

A URI in a comment in the form `http://boardsmanager#SEARCH_TERM` will open a search for SEARCH_TERM in
[Boards Manager](https://www.arduino.cc/en/Guide/Cores) when clicked in the Arduino IDE.

This can be used to offer the user an easy way to install dependencies of the sketch.

This feature is only available when using the Arduino IDE, so be sure to provide supplementary documentation to help the
users of other development software install the sketch dependencies.

This feature was added in Arduino IDE 1.6.9.

### Example

```c++
// install the Arduino SAMD Boards platform to add support for your MKR WiFi 1010 board
// if using the Arduino IDE, click here: http://boardsmanager#SAMD

// install the WiFiNINA library via Library Manager
// if using the Arduino IDE, click here: http://librarymanager#WiFiNINA
#include <WiFiNINA.h>
```

## See also

- [Sketch build process documentation](sketch-build-process.md)
- [Style guide for example sketches](https://www.arduino.cc/en/Reference/StyleGuide)

## Boards

gustavocampos@GUSTAVOs-MBP .ssh % arduino-cli core list
ID              Installed Latest Name
arduino:avr     1.8.3     1.8.4  Arduino AVR Boards
arduino:megaavr 1.8.7     1.8.7  Arduino megaAVR Boards
arduino:sam     1.6.12    1.6.12 Arduino SAM Boards (32-bits ARM Cortex-M3)
arduino:samd    1.8.11    1.8.12 Arduino SAMD Boards (32-bits ARM Cortex-M0+)
esp32:esp32     1.0.6     1.0.6  ESP32 Arduino
esp8266:esp8266 3.0.2     3.0.2  ESP8266 Boards (3.0.2)
Maixduino:k210  0.3.11    0.3.11 Maixduino

## repos

https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json
https://dl.espressif.com/dl/package_esp32_index.json
http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
http://dl.sipeed.com/MAIX/Maixduino/package_Maixduino_k210_index.json




