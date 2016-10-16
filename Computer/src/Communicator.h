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
    Communicator(int vendorId, int productId);
    ~Communicator();
    void sendCommand(string command);
private:
    struct sp_port ** mPorts;
    sp_port * mPort;
};


#endif //ROBOTEX2016_COMMUNICATOR_H
