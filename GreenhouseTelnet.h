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
    void setWaterHoursCommand(cmd *c);
    void setWaterMinSOCCommand(cmd *c);
    void closeCommand(cmd *c);
    void helpCommand(cmd *c);
    void errorCallback(cmd_error* e);

protected:
    status_t registerCommands();
    status_t setupTelnet();

    status_t registerSetWaterHoursCommand();
    status_t registerSetWaterMinSOCCommand();

    ESPTelnet _telnet;
    SimpleCLI _cli;

    SystemManager *_systemManager;

    bool endTelnet = false;

    /**
     * CLI Commands
     */
    Command _pingCommand;
    Command _setWaterHoursCommand;
    Command _setWaterMinSOCCommand;
    Command _helpCommand;
    Command _closeCommand;
};

extern GreenhouseTelnet gGreenhouseTelnet;

#endif /* end of include guard: GREENHOUSETELNET_H_WMCCPGRU */
