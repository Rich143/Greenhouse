#include "GreenhouseTelnet.h"
#include "Logger.h"

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
    _setWaterHoursCommand.addArgument("startHour");
    _setWaterHoursCommand.addArgument("endHour");

    if (!_setWaterHoursCommand) {
        return STATUS_FAIL;
    } 

    _setWaterHoursCommand.setDescription(" Sets the hours that the plants can be watered");

    return STATUS_OK;
}

status_t GreenhouseTelnet::registerSetWaterMinSOCCommand()
{
    _setWaterMinSOCCommand = _cli.addCommand("setWaterMinSOC",
                                            [](cmd *c) {
                                                gGreenhouseTelnet.setWaterMinSOCCommand(c);
                                            });
    _setWaterMinSOCCommand.addArgument("minSOC");

    if (!_setWaterMinSOCCommand) {
        return STATUS_FAIL;
    } 

    _setWaterMinSOCCommand.setDescription(" Sets the minimum SOC where the plants can be watered");
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
    _cli.parse(input);
}
