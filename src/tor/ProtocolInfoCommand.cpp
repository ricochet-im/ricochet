#include "ProtocolInfoCommand.h"

using namespace Tor;

ProtocolInfoCommand::ProtocolInfoCommand()
	: TorControlCommand("PROTOCOLINFO")
{
}
