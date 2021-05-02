#ifndef CONFIGVALUE_H_Y3AWVYTU
#define CONFIGVALUE_H_Y3AWVYTU

#include "Status.h"

// Preferences keys can be a max of 15 chars
#define CONFIG_VALUE_MAX_CONFIG_NAME_LEN 15

/*! \class ConfigValue
 *  \brief Class to manage a config value backed by ESP preferences
 *
 *  Detailed description
 */
class ConfigValue
{
public:
    ConfigValue() :
        _value(0)
    {
        _configName[0] = '\0';
    }

    /**
     * @brief Initialize the config value by reading it from preferences
     *
     * @param configValueName The name of the config value, which is used for
     * the preferences key name
     *
     * @param defaultValue The default value to use if the value doesn't exist
     * in the preferences store
     *
     * @return status
     */
    status_t initAndLoad(const char *configValueName, double defaultValue=0);

    double getValue();

    status_t updateValue(double newValue);
protected:
    double _value;
    char _configName[CONFIG_VALUE_MAX_CONFIG_NAME_LEN];
};

#endif /* end of include guard: CONFIGVALUE_H_Y3AWVYTU */
