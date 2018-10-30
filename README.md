# DIGITAiL
Development of the app to control our new breed of animatronic tails!

Notes!

Very basic words beneath - will improve this massively soon.

Basically you find the device by its ID, then you find its service (like its battery gauge) and the its characteristic (what's the power level?) and connect. And read the info, and show it, or write to it!

Device name is "(!)Tail1"

Device Service [0xffe0]

Device Characteristic [0xffe1]
All aspects of the Tail are controlled through this service and this characteristic

Red Power LED - A0 off A1 On
LEDs attached to tail PCB - L0-L9 to set brightness

Built in Moves

The remote will be literally sending a number to the tail to the right characteristic. And there will be 9 or 10 built in moves u can call up immediately.


Notifications

Notifications will do the same, once they are hooked in to the phone notification system

The most complex thing will be the move designer
And that is simply because I think it probably should be graphical:


Move Designer

Inside the pic, we will have a language to describe moves - direction, speed, height and number of repetitions - and that's it.

I've been imagining a screen with a tail viewed as if your looking back at the person wearing it. The tip of the tail is in the centre of the screen. With a mouse, you drag it one way or the other, and it "springs" back and the other way creating a pattern. If you want a shorter move, you just let it go a shorter distance. Or a higher way, drag it higher. See what i mean? And it has a kind of gravity keeping it moving until it comes to rest. Something like that.

Dont worry about that. First up is simply a screen to replace a remote - 9 buttons, each sending a number to the tail to call up its built in moves. And thats easy. Once you have the bt connection, you send a number to a service!
