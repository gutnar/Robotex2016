//
// Created by jk on 12.10.16.
//

#include <iostream>
#include "Communicator.h"

void Communicator::connect(int vendorId, int productId) {
    // Get list of all connected USB devices
    sp_list_ports(&mPorts);

    // Find the total number of connected devices
    int numberOfPorts = sizeof(mPorts) / sizeof(mPorts[0]);

    // Try to find the device with given vendor ID and product ID
    for (int i = 0; i < numberOfPorts; ++i) {
        int vid, pid;
        sp_get_port_usb_vid_pid(mPorts[i], &vid, &pid);

        if (vid == vendorId && pid == productId) {
            // Device found, open it for communication
            mPort = mPorts[i];
            sp_open(mPort, SP_MODE_READ_WRITE);
            sp_free_port_list(mPorts);
            return;
        }
    }

    throw 1;
}

void Communicator::connect(string name) {
    if (sp_get_port_by_name(name.c_str(), &mPort) < 0) {
        throw 1;
    } else {
        sp_open(mPort, SP_MODE_READ_WRITE);
    }
}

Communicator::~Communicator() {
    sp_close(mPort);
}

void Communicator::send(string bytes) {
    sp_nonblocking_write(mPort, bytes.c_str(), bytes.length());
}

void Communicator::sendCommand(string command) {
    send(command+"\n");
    //sp_nonblocking_write(mPort, (command + "\n").c_str(), command.length() + 1);
}

bool Communicator::isBallCaptured() {
    sp_nonblocking_read(mPort, mInfraredBuf, 3);

    if (mInfraredBuf[1] == '1') {
        mBallCapturedFrames++;
    } else {
        mBallCapturedFrames = 0;
    }

    return mBallCapturedFrames == 10;
}

string Communicator::getRefereeCommand() {
    mBufWritten += sp_nonblocking_read(mPort, mBuf+(mBufWritten%11), 11);

    for (int i = 0; i < 11; ++i) {
        if (mBuf[i] == 'a' && mBuf[(i+1)%11] == 'A' && mBuf[(i+2)%11] == 'K') {
            string command;

            for (int j = 3; j < 11; ++j) {
                command += mBuf[(i+j)%11];
            }

            if (command == lastCommand) {
                return "";
            }

            lastCommand = command;

            if (command == "START---") {
                send("aAKACK-----");
                return "START";
            }

            if (command == "STOP----") {
                send("aAKACK-----");
                return "STOP";
            }

            if (command == "PING----") {
                send("aAKACK-----");
            }

            break;
        }
    }

    return "";
}
