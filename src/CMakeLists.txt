add_library(wizcore STATIC
  core/arith.cpp
  core/rng.cpp
  core/character.cpp
  core/item.cpp
  core/monster.cpp
  core/spell.cpp
  core/maze.cpp
  core/rules.cpp
  core/combat.cpp
  data/items_db.cpp
  save/roster.cpp
  i18n/tr.cpp
)
target_compile_definitions(wizcore PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)
target_include_directories(wizcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(wizcore PUBLIC cxx_std_17)
if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(wizcore PUBLIC nlohmann_json::nlohmann_json)
endif()

add_library(wizrender STATIC
  render/window.cpp
  render/text.cpp
  render/ui.cpp
  render/maze_view.cpp
  render/auto_map.cpp
  render/sprite.cpp
  render/audio.cpp
  render/theme.cpp
)
target_include_directories(wizrender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wizrender PUBLIC wizcore)

# Link SDL2 family — prefer modern Config-mode targets (vcpkg, recent distros)
# and fall back to library/path variables for older system installs.
if(TARGET SDL2::SDL2 OR TARGET SDL2::SDL2-static)
  if(TARGET SDL2::SDL2)
    target_link_libraries(wizrender PUBLIC SDL2::SDL2)
  else()
    target_link_libraries(wizrender PUBLIC SDL2::SDL2-static)
  endif()
  # vcpkg's SDL2main wraps main() into WinMain on Windows; required for tests
  if(TARGET SDL2::SDL2main)
    target_link_libraries(wizrender PUBLIC SDL2::SDL2main)
  endif()
else()
  target_include_directories(wizrender PUBLIC ${SDL2_INCLUDE_DIRS})
  target_link_libraries(wizrender PUBLIC ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
  target_link_libraries(wizrender PUBLIC SDL2_ttf::SDL2_ttf)
elseif(TARGET SDL2_ttf::SDL2_ttf-static)
  target_link_libraries(wizrender PUBLIC SDL2_ttf::SDL2_ttf-static)
elseif(SDL2_TTF_LIBRARY)
  target_include_directories(wizrender PUBLIC ${SDL2_TTF_INCLUDE_DIR})
  target_link_libraries(wizrender PUBLIC ${SDL2_TTF_LIBRARY})
endif()

if(TARGET SDL2_image::SDL2_image)
  target_link_libraries(wizrender PUBLIC SDL2_image::SDL2_image)
elseif(TARGET SDL2_image::SDL2_image-static)
  target_link_libraries(wizrender PUBLIC SDL2_image::SDL2_image-static)
elseif(SDL2_IMAGE_LIBRARY)
  target_include_directories(wizrender PUBLIC ${SDL2_IMAGE_INCLUDE_DIR})
  target_link_libraries(wizrender PUBLIC ${SDL2_IMAGE_LIBRARY})
endif()

if(TARGET SDL2_mixer::SDL2_mixer)
  target_link_libraries(wizrender PUBLIC SDL2_mixer::SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
  target_link_libraries(wizrender PUBLIC SDL2_mixer::SDL2_mixer-static)
elseif(SDL2_MIXER_LIBRARY)
  target_include_directories(wizrender PUBLIC ${SDL2_MIXER_INCLUDE_DIR})
  target_link_libraries(wizrender PUBLIC ${SDL2_MIXER_LIBRARY})
endif()
target_compile_definitions(wizrender PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

add_library(wizgame STATIC
  game/state.cpp
  game/screens.cpp
  game/roller.cpp
  game/shop.cpp
  game/inn.cpp
  game/tavern.cpp
  game/camp.cpp
  game/help.cpp
  game/intro.cpp
  game/traps.cpp
  game/class_change.cpp
  save/gamesave.cpp
)
target_include_directories(wizgame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wizgame PUBLIC wizcore wizrender)
target_compile_definitions(wizgame PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

add_executable(wizardry_cht main.cpp)
target_link_libraries(wizardry_cht PRIVATE wizgame wizcore wizrender)

# Dev tool — snapshot the combat scene for a specified monster id.
add_executable(snapshot_combat ${CMAKE_SOURCE_DIR}/tools/snapshot_combat.cpp)
target_link_libraries(snapshot_combat PRIVATE wizgame wizcore wizrender)
target_compile_definitions(snapshot_combat PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)

# Dev tool — snapshot the F1 help overlay in a specific scene.
add_executable(snapshot_help ${CMAKE_SOURCE_DIR}/tools/snapshot_help.cpp)
target_link_libraries(snapshot_help PRIVATE wizgame wizcore wizrender)
target_compile_definitions(snapshot_help PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)

# Dev tool — snapshot the EdgeOfTown scene with the demo Chinese-named party.
add_executable(snapshot_party ${CMAKE_SOURCE_DIR}/tools/snapshot_party.cpp)
target_link_libraries(snapshot_party PRIVATE wizgame wizcore wizrender)
target_compile_definitions(snapshot_party PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)

# Dev tool — snapshot one slide of the new-player intro guide.
add_executable(snapshot_intro ${CMAKE_SOURCE_DIR}/tools/snapshot_intro.cpp)
target_link_libraries(snapshot_intro PRIVATE wizgame wizcore wizrender)
target_compile_definitions(snapshot_intro PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)
target_compile_definitions(wizardry_cht PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)
