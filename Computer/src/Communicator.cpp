//
// Created by jk on 12.10.16.
//

#include <iostream>
#include "Communicator.h"

Communicator::Communicator() {
}

Communicator::Communicator(char fieldId, char robotId) {
    mFieldId = fieldId;
    mRobotId = robotId;
}

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
    bool captured = mInfraredBuf[1] == '1'; // has ball

    if (captured != mBallCaptured) {
        if (++mBallCapturedChangedFrames == 10) {
            mBallCaptured = captured;
            mBallCapturedChangedFrames = 0;
        }
    } else {
        mBallCapturedChangedFrames = 0;
    }

    if (!captured) {
        return false;
    }

    return mBallCaptured;
}

string Communicator::getRefereeCommand() {
    mBufWritten += sp_nonblocking_read(mPort, mBuf+(mBufWritten%12), 12);

    // aAXSTART----
    //cout << "mBuf " << mBuf << endl;

    for (int i = 0; i < 12; ++i) {
        if (mBuf[i] != 'a') {
            continue;
        }

        if (mBuf[(i+1)%12] != 'X' && mBuf[(i+1)%12] != mFieldId) {
            continue;
        }

        if (mBuf[(i+2)%12] != 'X' && mBuf[(i+2)%12] != mRobotId) {
            continue;
        }

        string command;

        for (int j = 3; j < 12; ++j) {
            command += mBuf[(i+j)%12];
        }

        if (command == lastCommand) {
            return "";
        }

        lastCommand = command;

        //cout << command << endl;

        if (command == "START----") {
            if (mBuf[(i+2)%12] == mRobotId) {
                send("aABACK------");
            }

            return "START";
        }

        if (command == "STOP-----") {
            if (mBuf[(i+2)%12] == mRobotId) {
                send("aABACK------");
            }

            return "STOP";
        }

        if (command == "PING-----") {
            send("aABACK------");

            /*
            for (int j = 0; j < 11; ++j) {
                mBuf[j] = '-';
            }
             */
        }

        break;
    }

    return "";
}
