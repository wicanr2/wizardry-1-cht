#include <cstdio>
#include <cstdlib>
#include <string>

#include "save/roster.h"
#include "wiz_test.h"

// Platform-portable temp path: prefer TMPDIR/TEMP/TMP env vars, fall back to
// the current working directory (CTest's BINARY_DIR is writable).
static std::string temp_save_path() {
    for (const char* var : {"TMPDIR", "TEMP", "TMP"}) {
        const char* v = std::getenv(var);
        if (v && *v) return std::string(v) + "/wiz_roster_test.json";
    }
#ifdef _WIN32
    return std::string("C:\\Windows\\Temp\\wiz_roster_test.json");
#else
    return std::string("/tmp/wiz_roster_test.json");
#endif
}

int main() {
    using namespace wiz;
    save::Roster r;
    r.used = 2;

    r.chars[0].name = "ALOZAR";
    r.chars[0].race = core::Race::Elf;
    r.chars[0].klass = core::Klass::Mage;
    r.chars[0].alignment = core::Alignment::Good;
    r.chars[0].attr = {10, 18, 12, 11, 14, 9};
    r.chars[0].char_level = 7;
    r.chars[0].hp_max = 32;
    r.chars[0].hp_left = 25;
    r.chars[0].gold = 1234;
    r.chars[0].experience = 50000;

    r.chars[1].name = "格爾達";   // unicode round-trip
    r.chars[1].race = core::Race::Dwarf;
    r.chars[1].klass = core::Klass::Fighter;
    r.chars[1].attr = {18, 9, 8, 17, 11, 10};
    r.chars[1].char_level = 9;
    r.chars[1].hp_max = 88;
    r.chars[1].hp_left = 88;

    const std::string path = temp_save_path();
    WIZ_CHECK(save::save(r, path));

    save::Roster r2;
    WIZ_CHECK(save::load(r2, path));
    WIZ_CHECK_EQ(r2.used, 2u);
    WIZ_CHECK_EQ(r2.chars[0].name, std::string("ALOZAR"));
    WIZ_CHECK_EQ(r2.chars[0].attr.iq, 18);
    WIZ_CHECK_EQ(r2.chars[0].hp_max, 32);
    WIZ_CHECK_EQ(r2.chars[0].gold, 1234);
    WIZ_CHECK_EQ(r2.chars[1].name, std::string("格爾達"));
    WIZ_CHECK_EQ(int(r2.chars[1].klass), int(core::Klass::Fighter));
    WIZ_CHECK_EQ(r2.chars[1].hp_max, 88);

    std::remove(path.c_str());
    return 0;
}
