#include "../includes/database.hpp"
#include "../includes/config.hpp"
#include <LittleFS.h>

bool CfgDatabase::write(const String &filename, const Cfg &data)
{
    File file = LittleFS.open(filename.c_str(), FILE_MODE_WRITE);

    if (!file)
    {
        LOGLN("Failed to open " + filename);
        file.close();
        return false;
    }

    for (const auto &i : data)
    {
        String value = i.first + "=" + i.second + "\n";
        file.print(value);
    }

    file.close();
    return true;
}

Cfg CfgDatabase::read(const String &filename)
{
    Cfg data;
    File file = LittleFS.open(filename.c_str(), FILE_MODE_READ);

    if (!file)
    {
        LOGLN("Failed to open " + filename);
        file.close();
        return data;
    }

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        int index = line.indexOf('=');
        if (index != -1)
        {
            String key = line.substring(0, index);
            String value = line.substring(index + 1);
            data[key] = value;
        }
    }

    file.close();
    return data;
}