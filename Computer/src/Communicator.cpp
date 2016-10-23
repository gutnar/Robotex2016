//
// Created by jk on 12.10.16.
//

#include <iostream>
#include "Communicator.h"

void Communicator::connect(int vendorId, int productId) {
    // Get list of all connected USB devices
    sp_list_ports(&mPorts);

    // Find the total number of connected devices
    int numberOfPorts = sizeof(mPorts)/sizeof(mPorts[0]);

    // Try to find the device with given vendor ID and product ID
    for (int i = 0; i < numberOfPorts; ++i) {
        int vid, pid;
        sp_get_port_usb_vid_pid(mPorts[i], &vid, &pid);

        if (vid == vendorId && pid == productId) {
            // Device found, open it for communication
            mPort = mPorts[i];
            sp_open(mPort, SP_MODE_READ_WRITE);

            // Test

            return;
        }
    }

    throw 1;
}

Communicator::~Communicator() {
    sp_close(mPort);
    sp_free_port_list(mPorts);
}

void Communicator::sendCommand(string command) {
    sp_nonblocking_write(mPort, (command + "\n").c_str(), command.length() + 1);
}

bool Communicator::isBallCaptured() {
    sp_nonblocking_read(mPort, mBuf, 3);

    if (mBuf[1] == '1') {
        mBallCapturedFrames++;
    } else {
        mBallCapturedFrames = 0;
    }

    return mBallCapturedFrames == 10;
}
