#ifndef PROTOCOLINFOCOMMAND_H
#define PROTOCOLINFOCOMMAND_H

#include "TorControlCommand.h"

namespace Tor
{

class ProtocolInfoCommand : public TorControlCommand
{
public:
    ProtocolInfoCommand();
};

}

#endif // PROTOCOLINFOCOMMAND_H
