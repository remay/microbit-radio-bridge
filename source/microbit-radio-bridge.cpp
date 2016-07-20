
/*
The MIT License (MIT)

Copyright (c) 2016 Robert May.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include "MicroBit.h"

MicroBit uBit;

const char * const radio_waves ="\
    000,000,000,000,000,  000,000,000,000,000,  000,000,255,255,255\n\
    000,000,000,000,000,  000,000,000,000,000,  000,255,000,000,000\n\
    000,000,000,000,000,  000,000,000,255,255,  255,000,000,255,255\n\
    000,000,000,000,000,  000,000,255,000,000,  255,000,255,000,000\n\
    000,000,000,000,255,  000,000,255,000,255,  255,000,255,000,255\n";

MicroBitImage radio(radio_waves);


/*
  Sends any received datagrams through the serial line
*/

void onData(MicroBitEvent) {
    ManagedString message = uBit.radio.datagram.recv();
    uBit.serial.send(message + "\n");
}

/*
  On receiving data through the serial line, check it looks like a radio datagram and transmit it
*/

void reader() {
    while(1) {
        ManagedString incoming = uBit.serial.readUntil("\n");

        if(incoming.length() < 32) {
            uBit.radio.datagram.send(incoming);
        }
    }
}


int main() {
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.radio.enable();

    // Initialise the micro:bit listeners for radio datagrams.
    // XXX Should probably move this until after all initialisation and we're
    // ready to process incoming messages
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM, onData);

    // Sets the group to an arbitrary number (69 in this case) to avoid interference
    // Might be good to be able to set the group via serial?
    uBit.radio.setGroup(69);

    // Use the highest output put level on the radio, to increase range and reliability.
    uBit.radio.setTransmitPower(7);

    // Increase the receive buffer size on our serial port, to be at least the same size as
    // a packet. This guarantees correct parsing of packets.
    // XXX Do we need to do the same for the tx buffer?
    uBit.serial.setRxBufferSize(32);

    // Run a short animaiton at power up.
    uBit.display.animateAsync(radio, 500, 5, 0, 0);

    // Creates a new fiber that listens for incoming serial signals
    create_fiber(reader);

    // Get into powersaving sleep mode, whilst still processing events?  XXX Wouldn't release_fiber() be better?
    while(1) {
        uBit.sleep(10000);
    }

    // NOT REACHED
}
