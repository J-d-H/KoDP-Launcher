## King of Dragon Pass launcher
Scaled fullscreen launcher for King of Dragon Pass. Also search for blocking processes (SetPoint) and terminates/restarts them.

I've written this program because:
* KoDP didn't launch at my system.
* I hate how my second monitor view gets messed up, if resolution switches on my primary one. I like watching something besides playing turn based games and now I can do it while playing King of Dragon Pass.

#### Current Release
v0.2.1: https://github.com/J-d-H/KoDP-Launcher/releases/tag/v0.2.1

### Update: Simple Version

I just added a simpler version of the launcher, that only does he scaling after launching the game.
I recommend the KoDP-launcher-fullscreen-only.exe whenever possible, because it doesn't search through 
your running processes.

#### Installation
Just drop the KoDP-Launcher-fullscreen-only.exe inside your King of Dragon Pass installation directory.
Remove the "Launch with 640 x 480" setting from the compatibility tab of your KoDP.exe.
And you are ready to go.



### Blocking-Processes-Version

The version that searches blocking processes is still available in the master branch.

WARNING: This program may messes a lot with your processes running. Use at your own risk!

Initial idea comes from a Python script found at a gog.com forum post:
https://www.gog.com/forum/king_of_dragon_pass/can_i_play_this_windowed


#### Installation:
Just unpack into you King of Dragon Path folder.
Remove the "Launch with 640 x 480" setting from the compatibility tab of your KoDP.exe.


#### How it works (if it works)
At first it needs Administrator rights to mess around with your running processes.
Then it launches "KodP.exe -w" and searches for the main window.
If it does not find the window, it searches for processes that may be blocking the game (in my case it is Logitech SetPoint),
It asks you if you with to terminate those processes, if you with the launcher to try restarting terminated processes after your game quits and if you want those dissensions to be remembered.
Then it searches for the main window again...

If it found the window, it messes around with the window styles. And creates a new full screen borderless window.
It then renders the game into the new window.
To get the mouse working I had to move the game window around, don't know why...


#### What to do if it doesn't work
Please gather as much information as possible and create an Issue entry.


#### It broke my PC!
I'm sorry, please follow the instructions listed under "What to do if it doesn't work".

No, serriously: As far as I know, this programm will not harm your system in any way.
But I cannot take any responsibility in case it does, as I've only tested it on my system.
And I use it everytime I play King of Dragon Pass.
