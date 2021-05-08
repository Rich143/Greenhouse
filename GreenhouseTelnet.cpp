#include "GreenhouseTelnet.h"
#include "Logger.h"

#define TELNET_INACTIVITY_TIME_MS (30*1000)

GreenhouseTelnet gGreenhouseTelnet;

/* ------------------------------------------------- */

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
    LOG_INFO("- Telnet: " + String (ip) + " connected");
}

void onTelnetDisconnect(String ip) {
    LOG_INFO("- Telnet: " + String (ip) + " disconnected");
}

void onTelnetReconnect(String ip) {
    LOG_INFO("- Telnet: " + String (ip) + " reconnected");
}

void onTelnetConnectionAttempt(String ip) {
    LOG_INFO("- Telnet: " + String (ip) + " tried to connect");
}

/* ------------------------------------------------- */

void GreenhouseTelnet::helpCommand(cmd *c)
{
    _telnet.println("Help:");
    _telnet.println(_cli.toString());
}

void GreenhouseTelnet::pingCommand(cmd *c)
{
    _telnet.println("> pong");
}

void GreenhouseTelnet::setWaterHoursCommand(cmd *c)
{
    Command cmd(c);

    int startHour = cmd.getArgument("startHour").getValue().toInt();
    int endHour = cmd.getArgument("endHour").getValue().toInt();

    status_t rc = _systemManager->setWaterHours(startHour, endHour);

    if (rc == STATUS_OK) {
        _telnet.println("> Success: updated water hours");
    } else {
        _telnet.println("> Fail: failed to update water hours");
    }
}

void GreenhouseTelnet::setWaterMinSOCCommand(cmd *c)
{
    Command cmd(c);

    double minSOC = cmd.getArgument("minSOC").getValue().toDouble();

    status_t rc = _systemManager->setWaterMinSOC(minSOC);

    if (rc == STATUS_OK) {
        _telnet.println("> Success: updated min water SOC");
    } else {
        _telnet.println("> Fail: failed to update min water SOC");
    }
}

void GreenhouseTelnet::updateWaterLevelCalibrationCommand(cmd *c)
{
    Command cmd(c);

    double emptyCm = cmd.getArgument("emptyCm").getValue().toDouble();
    double fullCm = cmd.getArgument("fullCm").getValue().toDouble();

    status_t rc = _systemManager->updateWaterLevelCalibration(fullCm, emptyCm);

    if (rc == STATUS_OK) {
        _telnet.println("> Success: updated water level cal");
    } else {
        _telnet.println("> Fail: failed to update water level cal");
    }
}

void GreenhouseTelnet::getWaterDistanceCommand(cmd *c)
{
    uint32_t distanceCm = _systemManager->getWaterDistanceCm();

    _telnet.print("> Water Distance ");
    _telnet.print(String(distanceCm));
    _telnet.println(" cm");
}

void GreenhouseTelnet::getWaterLevelPercentCommand(cmd *c)
{
    double waterLevel = _systemManager->getWaterLevelPercent();

    _telnet.print("> Water Level ");
    _telnet.print(String(waterLevel));
    _telnet.println(" %");
}

void GreenhouseTelnet::closeCommand(cmd *c)
{
    _telnet.println("> Goodbye");
    endTelnet = true;
}

// Callback in case of an error
void GreenhouseTelnet::errorCallback(cmd_error* e) {
    CommandError cmdError(e); // Create wrapper object

    _telnet.print("ERROR: ");
    _telnet.println(cmdError.toString());

    if (cmdError.hasCommand()) {
        _telnet.print("Did you mean \"");
        _telnet.print(cmdError.getCommand().toString());
        _telnet.println("\"?");
    }
}

status_t GreenhouseTelnet::registerSetWaterHoursCommand()
{
    _setWaterHoursCommand = _cli.addCommand("setWaterHours",
                                            [](cmd *c) {
                                                gGreenhouseTelnet.setWaterHoursCommand(c);
                                            });

    if (!_setWaterHoursCommand) {
        return STATUS_FAIL;
    } 

    _setWaterHoursCommand.addArgument("startHour");
    _setWaterHoursCommand.addArgument("endHour");

    _setWaterHoursCommand.setDescription(" Sets the hours that the plants can be watered");

    return STATUS_OK;
}

status_t GreenhouseTelnet::registerSetWaterMinSOCCommand()
{
    _setWaterMinSOCCommand = _cli.addCommand("setWaterMinSOC",
                                            [](cmd *c) {
                                                gGreenhouseTelnet.setWaterMinSOCCommand(c);
                                            });

    if (!_setWaterMinSOCCommand) {
        return STATUS_FAIL;
    } 

    _setWaterMinSOCCommand.addArgument("minSOC");

    _setWaterMinSOCCommand.setDescription(" Sets the minimum SOC where the plants can be watered");
    return STATUS_OK;
}

status_t GreenhouseTelnet::registerWaterLevelCalibrationCommand()
{
    _updateWaterLevelCalibrationCommand = _cli.addCommand("waterLevelCal",
                                            [](cmd *c) {
                                                gGreenhouseTelnet.updateWaterLevelCalibrationCommand(c);
                                            });

    if (!_updateWaterLevelCalibrationCommand) {
        return STATUS_FAIL;
    } 

    _updateWaterLevelCalibrationCommand.addArgument("emptyCm");
    _updateWaterLevelCalibrationCommand.addArgument("fullCm");

    _updateWaterLevelCalibrationCommand.setDescription(
        " Sets the calibration for water level."
        " The empty and full distances are the distances measured when the"
        " water reservoir is empty and full respectively");
    return STATUS_OK;
}

status_t GreenhouseTelnet::registerCommands()
{
    _pingCommand = _cli.addCmd("ping", 
                               [](cmd *c) {
                                gGreenhouseTelnet.pingCommand(c);
                               });
    if (!_pingCommand) {
        LOG_ERROR("Failed to create ping command");
        return STATUS_FAIL;
    }
    _pingCommand.setDescription(" Ping the CLI, return Pong");

    _closeCommand = _cli.addCmd("close", 
                               [](cmd *c) {
                                gGreenhouseTelnet.closeCommand(c);
                               });
    if (!_closeCommand) {
        LOG_ERROR("Failed to create close command");
        return STATUS_FAIL;
    }
    _closeCommand.setDescription(" Closes the telnet connection and restarts the greenhouse app");

    status_t rc;

    rc = registerSetWaterHoursCommand();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to register setWaterHours command");
        return STATUS_FAIL;
    }

    rc = registerSetWaterMinSOCCommand();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to register setWaterMinSOC command");
        return STATUS_FAIL;
    }

    rc = registerWaterLevelCalibrationCommand();
    if (rc != STATUS_OK) {
        LOG_ERROR("Failed to register waterLevelCal command");
        return STATUS_FAIL;
    }

    _getWaterDistanceCommand = _cli.addCmd("waterDistance", 
                               [](cmd *c) {
                                gGreenhouseTelnet.getWaterDistanceCommand(c);
                               });
    if (!_getWaterDistanceCommand) {
        LOG_ERROR("Failed to create waterDistance command");
        return STATUS_FAIL;
    }
    _getWaterDistanceCommand.setDescription(" Get the distance to the water as measured by the water level sensor");

    _getWaterDistancePercentCommand = _cli.addCmd("waterPercent", 
                               [](cmd *c) {
                                gGreenhouseTelnet.getWaterLevelPercentCommand(c);
                               });
    if (!_getWaterDistancePercentCommand) {
        LOG_ERROR("Failed to create waterDistance command");
        return STATUS_FAIL;
    }
    _getWaterDistancePercentCommand.setDescription(" Get the water level");
 
 
    // Set error Callback
    _cli.setOnError([](cmd_error *e) {
                        gGreenhouseTelnet.errorCallback(e);
                    });

    _helpCommand = _cli.addCmd("help",
                [](cmd *c) {
                    gGreenhouseTelnet.helpCommand(c);
                });
    if (!_helpCommand) {
        LOG_ERROR("Failed to create help command");
        return STATUS_FAIL;
    }
    _helpCommand.setDescription(" Get CLI help");

    return STATUS_OK;
}

status_t GreenhouseTelnet::setupTelnet()
{
    // passing on functions for various telnet events
    _telnet.onConnect(onTelnetConnect);
    _telnet.onConnectionAttempt(onTelnetConnectionAttempt);
    _telnet.onReconnect(onTelnetReconnect);
    _telnet.onDisconnect(onTelnetDisconnect);

    // passing a lambda function
    _telnet.onInputReceived(
        [](String input) {
            gGreenhouseTelnet.handleInput(input);
        });

    if (_telnet.begin()) {
        LOG_INFO("- Telnet: running");
        return STATUS_OK;
    } else {
        LOG_INFO("- Telnet failed to start");
        return STATUS_FAIL;
    }
}

status_t GreenhouseTelnet::start(SystemManager* systemManager)
{
    status_t rc;

    _systemManager = systemManager;

    rc = registerCommands();
    if (rc != STATUS_OK) {
        return rc;
    }

    rc = setupTelnet();
    if (rc != STATUS_OK) {
        return rc;
    }


    // Mark the telnet as active when we start
    lastActiveMS = millis();

    return STATUS_OK;
}

status_t GreenhouseTelnet::run()
{
    _telnet.loop();

    if (endTelnet) {
        _systemManager->goToSleep();
    }
}

void GreenhouseTelnet::handleInput(String input)
{
    lastActiveMS = millis();

    _cli.parse(input);
}

bool GreenhouseTelnet::isActive()
{
    return (millis() - lastActiveMS) < TELNET_INACTIVITY_TIME_MS;
}
