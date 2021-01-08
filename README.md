# CRUMPET - The Brains For Your Tail Company Gear
Development of the app to control our new breed of animatronic gear, from tails to ears, and everything inbetween!

We are currently editing files to be less DIGITAiL only..

For information on the basic concepts of the application's codebase, and for information on how you should approach the code if you wish to contribute, please see our [HACKING document](HACKING.md).

## EarGear

Please see separate document entitled "EarGear Protocol" for full details

## DIGITAiL NOTES

You find the device by its ID, then you find its service (like its battery gauge) and the its characteristic (what's the power level?) and then Connect. And read the info, and show it, or write to it!

Device name is "(!)Tail1" 

Device Service [0xffe0]

Device Characteristic [0xffe1]

All aspects of the Tail are controlled through this service and this characteristic.
The app will send a text string to the tail to the right characteristic. 
And there are 11 built in moves you can call up immediately.

*YOU CANNOT USE THE TAIL WHILE IT IS BEING POWERED BY THE POWER ADAPTOR*

## BUILT IN MOVES

### TAIL MOVES AND DURATION
```
TAILS1 - SLOW_WAG1 11.53 secs
TAILS2 - SLOW_WAG2 7.10 secs
TAILS3 - SLOW_WAG3 8.50 secs
TAILFA - FAST_WAG 9.96 secs
TAILSH - SHORT_WAG 7.46 secs
TAILHA - HAPPY_WAG 8.86 secs
TAILER - ERECT 5.80 secs
TAILEP - ERECT_PULSE 9.76 secs
TAILT1 - TREMBLE1 4.06 secs
TAILT2 - TREMBLE2 5.55 secs
TAILET - ERECT_TREM 4.73 secs
```
and a Home Move: TAILHM

There are 6 built in LED patterns.

### LED PATTERNS
```
LEDREC - intermittent (on/off)
LEDTRI - triangular (fade in/out)
LEDSAW - sawtooth (fade in/off)
LEDSOS - Morse SOS
LEDFLA - flame simulation
LEDSTR - strobo
```
and an Leds off command: LEDOFF

### OTHER COMMANDS
```
VER - returns the firmware version number
PING - returns OK
SHUTDOWN - powers off the unit (will lose the connection!)
BATT - returns Bn (n= 0,1,2,3,4 number of 'bars')
USERMOVE and USERLEDS which deal with user created moves or LED patterns
```
Also, commands are case insensitive

Note about Battery Level: the batteries are read charge-wise only when the app asks. There is a super-quick flash of any connected LEDS when the reading is done. The charge level is returned BATTn with n showing charge. If the Tail determines the charge is low, the tail will flash the internal red led.

We will split the app in to two parts from the Users point of view.



## Move Groups

Slower, gentler, relaxed:
```
Slow Wag 1
Slow Wag 2
Slow Wag 3
```

Faster, happier, excited:
```
Fast Wag
Short Wag
Happy Wag
Erect
```

Angry, frustrated, tense:
```
Tremble 1
Tremble 2
Tremble Erect
Pulse Erect
```


## MOVE PLAYLIST

A simple way of chaining pre-defined moves together. We need a pause in between moves
which serves both to make the system cooler, and also more natural.

Pauses defined as a range, so that they are of varying length.

I suggest we auto-generate or preset 4 playlists - a slow gentler one, a faster, excited one, an angry, frustrated one, and a list with all moves in. These could be a predefined list. But it would be nice if we randomise them every time, including pauses.

## CASUAL MODE

This will be the go-to mode for people who wear their Tail a lot - at a convention or around the house! Just click the top level button to string together calm and energetic moves, with a random duration pauses in between. No other input required

We are creating Casual Mode by sending the moves at the appropriate time. However in the latest firmware, it is possible to let the tail receive on composite command to do this itself. Please see the Protocol Document for syntax.

## MOVE DESIGNER - As yet uncreated

See the Protocol Doc for methods of describing moves, and how to send them to the Tail.

I've been imagining a screen with a tail viewed as if your looking back at the person wearing it. The tip of the tail is in the centre of the screen. With a mouse, you drag it one way or the other, and it "springs" back and the other way creating a pattern. If you want a shorter move, you just let it go a shorter distance. Or a higher way, drag it higher. See what i mean? And it has a kind of gravity keeping it moving until it comes to rest. Something like that.

## NOTIFICATIONS

Notifications will available once we are hooked in to the phone notification system
This is not part of the project until the other two modes are complete.



# How To Compile The App

Building the app is a multi step process, so please make sure you do the steps in the right order.

## Getting The Source

To clone the [repository](https://github.com/MasterTailer/CRUMPET.git), you must also remember to clone the submodules. Using the git command line this would be done like so:

```
git clone --recursive (repository URL goes here) $HOME/DIGITAiL
```

If you have already cloned the repository before reading this, you will need to initialise the submodules. Again using the command line, this would be done like so:

```
cd DIGITAiL
git submodule update --init --recursive
```

## Building

### Windows

(this will need a touch of help from someone more used to working on Windows)

### Linux

These instructions assume you are building on the command line. You are entirely able to build things using an IDE (such as KDevelop), but the command line instructions give the most terse set of steps, and it should be possible to extract needed information for building using your favourite graphical tools from the listings below.

Once you have cloned the source code, you will need a few bits of software to be able to build and run the app.

- Basic C++ building tools
- CMake 3.2 or later
- Extra Cmake Modules version 5.68 for [KDE/kirigami](https://github.com/KDE/kirigami) or later
- Development packages for [Qt](https://wiki.qt.io/Install_Qt_5_on_Ubuntu) version 5.12 or later

#### For openSUSE Tumbleweed:

The following lines will install the packages you need:

```
sudo zypper install --type pattern devel_C_C++ devel_qt5
sudo zypper install extra-cmake-modules
sudo zypper install libQt5QuickControls2-devel libqt5-qtbase-private-headers-devel
```

#### For Ubuntu Focal Fossa 

You should install at least the following packages:

```
sudo apt install g++ # or sudo apt install build-essential
```

```
sudo apt install cmake
```

```
sudo apt install extra-cmake-modules # ECM
```

```
sudo apt install qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev qtquickcontrols2-5-dev libqt5svg5-dev libqt5bluetooth5 qtconnectivity5-dev libqt5svg5-dev qtbase5-private-dev libqt5remoteobjects5-dev libqt5remoteobjects5-bin
```


Once these have been installed, you are ready to build. You should always build software out-of-source, which means creating a directory somewhere and performing the actual building there. Enter the clone directory as created above by the clone command. Unless you have moved around the filesystem, you should already be in the directory containing the source code.

```
mkdir build
cd build
cmake .. #For successfull CMake on Ubuntu 20.04 need to override ECM version in $HOME/DIGITAiL/3rdparty/kirigami/CMakeLists.txt as "find_package(ECM 5.68.0 NO_MODULE) #find_package(ECM 5.72.0 NO_MODULE)"
make
```

If any errors occur during the cmake step, you may find that you have either not followed the steps above correctly, or that you are missing some piece of software or another required for the build. The way to fix this will vary depending on your distribution.

You may also notice some warnings during the CMake configuration step about policy changes. Don't worry about those, they're less dangerous than they seem.

Once the make command has completed successfully, you should find yourself with a binary named digital inside the src subdirectory of your build directory. As the app is split into two (a service part and the visible app which controls that service), you can run this by running the following two commands, each in their separate shell:


These are runtime requirements:

```
sudo apt install qml-module-qt-labs-calendar qml-module-qtmultimedia
```

```
./bin/digitail -service
./bin/digitail
```

You should now have a nice little app show up on your desktop which suggests the ability to control tails.

### Android APK

Building the Android APK is done most straightforwardly by using the pre-prepared Docker image provided by the KDE community, in which all the tools are already installed for you.

To do so, run the following command for the 32bit docker

```
sudo apt install docker.io
sudo chmod 666 /var/run/docker.sock
sudo chown $USER:$USER $HOME/apks
sudo chmod 777 $HOME/apks
```

```
docker run -ti --rm kdeorg/android-sdk bash
```

This will download the image the first time it is run, and subsequently it will simply run what you already did.

A handy trick is to also add something like `-v $HOME/apks:/output` to the command line, which will then add a directory named output at the root of the docker instance, pointed at the apks directory in your home folder. This will allow you to fairly easily transfer the apk onto your host system from inside docker (the switch above would link the directory /output inside the docker to the apks directory in your home directory, which does need to already exist).

Also you can share source directory from your host machine, for example:

```
docker run -ti --rm -v $HOME/apks:/output -v $HOME/DIGITAiL:/DIGITAiL kdeorg/android-sdk bash
```

Cloning is done as in a usual Linux situation (see above), but your build steps are a little bit more involved here:

```
mkdir build-arm
cd build-arm
export ANDROID_ARCH_ABI=armeabi-v7a
cmake \
    -DCMAKE_TOOLCHAIN_FILE=/opt/nativetooling/share/ECM/toolchain/Android.cmake \
    -DCMAKE_INSTALL_PREFIX="/opt/kdeandroid-arm" \
    -DECM_ADDITIONAL_FIND_ROOT_PATH=/opt/Qt \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=../export \
    -DQTANDROID_EXPORTED_TARGET=digitail \
    -DANDROID_APK_DIR=../data \
    -DANDROID_EXTRA_LIBS=/opt/kdeandroid-arm/lib/libcrypto.so,/opt/kdeandroid-arm/lib/libssl.so \
    -DCMAKE_ANDROID_API=23 \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DKF5_HOST_TOOLING=/opt/nativetooling/lib/x86_64-linux-gnu/cmake/ \
    ..
make
cd src # This ensures we only install the digitail binary and not the Kirigami bits
make install
cd ..
make create-apk-digitail ARGS="--android-platform android-30"
```

The above will build for 32bit arm. To build for 64bit arm, use the lines below instead (the cmake line is identical apart from three instances of arm becoming arm64)

```
mkdir build-arm64
cd build-arm64
export ANDROID_ARCH_ABI=arm64-v8a
cmake \
    -DCMAKE_TOOLCHAIN_FILE=/opt/nativetooling/share/ECM/toolchain/Android.cmake \
    -DCMAKE_INSTALL_PREFIX="/opt/kdeandroid-arm64" \
    -DECM_ADDITIONAL_FIND_ROOT_PATH=/opt/Qt \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=../export \
    -DQTANDROID_EXPORTED_TARGET=digitail \
    -DANDROID_APK_DIR=../data \
    -DANDROID_EXTRA_LIBS=/opt/kdeandroid-arm64/lib/libcrypto.so,/opt/kdeandroid-arm64/lib/libssl.so \
    -DCMAKE_ANDROID_API=23 \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DKF5_HOST_TOOLING=/opt/nativetooling/lib/x86_64-linux-gnu/cmake/ \
    ..
make
cd src # This ensures we only install the digitail binary and not the Kirigami bits
make install
cd ..
make create-apk-digitail ARGS="--android-platform android-30"
```

Once this final command completes, you should hopefully have an apk in `/home/user/DIGITAiL/build/digitail_build_apk/build/outputs/apk/debug/digitail_build_apk-debug.apk` (or where ever else you created your clone).

If you added the `-v` bit to your docker command, then you can copy that file to your local machine by doing e.g.

`cp $HOME/build/digitail_build_apk/build/outputs/apk/debug/digitail_build_apk-debug.apk /output/`

and then looking in your apks folder using whatever method you usually use.

You now have an apk, which you can install to your android device in the usual way (and which that is will depend on you, though `adb install apkfile` usually does the trick).

To also sign the apk for use on the Play store, the following command might be used. Note that you will need the appropriate keystore to hand in an appropriate location, or it will quite entirely fail:

```
make create-apk-digitail ARGS="--android-platform android-30 --sign /mnt/projects-dir/DIGITAiL/thetailcompany-release-key.keystore thetailcompany"
```
