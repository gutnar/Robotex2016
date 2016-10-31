//
// Created by jk on 12.10.16.
//

#ifndef ROBOTEX2016_COMMUNICATOR_H
#define ROBOTEX2016_COMMUNICATOR_H

#include <libserialport.h>
#include <string>

using namespace std;

class Communicator {
public:
    ~Communicator();
    void connect(int vendorId, int productId);
    void connect(string name);
    void sendCommand(string command);
    void send(string bytes);
    bool isBallCaptured();
    string getRefereeCommand();
private:
    char mBuf[12];
    int mBufWritten = 0;
    string lastCommand = "";
    char mInfraredBuf[3];
    int mBallCapturedFrames;
    struct sp_port ** mPorts;
    sp_port * mPort;
};


#endif //ROBOTEX2016_COMMUNICATOR_H
