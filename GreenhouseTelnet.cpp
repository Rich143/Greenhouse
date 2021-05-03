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

void GreenhouseTelnet::pingCommand(cmd *c)
{
    _telnet.println("> pong");
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

    _closeCommand = _cli.addCmd("close", 
                               [](cmd *c) {
                                gGreenhouseTelnet.closeCommand(c);
                               });
    if (!_pingCommand) {
        LOG_ERROR("Failed to create ping command");
        return STATUS_FAIL;
    }
 
    // Set error Callback
    _cli.setOnError([](cmd_error *e) {
                        gGreenhouseTelnet.errorCallback(e);
                    });

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
