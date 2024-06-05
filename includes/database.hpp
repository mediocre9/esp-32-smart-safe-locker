#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include <map>

using Cfg = std::map<String, String>;

template <typename T>
class Database
{
public:
    virtual bool write(const String &filename, const T &data) = 0;
    virtual T read(const String &filename) = 0;
};

class CfgDatabase : public Database<Cfg>
{
public:
    bool write(const String &filename, const Cfg &data) override;
    Cfg read(const String &filename) override;
};

#endif