# Gecko Reader

This is a Wii homebrew app to read and print data from a USB Gecko.

The USB Gecko must be connected to slot A.

This app will also listen for TCP connections on port 4405. This connection is
bi-directional; any data sent to this port will be written to the USB Gecko.


## Usage

Press HOME or START to quit the app.

Pressing 1/X will toggle translation of end-of-line characters, from `\r` to `\n`. Most
terminals expect `\n` to be the EOL character.


### Example: accessing the COS Shell on a Wii U

Assuming you have the `socat` package installed, run:

    socat READLINE TCP:wii:4405

Where `wii` is the address of your Wii console on your local network.


## Build instructions

If you obtained the source through a tarball, you can skip step 0.

0. `./bootstrap`

1. `./configure --host=powerpc-eabi`

2. `make`

3. `make zip`

The resulting `gecko-reader.zip` is installable with `wiiload`.
