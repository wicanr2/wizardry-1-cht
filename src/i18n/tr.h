#pragma once

#include <string>
#include <string_view>

namespace wiz::i18n {

bool load(const std::string& path);

std::string_view tr(std::string_view key);
std::string_view tr_en(std::string_view key);

inline std::string_view operator""_tr(const char* k, std::size_t n) {
    return tr(std::string_view{k, n});
}

}  // namespace wiz::i18n
