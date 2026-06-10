#pragma once

#include <string>
#include <string_view>

namespace wiz::i18n {

enum class Lang { ZhTW = 0, En, JaJP, Count };

bool load(const std::string& path);

// Active UI language. tr() respects this; F4 cycles via cycle_lang().
Lang current_lang();
void set_lang(Lang l);
void cycle_lang();                       // ZhTW → En → JaJP → ZhTW
std::string_view lang_display_name(Lang l);

std::string_view tr(std::string_view key);    // current-language lookup
std::string_view tr_en(std::string_view key); // forced English

inline std::string_view operator""_tr(const char* k, std::size_t n) {
    return tr(std::string_view{k, n});
}

}  // namespace wiz::i18n
