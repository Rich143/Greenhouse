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

    bool isActive();

    /**
     * Private, only to be used in callbacks
     */
    void handleInput(String input);
    void pingCommand(cmd * c);
    void setWaterHoursCommand(cmd *c);
    void setWaterMinSOCCommand(cmd *c);
    void updateWaterLevelCalibrationCommand(cmd *c);
    void getWaterDistanceCommand(cmd *c);
    void getWaterLevelPercentCommand(cmd *c);
    void closeCommand(cmd *c);
    void helpCommand(cmd *c);
    void errorCallback(cmd_error* e);

protected:
    status_t registerCommands();
    status_t setupTelnet();

    status_t registerSetWaterHoursCommand();
    status_t registerSetWaterMinSOCCommand();
    status_t registerWaterLevelCalibrationCommand();

    ESPTelnet _telnet;
    SimpleCLI _cli;

    SystemManager *_systemManager;

    bool endTelnet = false;

    uint64_t lastActiveMS = 0;

    /**
     * CLI Commands
     */
    Command _pingCommand;

    Command _setWaterHoursCommand;
    Command _setWaterMinSOCCommand;

    Command _updateWaterLevelCalibrationCommand;
    Command _getWaterDistanceCommand;
    Command _getWaterDistancePercentCommand;

    Command _helpCommand;
    Command _closeCommand;
};

extern GreenhouseTelnet gGreenhouseTelnet;

#endif /* end of include guard: GREENHOUSETELNET_H_WMCCPGRU */
