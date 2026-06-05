# Unit tests use a tiny header-only assertion macro to avoid an external
# test-framework dependency for the prototype. Each test_* executable is
# registered with CTest.

function(wiz_add_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE wizcore)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

wiz_add_test(test_arith test_arith.cpp)
wiz_add_test(test_rng test_rng.cpp)
wiz_add_test(test_character test_character.cpp)
wiz_add_test(test_rules test_rules.cpp)
wiz_add_test(test_save test_save.cpp)
wiz_add_test(test_combat test_combat.cpp)
wiz_add_test(test_spell_engine test_spell_engine.cpp)
wiz_add_test(test_monster_db test_monster_db.cpp)
