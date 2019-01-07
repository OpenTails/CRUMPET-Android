DIGITAiL
Development of the app to control our new breed of animatronic tails!

NOTES

You find the device by its ID, then you find its service (like its battery gauge) and the its characteristic (what's the power level?) and then Connect. And read the info, and show it, or write to it!

Device name is "(!)Tail1"

Device Service [0xffe0]

Device Characteristic [0xffe1]
All aspects of the Tail are controlled through this service and this characteristic

BUILT IN MOVES

The remote will send a text string to the tail to the right characteristic. And there are 11 built in moves u can call up immediately.

TAIL MOVES and their Duration

TAILS1 - SLOW_WAG1 9.5 secs
TAILS2 - SLOW_WAG2 6 secs
TAILS3 - SLOW_WAG3 7.8 secs
TAILFA - FAST_WAG 9 secs
TAILSH - SHORT_WAG 6.8 secs
TAILHA - HAPPY_WAG 16 secs
TAILER - ERECT (Cant test, bug in firmware)
TAILEP - ERECT_PULSE (cant test, bug in firmware)
TAILT1 - TREMBLE1 4 secs
TAILT2 - TREMBLE2 4.8 secs
TAILET - ERECT_TREM 4.6 secs

and a Home Move: TAILHM

There are 6 built in LED patterns

LED PATTERNS----------

LEDREC - intermittent (on/off)
LEDTRI - triangular (fade in/out)
LEDSAW - sawtooth (fade in/off)
LEDSOS - Morse SOS
LEDFLA - flame simulation
LEDSTR - strobo

and an Leds off command: LEDOFF

OTHER COMMANDS-------

VER - returns the firmware version number
PING - returns OK
SHUTDOWN - powers off the unit (will lose the connection!)
BATT - returns Bn (n= 0,1,2,3,4 number of 'bars')
USERMOVE and USERLEDS which deal with user created moves or LED patterns

Also, commands are now case insensitive

We will split the app in to two parts from the Users point of view.

NORMAL MODE

This will have the Remote Control page (for moves and LEDs) as above.
It will also have the Move playlist page and Pose Mode page.

In this mode, moves cant be interrupted. All moves are followed by the homing move.

ADVANCED MODE

This will include the Move Designer page. It might also allow some different settings

Perhaps the advanced mode is unlocked after a splash screen communicating the
potential for harm to the Tail if used inappropriately. Should we have a test mode
that logs moves? For diagnosis, edge cases, etc?

REMOTE CONTROL


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

A screen toggle for a second page to bring up the LED remote control buttons.

POSE MODE

Poses are static shapes that we can define for the Tailer, so that they can strike a pose
and it will hold it, for the all important photo. These can be described using
Move Designer notation, and sent on button press.

MOVE PLAYLIST

A simple way of chaining pre-defined moves together. We need a pause in between moves
which serves both to make the system cooler, and also more natural.

Pauses could be defined as a range, so that they are of varying length.

I suggest we auto-generate or preset 4 playlists - a slow gentler one, a faster, excited one, an angry, frustrated one, and a list with all moves in
These could be a predefined list. But it would be nice if we randomise them every time, including pauses.

MOVE DESIGNER

See the separate doc for methods of describing moves, and how to send them to the Tail.

I've been imagining a screen with a tail viewed as if your looking back at the person wearing it. The tip of the tail is in the centre of the screen. With a mouse, you drag it one way or the other, and it "springs" back and the other way creating a pattern. If you want a shorter move, you just let it go a shorter distance. Or a higher way, drag it higher. See what i mean? And it has a kind of gravity keeping it moving until it comes to rest. Something like that.

NOTIFICATIONS

Notifications will available once we are hooked in to the phone notification system
This is not part of the project until the other two modes are complete.

The most complex thing will be the move designer, from a technical and graphical
point of view.

To Do

Remove LED word from each LED pattern. Move the LED OFF button to below the LED section.
Add a nice green RGB 163,210,134 behind the calm moves, a nice blue 0,192,220 behind the excited ones and a red one 251,107,70 behind the angry ones
Add a "DIGITAiL designed and created by The Tail Company - http://thetailcompany.com" on the about page
Add a "VER" call on the about page
Add a "BATT" call somewhere!

# How To Compile The App

Building the app is a multi step process, so please make sure you do the steps in the right order.

## Getting The Source

To clone the repository, you must also remember to clone the submodules. Using the git command line this would be done like so:

```
git clone --recursive (clone URL goes here)
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
- Extra Cmake Modules version 5.52 or later
- Development packages for Qt version 5.11 or later

For openSuse Tumbleweed, the following lines will install the packages you need:

```
sudo zypper install --type pattern devel_C_C++ devel_qt5
sudo zypper install extra-cmake-modules
```

Once these have been installed, you are ready to build. You should always build software out-of-source, which means creating a directory somewhere and performing the actual building there. Enter the clone directory as created above by the clone command. Unless you have moved around the filesystem, you should already be in the directory containing the source code.

```
mkdir build
cd build
cmake ..
make
```

If any errors occur during the cmake step, you may find that you have either not followed the steps above correctly, or that you are missing some piece of software or another required for the build. The way to fix this will vary depending on your distribution.

You may also notice some warnings during the CMake configuration step about policy changes. Don't worry about those, they're less dangerous than they seem.

Once the make command has completed successfully, you should find yourself with a binary named digital inside the src subdirectory of your build directory. You can run this by doing as follows:

```
./bin/digitail
```

You should now have a nice little app show up on your desktop which suggests the ability to control tails.

### Android APK

Building the Android APK is done most straightforwardly by using the pre-prepared Docker image provided by the KDE community, in which all the tools are already installed for you.

To do so, run the command

```
docker run -ti --rm kdeorg/android-sdk bash
```

This will download the image the first time it is run, and subsequently it will simply run what you already did.

A handy trick is to also add something like "-v $HOME/apks:/output" to the command line, which will then add a directory named output at the root of the docker instance, pointed at the apks directory in your home folder. This will allow you to fairly easily transfer the apk onto your host system from inside docker.

Cloning is done as in a usual Linux situation (see above), but your build steps are a little bit more involved here:

```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=/opt/kdeandroid-deps/share/ECM/toolchain/Android.cmake -DECM_ADDITIONAL_FIND_ROOT_PATH=/opt/Qt/5.11.0/android_armv7 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../export -DQTANDROID_EXPORTED_TARGET=digitail -DANDROID_APK_DIR=../data ..
make
cd src # This ensures we only install the digitail binary and not the Kirigami bits
make install
cd ..
make create-apk-digital
```

Once this final command completes, you should hopefully have an apk in /home/user/DIGITAiL/build/digitail_build_apk/build/outputs/apk/debug/digitail_build_apk-debug.apk (or where ever else you created your clone).

If you added the -v bit to your docker command, then you can copy that file to your local machine by doing e.g. "cp /home/user/DIGITAiL/build/digitail_build_apk/build/outputs/apk/debug/digitail_build_apk-debug.apk /output", and then looking in your apks folder using whatever method you usually use.

You now have an apk, which you can install to your android device in the usual way (and which that is will depend on you, though "adb install apkfile" usually does the trick).
