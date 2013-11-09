cHat
====

This is a simple chat client with video and audio chat options.

Compile chat client (chat) with 
``gcc chat.c -o chat -lncurses `pkg-config --cflags --libs opencv` ``

Compile server (chatd) with
`gcc chatd.c -o chatd`
