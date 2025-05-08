#ifndef DATABASE_H
#define DATABASE_H

#include <WString.h>
#include <map>

#include "Config.hpp"

using Cfg = std::map<String, String>;

template <typename T>
class Database {
public:
    [[nodiscard]] virtual bool write(const String& filename, const T& data) = 0;
    [[nodiscard]] virtual T read(const String& filename) = 0;
    virtual ~Database() {}
};

class CfgDatabase final : public Database<Cfg> {
public:
    [[nodiscard]] bool write(const String& filename, const Cfg& data) override;
    [[nodiscard]] Cfg read(const String& filename) override;
};

#endif