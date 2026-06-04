#include "game/state.h"

namespace wiz::game {

void State::push_message(std::string m) {
    message_log.push_back(std::move(m));
    if (message_log.size() > 8) {
        message_log.erase(message_log.begin(),
                          message_log.begin() + (message_log.size() - 8));
    }
    dirty = true;
}

void State::change_scene(Scene s) {
    prev_scene = scene;
    scene = s;
    dirty = true;
}

}  // namespace wiz::game
