DIGITAiL
Development of the app to control our new breed of animatronic tails!

Notes!

Very basic words beneath - will improve this massively soon.

Basically you find the device by its ID, then you find its service (like its battery gauge) and the its characteristic (what's the power level?) and connect. And read the info, and show it, or write to it!

Device name is "(!)Tail1"

Device Service [0xffe0]

Device Characteristic [0xffe1]
All aspects of the Tail are controlled through this service and this characteristic

(Used to exist, but not now: Red Power LED - A0 off A1 On, LEDs attached to tail PCB - L0-L9 to set brightness)

Built in Moves

The remote will be literally sending a number to the tail to the right characteristic. And there will be 9 or 10 built in moves u can call up immediately.
"S1",&SLOW_WAG1,
"S2",&SLOW_WAG2,
"S3",&SLOW_WAG3,
"FA",&FAST_WAG,
"SH",&SHORT_WAG,
"HA",&HAPPY_WAG,
"ER",&ERECT,
"EP",&ERECT_PULSE,
"T1",&TREMBLE1,
"T2",&TREMBLE2,
"ET",&ERECT_TREM,

"LO", OFF,
"LR", BLINK,
"LT", TRIANGULAR PATTERN (intensity up and down)
"LS", SAW TOOTH (RAMP UP) PATTERN (intensity up then off)

"SHUTDOWN" switches off the tail.


Notifications

Notifications will do the same, once they are hooked in to the phone notification system

The most complex thing will be the move designer
And that is simply because I think it probably should be graphical:


Move Designer

Inside the pic, we will have a language to describe moves - direction, speed, height and number of repetitions - and that's it.

I've been imagining a screen with a tail viewed as if your looking back at the person wearing it. The tip of the tail is in the centre of the screen. With a mouse, you drag it one way or the other, and it "springs" back and the other way creating a pattern. If you want a shorter move, you just let it go a shorter distance. Or a higher way, drag it higher. See what i mean? And it has a kind of gravity keeping it moving until it comes to rest. Something like that.

Dont worry about that. First up is simply a screen to replace a remote - 9 buttons, each sending a number to the tail to call up its built in moves. And thats easy. Once you have the bt connection, you send a number to a service!

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
