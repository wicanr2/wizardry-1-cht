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
)
target_include_directories(wizrender PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_TTF_INCLUDE_DIR}
)
target_link_libraries(wizrender PUBLIC
  wizcore
  ${SDL2_LIBRARIES}
  ${SDL2_TTF_LIBRARY}
)
if(SDL2_IMAGE_LIBRARY)
  target_link_libraries(wizrender PUBLIC ${SDL2_IMAGE_LIBRARY})
endif()

add_library(wizgame STATIC
  game/state.cpp
  game/screens.cpp
  game/roller.cpp
  game/shop.cpp
  game/inn.cpp
  game/tavern.cpp
  game/camp.cpp
  save/gamesave.cpp
)
target_include_directories(wizgame PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(wizgame PUBLIC wizcore wizrender)
target_compile_definitions(wizgame PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)

add_executable(wizardry_cht main.cpp)
target_link_libraries(wizardry_cht PRIVATE wizgame wizcore wizrender)
target_compile_definitions(wizardry_cht PRIVATE
  WIZ_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
  WIZ_I18N_FILE="${CMAKE_SOURCE_DIR}/src/i18n/strings_zh_tw.json"
)
