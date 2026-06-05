#include <string>

#include "core/rng.h"
#include "game/camp.h"
#include "game/state.h"
#include "save/roster.h"
#include "wiz_test.h"

using namespace wiz;

static int seed_party(game::State& state) {
    auto& r = state.roster;
    auto add = [&](const char* name, int level, int hp) {
        auto& c = r.chars[r.used];
        c.name = name;
        c.char_level = static_cast<std::uint8_t>(level);
        c.hp_max = static_cast<std::int16_t>(hp);
        c.hp_left = static_cast<std::int16_t>(hp / 2);
        c.status = core::Status::Ok;
        c.armor_class = 8;
        ++r.used;
    };
    add("HEALER", 5, 30);
    add("DEAD-A", 1, 10);
    state.roster.chars[1].status = core::Status::Dead;
    state.party.count = 2;
    state.party.roster_index[0] = 0;
    state.party.roster_index[1] = 1;
    return 2;
}

void test_dios_heals() {
    core::global_rng().reseed(7);
    game::State state;
    seed_party(state);
    auto& c = state.roster.chars[0];
    auto hp_before = c.hp_left;
    auto msg = game::cast_camp_spell(state, "DIOS", 0);
    WIZ_CHECK(msg.find("DIOS") != std::string::npos);
    WIZ_CHECK(c.hp_left > hp_before);
    WIZ_CHECK(c.hp_left <= c.hp_max);
}

void test_madi_heals_all() {
    core::global_rng().reseed(100);
    game::State state;
    seed_party(state);
    auto msg = game::cast_camp_spell(state, "MADI", 0);
    WIZ_CHECK(msg.find("MADI") != std::string::npos);
    WIZ_CHECK_EQ(state.roster.chars[0].hp_left, state.roster.chars[0].hp_max);
}

void test_di_resurrect() {
    core::global_rng().reseed(1);  // Force success path
    game::State state;
    seed_party(state);
    // Target member 1 (DEAD-A)
    auto msg = game::cast_camp_spell(state, "DI", 1);
    auto& c = state.roster.chars[1];
    // Either resurrected (Ok, HP=1) or ashes
    WIZ_CHECK(c.status == core::Status::Ok || c.status == core::Status::Ashes);
    if (c.status == core::Status::Ok) {
        WIZ_CHECK_EQ(c.hp_left, 1);
        WIZ_CHECK(msg.find("復活") != std::string::npos);
    }
}

void test_dumapic_shows_position() {
    core::global_rng().reseed(42);
    game::State state;
    seed_party(state);
    state.camera.x = 7;
    state.camera.y = 3;
    state.camera.level = 1;
    state.camera.facing = render::Facing::East;
    auto msg = game::cast_camp_spell(state, "DUMAPIC", 0);
    WIZ_CHECK(msg.find("X:7") != std::string::npos);
    WIZ_CHECK(msg.find("Y:3") != std::string::npos);
    WIZ_CHECK(msg.find("東") != std::string::npos);
}

void test_unknown_spell_returns_message() {
    game::State state;
    seed_party(state);
    auto msg = game::cast_camp_spell(state, "FAKESPELL", 0);
    WIZ_CHECK(!msg.empty());
    WIZ_CHECK(msg.find("尚未實作") != std::string::npos);
}

int main() {
    test_dios_heals();
    test_madi_heals_all();
    test_di_resurrect();
    test_dumapic_shows_position();
    test_unknown_spell_returns_message();
    return 0;
}
