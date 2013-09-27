#pragma once
#include <string>
#include <map>
#include <vector>

namespace event {

using EventHandler = std::function<void(void*, void*)>;

struct EventMap {

    bool Trigger(const char *event, void *sender, void *args) {
        for (auto& func : map[std::string(event)]) {
            func(sender, args);
        }
        return map[std::string(event)].size() > 0;
    }

    void Register(const char * event, EventHandler handler) {
        map[std::string(event)].push_back(handler);
    }

protected:
    std::map<std::string, std::vector<std::function<void(void*, void*)>>> map;
};

bool Trigger(const char *event, void *sender, void *args);

void Register(const char *event, EventHandler handler);

struct EventRegisterer {
    EventRegisterer(const char *event, EventHandler handler);
};

} // namespace event

#define REGISTER_EVENT(name, func) \
    namespace event { \
        EventRegisterer event_registerer_##name(#name, func); \
    }
