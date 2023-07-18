How to build
============

This document provides instructions for building the Suika2 application from its source. If you encounter any unexpected behaviour, please contact the
Team for further instructions and assistance. 

If you encounter any problems or unexpected behaviour, please open an issue via GitHub, e-mail `midori@suika2.com`, or join our Discord from the main Suika2 repository.

## Getting the Source Code
Firstly, you have to clone the Suika2 repository using `Git`.

To retain symbolic links on Windows, you have to use `Git for Windows`.
Check `Enable symbolic links` during the installation.

* On Windows:
  * Run `Git Bash` as **admin** (If you do not use admin, symbolic links cannot be created)
  * Run `git clone -c core.symlinks=true https://github.com/ktabata/suika2.git`
* On other platforms:
  * From the terminal, run the following command:
    * Run `git clone https://github.com/ktabata/suika2.git`

## Docker Build
If you are a user of `Docker`, you can build Windows, Linux, Web, and Android binaries in a single step.

* On Windows:
  * Double click `build/docker/build.bat`.
* On other platforms:
  * From the terminal, navigate to the `build/docker` directory and run the following command:
    * `./build.sh`

## Windows Binary from source
This method will utilise a cross compiler to build a Windows binary.

* On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
  * `build-essential`
  * `mingw-w64`
* Alternatively, on macOS 13, install `Homebrew` and the following package:
  * `mingw-w64`
* From the terminal, navigate to the `build/mingw` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `make` to build `suika.exe`.
  * Run `make install` to copy `suika.exe` to the suika2 directory.
* Sign `suika.exe` with your own certificate if you require it.

To run Suika2, copy `suika.exe` to either the `game-en` or `game-jp` folder.

## macOS Binary from Source
This method will utilise Xcode to build a macOS binary.

* On macOS 13, install Xcode 14.
* From the terminal, navigate to the `build/macos` directory and run the following command:
  * Run `./build-libs.sh` to build the libraries.
* From Xcode, open `build/macos` and complete the following steps:
  * Set your development team.
  * Build the project.
  * Archive the project.
  * Notarize the application with the `Distribute App` button.
  * Select the `Export Notarized App` option to export the app to the suika2 folder.

**Note:** Projects will likely fail to build on Apple Intel machines, presenting the following errors:
* `'png.h' file not found`
* `Could not read serialized diagnostics file: error("Failed to open diagnostics file")`

Building from an Apple Silicon machine should alleviate these errors. However, if this is not viable, [download them here (starts download)](https://suika2.com/dl/libroot-mac.tar.gz).

To run Suika2, copy `suika` to either the `game-en` or `game-jp` folder.

## iOS
This method will utilise Xcode to build an iOS application.

* On macOS 13, install Xcode 14.
* From the terminal, navigate to the `build/ios` directory and run the following command:
  * Run `./build-libs.sh` to build the libraries.
* Alternatively, run `./build-libs-sim.sh` to build the libraries for use with simulators on Apple Silicon Mac.
* From Xcode, open `build/ios` and complete the following steps:
  * Navigate to the `Signing & Capabilities` tab and select `Automatically Manage Signing`.
  * Connect the iOS device via cable and build the project for the device.
  * Run the application from your iOS device.
  * Replace `build/ios/suika/data01.arc` with your own `data01.arc` file.

Run the application.

## Android (Docker)
This method will utilise `Docker Desktop` on Windows 10 and 11.

* (First time only:) Install `Docker Desktop`.
* Double click `build/docker/build.bat`.

To run Suika2, upload `build/docker/suika.apk` on the Web and download it for your Android device.

## Android (Android Studio)
This method will utilise `Android Studio` to build an Android application.

* Install `Android Studio`.
* From the terminal, navigate to the `build/android` directory and run the following command:
  * Run `./prepare-libs.sh` to decompress the libraries.
* Open the Suika2 project from `build/android`.
* Build the project.

Run the application from your device or an emulator.

## Web
This method will utilise the Emscripten framework to build a web distribution.

* Prerequisites
  * Use a UNIX-like environment such as MSYS2, Linux, or macOS.
  * Ensure you can access `make` and `python`.
  * Install `Emscripten` using the `emsdk` command.
  * Generate a `data01.arc` using `suika-pro.exe`.
    * Using [Suika2 Pro for Creators](https://github.com/suika2engine/suika2/wiki/5.-Suika2-Pro-for-Creators), select `File –> Export Package`.

* Instructions
  * From the terminal, navigate to the `build/emscripten` directory and run the following commands:
    * Run `make`
    * Copy `data01.arc` into `build/emscripten/html/`
    * Run `make run`
* From a browser, navigate to `http://localhost:8000/html/`.

## Release Files
This method will create a release-ready ZIP file.

* From the terminal, navigate to the `build/release` directory and complete the following steps:
  * Modify the `SIGNATURE` stored in the `Makefile` to sign the `.dmg` files.
  * Run `make` to create ZIP files.
  * Rename `suika-2.x.x-en.zip` and `suika-2.x.x-jp.zip` to the desired application names.

## Release Files (Web)
This method will utilise the Emscripten framework to create a release-ready ZIP file for web distributions.

* From the terminal, navigate to the `build/emscripten` directory and run the following command:
  * Run `make`
* From the terminal, navigate to the `build/web-kit` directory and run the following command:
  * Run `make` to create ZIP files.
* Rename `suika-2.x.x-en.zip` and `suika-2.x.x-jp.zip` to the desired application names.

## Linux Binary (x86_64)
This method will build a Linux binary.

* On Ubuntu 22.04, install the following packages:
  * `build-essential`
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`

* From the terminal, navigate to the `build/linux-x86_64` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `make` to build the Suika2 binary.
  * Run `make install` to copy the suika binary to the suika2 directory.

* To run Suika2, copy `suika` to either the `game-en` folder or `game-jp` folder.
* From the terminal, navigate to the `game-en` or `game-jp` directory and run the following command:
  * Run `./suika`

## Raspberry Pi Binary
This method will build a Raspberry Pi binary.

* On Raspberry Pi OS, install the following packages:
  * `libasound2-dev`
  * `libx11-dev`
  * `libxpm-dev`
  * `mesa-common-dev`
  * `libgstreamer1.0-dev`
  * `libgstreamer-plugins-base1.0-dev`
* From the terminal, navigate to the `build/linux-arm` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `make` to build the Suika2 binary.
  * Run `make install` to copy the suika binary to the Suika2 directory.

* To run Suika2, copy `suika` to either the `game-en` folder or `game-jp` folder.
* From the terminal, navigate to the `game-en` or `game-jp` directory and run the following command:
  * Run `./suika`

## FreeBSD Binary
This method will build a FreeBSD binary.

* On FreeBSD 12, install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins`
* From the terminal, navigate to the `build/freebsd` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `gmake` to build the Suika2 binary.
  * Run `gmake install` to copy the suika binary to the suika2 directory.

* To run Suika2, copy `suika` to either the `game-en` folder or `game-jp` folder.
* From the terminal, navigate to the `game-en` or `game-jp` directory and run the following command:
  * Run `./suika`

## NetBSD Binary
This method will build a NetBSD binary.

* On NetBSD 9, install the following packages:
  * `gmake`
  * `alsa-lib`
  * `alsa-plugins-oss`
* Export `LD_LIBRARY_PATH=/usr/pkg/lib:/usr/X11R7/lib`
* From the terminal, navigate to the `build/netbsd` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `gmake` to build the Suika2 binary.
  * Run `gmake install` to copy the suika binary to the suika2 directory.

* To run Suika2, copy `suika` to either the `game-en` folder or `game-jp` folder.
* From the terminal, navigate to the `game-en` or `game-jp` directory and run the following command:
  * Run `./suika`

  **Note:** To setup `ALSA/OSS`, create `/etc/asound.conf` and copy the following code to the file:

  ```
  pcm.!default {
    type oss
    device /dev/audio
  }
  ctl.!default {
    type oss
    device /dev/mixer
  }
  ```

## Switch Binary  
This method will build a Switch binary.

* Install [devkitpro](https://devkitpro.org/wiki/Getting_Started)  
* Add env `DEVKITPRO`, for example `export DEVKITPRO=/opt/devkitpro`  
* Run `sudo dkp-pacman -S switch-dev switch-portlibs`
* in dir `build/switch`
  * use `make swika.nro` to build nro file (can be loaded by hbmenu)
  * use `make debug SWITH_IP=192.168.xx.xx` to run the app for debug  

Also you can use docker to compile:

``` shell
docker pull devkitpro/devkita64
docker run -d -it --rm -v $(pwd):/project --name devkita64_run devkitpro/devkita64
docker exec -i devkita64_run bash -c "cd /project/build/switch && make -j2"
```  

## Windows Capture Binary from source
`suika-capture.exe` records all user interactions and periodic/interactive screenshots.
One can make a bug report with a reproduction procedure using `suika-capture.exe`.

In addition, we can make test cases with `suika-capture.exe` and replay them
with `suika-replay` on Linux.

This method will utilise a cross compiler to build a Windows capturer binary.

* On Ubuntu 22.04 (WSL2 is acceptable), install the following packages:
  * `build-essential`
  * `mingw-w64`
* From the terminal, navigate to the `build/mingw-capture` directory and run the following commands:
  * Run `./build-libs.sh` to build the libraries.
  * Run `make` to build `suika-capture.exe`.

To run Suika2 capturer, copy `suika-capture.exe` to either the `game-en` or `game-jp` folder.
Result data will be stored in `record` folder.
