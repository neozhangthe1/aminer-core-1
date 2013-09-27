#include <map>
#include <mutex>
#include <utility>

#include "event.hpp"

namespace event {

EventMap *default_event_map = nullptr;
std::mutex default_event_map_lock;

bool Trigger(const char *event, void *sender, void *args) {
    if (!default_event_map) return false;
    return default_event_map->Trigger(event, sender, args);
}

void Register(const char *event, EventHandler handler) {
    default_event_map_lock.lock();
    if (default_event_map == nullptr) {
        default_event_map = new EventMap;
    }
    default_event_map->Register(event, handler);
    default_event_map_lock.unlock();
}


EventRegisterer::EventRegisterer(const char * event, EventHandler handler) {
    Register(event, handler);
}

} // namespace event
