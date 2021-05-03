#ifndef GREENHOUSETELNET_H_WMCCPGRU
#define GREENHOUSETELNET_H_WMCCPGRU

#include "ESPTelnet.h"
#include "SimpleCLI.h"
#include "Status.h"
#include "SystemManager.h"

/*! \class GreenhouseTelnet
 *  \brief Class to manage the greenhouse telnet server
 *
 */
class GreenhouseTelnet
{
public:
    status_t start(SystemManager* systemManager);

    status_t run();

    /**
     * Private, only to be used in callbacks
     */
    void handleInput(String input);
    void pingCommand(cmd * c);
    void closeCommand(cmd *c);
    void errorCallback(cmd_error* e);

protected:
    status_t registerCommands();
    status_t setupTelnet();

    ESPTelnet _telnet;
    SimpleCLI _cli;

    SystemManager *_systemManager;

    bool endTelnet = false;

    /**
     * CLI Commands
     */
    Command _pingCommand;
    Command _closeCommand;
};

extern GreenhouseTelnet gGreenhouseTelnet;

#endif /* end of include guard: GREENHOUSETELNET_H_WMCCPGRU */
