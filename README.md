cHat
====

This is a simple chat client with video and audio chat options.

It is currently early alpha and requires two dynamic libraries to be installed:
OpenCV --> http://opencv.org
portaudio --> http://www.portaudio.com

The whole thing is written in C and should ideally compile on any system with those libraries installed.


Compile chat client (chat) with 
``gcc chat.c -o chat -lncurses -portaudio `pkg-config --cflags --libs opencv` ``

Compile server (chatd) with
`gcc chatd.c -o chatd`

Compile video server (chatd-video) with
`gcc chatd-video.c -o chatd-vid`


====
Plans
====

- Eliminate hardcoded IPs (it was made a hackathon, give us a break)
- Stabilize video refreshing
- Standardize to P2P (already done in audio)
- Write a standardized protocol for sending color data along with image data
- Responsive video display
- Decentralized userbase/discovery
- Encryption

