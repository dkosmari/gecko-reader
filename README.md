# Gecko Reader

This is a Wii homebrew app to read data from a USB Gecko.

The USB Gecko must be connected to slot A. The data is printed to the screen and also
broadcast to UDP port 4405; you can receive the UDP broadcast using `udplogserver`.


# Usage

Press HOME or START to quit the app.

Pressing 1/X will enable translation of end-of-line characters, from `\r` to `\n`. Most
terminals expect `\n` to be the EOL character.


## Build instructions

If you obtained the source through a tarball, you can skip step 0.

0. `./bootstrap`

1. `./configure --host=powerpc-eabi`

2. `make`

3. `make zip`

The resulting `gecko-reader.zip` is installable with `wiiload`.
