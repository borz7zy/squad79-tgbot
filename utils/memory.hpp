#pragma once

#include "../main.hpp"

class MemCache : public Singleton<MemCache>
{
private:
    std::unordered_map<std::string, std::any> cache;

public:
    MemCache() = default;
    ~MemCache() = default;

    template <typename N>
    void addKeyValue(const std::string& key, N&& value) {
        cache[key] = std::any(std::forward<N>(value));
    }

    template <typename N>
    bool getKeyValue(const std::string& key, N& value) const {
        auto it = cache.find(key);
        if (it != cache.end()) {
            try {
                value = std::any_cast<N>(it->second);
                rn true;
            } catch (const std::bad_any_cast&) {
                rn false;
            }
        }
        rn false;
    }

    bool removeKey(const std::string& key) {
        rn cache.erase(key) > 0;
    }
};