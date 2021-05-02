#include "ConfigValue.h"
#include "Logger.h"
#include "AppPreferences.h"

status_t ConfigValue::initAndLoad(const char *configValueName, double defaultValue) {
    if (configValueName == nullptr) {
        LOG_ERROR("Config value init got null value name");
        return STATUS_INVALID_PARAMS;
    }

    size_t name_len = strlen(configValueName);

    if (name_len == 0 || name_len > CONFIG_VALUE_MAX_CONFIG_NAME_LEN) {
        LOG_ERROR("Config value got name of invalid length " + String(name_len));
        return STATUS_INVALID_PARAMS;
    }

    strcpy(_configName, configValueName);

    _value = gAppPreferences.getDouble(_configName, defaultValue);

    return STATUS_OK;
}

double ConfigValue::getValue()
{
    return _value;
}

status_t ConfigValue::updateValue(double newValue)
{
    if (gAppPreferences.putDouble(_configName, newValue) == 0) {
        LOG_ERROR("Failed to update config value " + String(_configName));
        return STATUS_FAIL;
    }

    _value = newValue;

    return STATUS_OK;
}
