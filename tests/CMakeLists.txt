# Unit tests use a tiny header-only assertion macro to avoid an external
# test-framework dependency for the prototype. Each test_* executable is
# registered with CTest.

function(wiz_add_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE wizcore)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

# Tests that need the full game module (state, camp, etc.)
# wizgame transitively links wizrender → SDL2main. On Windows SDL2main wraps
# main() into WinMain by #defining main. We don't open a window in tests so
# we tell SDL to skip that wrapping via SDL_MAIN_HANDLED.
function(wiz_add_game_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE wizgame wizcore)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_compile_definitions(${name} PRIVATE SDL_MAIN_HANDLED)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

wiz_add_test(test_arith test_arith.cpp)
wiz_add_test(test_rng test_rng.cpp)
wiz_add_test(test_character test_character.cpp)
wiz_add_test(test_rules test_rules.cpp)
wiz_add_test(test_save test_save.cpp)
wiz_add_test(test_combat test_combat.cpp)
wiz_add_test(test_v13_mechanics test_v13_mechanics.cpp)
wiz_add_test(test_b1f_walk test_b1f_walk.cpp)
wiz_add_test(test_spell_engine test_spell_engine.cpp)
wiz_add_test(test_monster_db test_monster_db.cpp)
wiz_add_game_test(test_camp_spells test_camp_spells.cpp)
wiz_add_game_test(test_class_change test_class_change.cpp)
