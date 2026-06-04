#include "core/combat.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "core/rng.h"

namespace wiz::core {

namespace {

int member_attack_dmg(const Character& c) {
    auto& rng = global_rng();
    int base = rng.dice(1, 8);
    int str_bonus = (c.attr.strength >= 16) ? (c.attr.strength - 15) : 0;
    int dmg = base + str_bonus;
    return std::max(1, dmg);
}

bool member_hits(const Character& attacker, int target_ac) {
    // Simplified to-hit: roll d20 + char_level + str_bonus vs (20 + ac)
    auto& rng = global_rng();
    int roll = rng.range(1, 20);
    int bonus = attacker.char_level / 3;
    if (attacker.attr.strength >= 16) bonus += 1;
    return (roll + bonus + 10) >= (20 - target_ac);
}

// Forward decl
void cast_spell(CombatState& s, int caster_idx, Character& caster, const PlayerAction& a);

int monster_attack_dmg(const Monster& m) {
    auto& rng = global_rng();
    // Use hp_dice as rough proxy for damage dice if not otherwise set.
    int n = std::max<int>(1, m.hp_dice_n);
    int d = m.hp_dice_d > 0 ? m.hp_dice_d : 6;
    int dmg = rng.dice(n, d / 2 + 1);
    return std::max(1, dmg);
}

bool monster_hits(const Monster& m, int target_ac) {
    auto& rng = global_rng();
    int roll = rng.range(1, 20);
    return (roll + (10 - m.armor_class)) >= (20 - target_ac);
}

}  // namespace

const char* phase_label(CombatPhase p) noexcept {
    switch (p) {
        case CombatPhase::Engage: return "engage";
        case CombatPhase::PartyAction: return "party_action";
        case CombatPhase::PickSpell: return "pick_spell";
        case CombatPhase::PickTarget: return "pick_target";
        case CombatPhase::Resolve: return "resolve";
        case CombatPhase::End: return "end";
    }
    return "?";
}

// ---- Spell engine -------------------------------------------------------

namespace {

void apply_dmg_to_group(CombatGroup& g, int dmg) {
    g.hp_total -= dmg;
    int avg_hp = std::max(1, (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
    int alive_after = std::max(0, g.hp_total / avg_hp + 1);
    int dead = g.alive_count - alive_after;
    if (dead > 0) g.alive_count = static_cast<std::int16_t>(alive_after);
}

CombatGroup* group_at(CombatState& s, int idx) {
    if (idx < 0 || static_cast<std::size_t>(idx) >= s.groups.size()) return nullptr;
    return &s.groups[idx];
}

void log_cast(CombatState& s, const Character& c, const char* name, const char* effect) {
    char buf[200];
    std::snprintf(buf, sizeof(buf), "%s 詠唱 %s — %s", c.name.c_str(), name, effect);
    s.log.emplace_back(buf);
}

void dmg_spell(CombatState& s, const Character& c, const PlayerAction& a,
               const char* name, int n_dice, int d_sides, const char* desc) {
    auto& rng = global_rng();
    if (auto* g = group_at(s, a.target_group)) {
        if (g->alive_count <= 0) return;
        int dmg = rng.dice(n_dice, d_sides);
        apply_dmg_to_group(*g, dmg);
        char buf[200];
        std::snprintf(buf, sizeof(buf), "%s — %s 受 %d %s。",
                      name, g->prototype.name_unknown.c_str(), dmg, desc);
        log_cast(s, c, name, buf + std::strlen(name) + 3);
    }
}

void all_dmg_spell(CombatState& s, const Character& c,
                   const char* name, int n_dice, int d_sides, const char* desc) {
    auto& rng = global_rng();
    log_cast(s, c, name, "全敵受襲！");
    for (auto& g : s.groups) {
        if (g.alive_count <= 0) continue;
        int dmg = rng.dice(n_dice, d_sides);
        apply_dmg_to_group(g, dmg);
        char buf[200];
        std::snprintf(buf, sizeof(buf), "  %s 受 %d %s。",
                      g.prototype.name_unknown.c_str(), dmg, desc);
        s.log.emplace_back(buf);
    }
}

void heal_self(CombatState& s, Character& c, const char* name, int n_dice, int d_sides) {
    auto& rng = global_rng();
    int amount = rng.dice(n_dice, d_sides);
    int actual = std::min<int>(c.hp_max - c.hp_left, amount);
    c.hp_left = static_cast<std::int16_t>(c.hp_left + actual);
    char buf[120];
    std::snprintf(buf, sizeof(buf), "自身 +%d HP", actual);
    log_cast(s, c, name, buf);
}

[[maybe_unused]] void heal_party(CombatState& s, std::array<Character, 6>& party,
                                 const char* name, int n_dice, int d_sides) {
    auto& rng = global_rng();
    log_cast(s, const_cast<Character&>(party[0]), name, "全隊回復！");
    for (auto& m : party) {
        if (m.name.empty() || m.status != Status::Ok) continue;
        int amount = rng.dice(n_dice, d_sides);
        int actual = std::min<int>(m.hp_max - m.hp_left, amount);
        m.hp_left = static_cast<std::int16_t>(m.hp_left + actual);
        char buf[200];
        std::snprintf(buf, sizeof(buf), "  %s +%d HP", m.name.c_str(), actual);
        s.log.emplace_back(buf);
    }
}

[[maybe_unused]] void full_heal_party(CombatState& s, std::array<Character, 6>& party, const char* name) {
    if (party.empty()) return;
    log_cast(s, party[0], name, "全隊完全治癒。");
    for (auto& m : party) {
        if (m.name.empty()) continue;
        m.hp_left = m.hp_max;
    }
}

void self_ac_buff(CombatState& s, Character& c, int caster_idx,
                  const char* name, int delta) {
    if (caster_idx >= 0 && caster_idx < 6) s.per_member_ac[caster_idx] += delta;
    char buf[80];
    std::snprintf(buf, sizeof(buf), "自身 AC %+d", delta);
    log_cast(s, c, name, buf);
}

void party_ac_buff(CombatState& s, const Character& c, const char* name, int delta) {
    s.party_buff.ac_mod += delta;
    char buf[80];
    std::snprintf(buf, sizeof(buf), "全隊 AC %+d", delta);
    log_cast(s, c, name, buf);
}

void group_ac_debuff(CombatState& s, const Character& c, const PlayerAction& a,
                     const char* name, int delta) {
    if (auto* g = group_at(s, a.target_group)) {
        g->ac_mod += delta;
        char buf[120];
        std::snprintf(buf, sizeof(buf), "%s AC %+d", g->prototype.name_unknown.c_str(), delta);
        log_cast(s, c, name, buf);
    }
}

void all_groups_ac_debuff(CombatState& s, const Character& c,
                          const char* name, int delta) {
    log_cast(s, c, name, "全敵 AC 降低！");
    for (auto& g : s.groups) {
        if (g.alive_count <= 0) continue;
        g.ac_mod += delta;
    }
}

void group_status(CombatState& s, const Character& c, const PlayerAction& a,
                  const char* name, const char* status_zh, bool CombatGroup::*flag) {
    auto& rng = global_rng();
    if (auto* g = group_at(s, a.target_group)) {
        if (rng.range(1, 100) <= 75) {
            g->*flag = true;
            char buf[120];
            std::snprintf(buf, sizeof(buf), "%s 中 %s！",
                          g->prototype.name_unknown.c_str(), status_zh);
            log_cast(s, c, name, buf);
        } else {
            log_cast(s, c, name, "效果被抵抗。");
        }
    }
}

void instant_kill_group(CombatState& s, const Character& c, const PlayerAction& a,
                        const char* name, int max_level) {
    if (auto* g = group_at(s, a.target_group)) {
        if (g->prototype.monster_class <= max_level) {
            char buf[120];
            std::snprintf(buf, sizeof(buf), "%s 全滅 (%d 隻)！",
                          g->prototype.name_unknown.c_str(), int(g->alive_count));
            log_cast(s, c, name, buf);
            g->alive_count = 0;
            g->hp_total = 0;
        } else {
            log_cast(s, c, name, "敵人等級太高，無效。");
        }
    }
}

void single_target_kill(CombatState& s, const Character& c, const PlayerAction& a,
                        const char* name) {
    auto& rng = global_rng();
    if (auto* g = group_at(s, a.target_group)) {
        if (g->alive_count > 0 && rng.range(1, 100) <= 50) {
            g->alive_count--;
            int avg_hp = std::max(1, (g->prototype.hp_dice_n * (g->prototype.hp_dice_d + 1)) / 2);
            g->hp_total -= avg_hp;
            char buf[120];
            std::snprintf(buf, sizeof(buf), "%s 倒下 1 隻。",
                          g->prototype.name_unknown.c_str());
            log_cast(s, c, name, buf);
        } else {
            log_cast(s, c, name, "效果被抵抗。");
        }
    }
}

[[maybe_unused]] void haman_random(CombatState& s, Character& c, std::array<Character, 6>& party,
                                   const char* name, int age_weeks) {
    auto& rng = global_rng();
    int choice = rng.range(1, 7);
    c.age = static_cast<std::uint16_t>(c.age + age_weeks);
    char buf[200];
    switch (choice) {
        case 1:
            std::snprintf(buf, sizeof(buf), "全隊回滿 HP！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            for (auto& m : party) if (!m.name.empty()) m.hp_left = m.hp_max;
            break;
        case 2:
            std::snprintf(buf, sizeof(buf), "全敵受 4d8 傷害！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            for (auto& g : s.groups) {
                if (g.alive_count > 0) apply_dmg_to_group(g, rng.dice(4, 8));
            }
            break;
        case 3:
            std::snprintf(buf, sizeof(buf), "全隊 AC -4！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            s.party_buff.ac_mod -= 4;
            break;
        case 4:
            std::snprintf(buf, sizeof(buf), "全敵 AC +4！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            for (auto& g : s.groups) if (g.alive_count > 0) g.ac_mod += 4;
            break;
        case 5:
            std::snprintf(buf, sizeof(buf), "全隊魔法抗性提升！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            s.party_buff.magic_resist = true;
            break;
        case 6:
            std::snprintf(buf, sizeof(buf), "敵人沉睡！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            for (auto& g : s.groups) if (g.alive_count > 0) g.asleep = true;
            break;
        case 7: default:
            std::snprintf(buf, sizeof(buf), "TILTOWAIT 等效爆炸！術者老 %d 週。", age_weeks);
            log_cast(s, c, name, buf);
            for (auto& g : s.groups) {
                if (g.alive_count > 0) apply_dmg_to_group(g, rng.dice(10, 15));
            }
            break;
    }
}

}  // namespace

void cast_spell(CombatState& s, int caster_idx, Character& caster,
                const PlayerAction& a) {
    const std::string& n = a.spell_name;

    // === Mage damage (single group) ===
    if (n == "HALITO")        dmg_spell(s, caster, a, "HALITO",   1, 8,  "火焰傷害");
    else if (n == "MOLITO")   dmg_spell(s, caster, a, "MOLITO",   3, 6,  "魔法飛彈");
    else if (n == "MAHALITO") dmg_spell(s, caster, a, "MAHALITO", 4, 6,  "群體火焰");
    else if (n == "LAHALITO") dmg_spell(s, caster, a, "LAHALITO", 6, 6,  "巨型火焰");
    else if (n == "DALTO")    dmg_spell(s, caster, a, "DALTO",    6, 6,  "冰錐");
    else if (n == "MADALTO")  dmg_spell(s, caster, a, "MADALTO",  8, 8,  "暴雪");
    else if (n == "LITOKAN")  dmg_spell(s, caster, a, "LITOKAN",  3, 8,  "火柱");
    else if (n == "LORTO")    dmg_spell(s, caster, a, "LORTO",    6, 6,  "飛刃");
    else if (n == "ZILWAN")   dmg_spell(s, caster, a, "ZILWAN",   3, 8,  "聖光（vs 不死）");

    // === Mage damage (all groups) ===
    else if (n == "TILTOWAIT") all_dmg_spell(s, caster, "TILTOWAIT", 10, 15, "核爆");
    else if (n == "MALIKTO")   all_dmg_spell(s, caster, "MALIKTO",   12,  6, "審判");

    // === Priest damage ===
    else if (n == "BADIOS")    dmg_spell(s, caster, a, "BADIOS",   1, 8, "傷害");
    else if (n == "BADIAL")    dmg_spell(s, caster, a, "BADIAL",   2, 8, "傷害");
    else if (n == "BADIALMA")  dmg_spell(s, caster, a, "BADIALMA", 3, 8, "傷害");
    else if (n == "LABADI") {
        if (auto* g = group_at(s, a.target_group)) {
            int dmg = g->hp_total;
            apply_dmg_to_group(*g, dmg);
            char buf[120];
            std::snprintf(buf, sizeof(buf), "奪命：%s 群被瞬殺！",
                          g->prototype.name_unknown.c_str());
            log_cast(s, caster, "LABADI", buf);
        }
    }
    else if (n == "MABADI") single_target_kill(s, caster, a, "MABADI");

    // === Healing ===
    else if (n == "DIOS")    heal_self(s, caster, "DIOS",    1, 8);
    else if (n == "DIAL")    heal_self(s, caster, "DIAL",    2, 8);
    else if (n == "DIALMA")  heal_self(s, caster, "DIALMA",  3, 8);
    else if (n == "MADI") {
        caster.hp_left = caster.hp_max;
        log_cast(s, caster, "MADI", "自身回滿 HP。");
    }

    // === Status (group) ===
    else if (n == "KATINO")
        group_status(s, caster, a, "KATINO", "沉睡", &CombatGroup::asleep);
    else if (n == "MANIFO")
        group_status(s, caster, a, "MANIFO", "麻痺", &CombatGroup::paralyzed);
    else if (n == "MONTINO")
        group_status(s, caster, a, "MONTINO", "靜默", &CombatGroup::silenced);
    else if (n == "MORLIS") {
        group_ac_debuff(s, caster, a, "MORLIS", +2);
        if (auto* g = group_at(s, a.target_group)) g->feared = true;
    }
    else if (n == "MAMORLIS") {
        all_groups_ac_debuff(s, caster, "MAMORLIS", +4);
        for (auto& g : s.groups) if (g.alive_count > 0) g.feared = true;
    }
    else if (n == "DILTO") group_ac_debuff(s, caster, a, "DILTO", +2);

    // === Self AC buffs ===
    else if (n == "MOGREF") self_ac_buff(s, caster, caster_idx, "MOGREF", -2);
    else if (n == "PORFIC") self_ac_buff(s, caster, caster_idx, "PORFIC", -4);
    else if (n == "SOPIC")  self_ac_buff(s, caster, caster_idx, "SOPIC",  -4);

    // === Party AC buffs ===
    else if (n == "KALKI")    party_ac_buff(s, caster, "KALKI",    -1);
    else if (n == "MATU")     party_ac_buff(s, caster, "MATU",     -2);
    else if (n == "BAMATU")   party_ac_buff(s, caster, "BAMATU",   -4);
    else if (n == "MAPORFIC") {
        party_ac_buff(s, caster, "MAPORFIC", -2);
        s.party_buff.combat_only = false;
    }
    else if (n == "MASOPIC") {
        party_ac_buff(s, caster, "MASOPIC", -4);
    }
    else if (n == "MOGATO") {
        s.party_buff.magic_resist = true;
        log_cast(s, caster, "MOGATO", "全隊抵抗魔法。");
    }

    // === Instant death ===
    else if (n == "MAKANITO") instant_kill_group(s, caster, a, "MAKANITO", 8);
    else if (n == "LAKANITO") {
        if (auto* g = group_at(s, a.target_group)) {
            if (g->prototype.unaffected_mask == 0) {
                instant_kill_group(s, caster, a, "LAKANITO", 99);
            } else {
                log_cast(s, caster, "LAKANITO", "對方抵抗了真空。");
            }
        }
    }

    // === Random / wish-like (party arg not passed; degenerate to caster-only) ===
    else if (n == "HAMAN" || n == "MAHAMAN") {
        // Simplified: pick from a subset that doesn't need party data here.
        auto& rng = global_rng();
        int choice = rng.range(1, 5);
        int age_weeks = (n == "MAHAMAN") ? 52 : 5;
        caster.age = static_cast<std::uint16_t>(caster.age + age_weeks);
        char buf[200];
        switch (choice) {
            case 1:
                caster.hp_left = caster.hp_max;
                std::snprintf(buf, sizeof(buf), "術者回滿 HP，老 %d 週。", age_weeks);
                log_cast(s, caster, n.c_str(), buf);
                break;
            case 2: {
                for (auto& g : s.groups) {
                    if (g.alive_count > 0) apply_dmg_to_group(g, rng.dice(4, 8));
                }
                std::snprintf(buf, sizeof(buf), "全敵受 4d8 傷害！老 %d 週。", age_weeks);
                log_cast(s, caster, n.c_str(), buf);
                break;
            }
            case 3:
                s.party_buff.ac_mod -= 4;
                std::snprintf(buf, sizeof(buf), "全隊 AC -4！老 %d 週。", age_weeks);
                log_cast(s, caster, n.c_str(), buf);
                break;
            case 4:
                for (auto& g : s.groups) if (g.alive_count > 0) g.asleep = true;
                std::snprintf(buf, sizeof(buf), "全敵沉睡！老 %d 週。", age_weeks);
                log_cast(s, caster, n.c_str(), buf);
                break;
            case 5: default:
                for (auto& g : s.groups) {
                    if (g.alive_count > 0) apply_dmg_to_group(g, rng.dice(10, 15));
                }
                std::snprintf(buf, sizeof(buf), "TILTOWAIT 等效爆炸！老 %d 週。", age_weeks);
                log_cast(s, caster, n.c_str(), buf);
                break;
        }
    }

    // === Out-of-combat / not yet implemented (just log) ===
    else if (n == "DUMAPIC")   log_cast(s, caster, "DUMAPIC",   "顯示位置（迷宮才生效）。");
    else if (n == "MILWA")     log_cast(s, caster, "MILWA",     "短時迷宮照明。");
    else if (n == "LOMILWA")   log_cast(s, caster, "LOMILWA",   "長時迷宮照明。");
    else if (n == "CALFO")     log_cast(s, caster, "CALFO",     "鑑定陷阱（寶箱才生效）。");
    else if (n == "LATUMAPIC") {
        for (auto& g : s.groups) g.identified = true;
        log_cast(s, caster, "LATUMAPIC", "看穿全敵真身。");
    }
    else if (n == "LATUMOFIS") {
        if (caster.status == Status::Ok) {
            // Cure poison handled by status; placeholder
        }
        log_cast(s, caster, "LATUMOFIS", "解除中毒。");
    }
    else if (n == "DIALKO") {
        if (caster.status == Status::Asleep || caster.status == Status::Paralyzed) {
            caster.status = Status::Ok;
        }
        log_cast(s, caster, "DIALKO", "解除麻痺與沉睡。");
    }
    else if (n == "KANDI")     log_cast(s, caster, "KANDI",     "指出特定角色位置（迷宮才生效）。");
    else if (n == "LOKTOFEIT") log_cast(s, caster, "LOKTOFEIT", "全隊瞬移回城（迷宮才生效）。");
    else if (n == "MALOR")     log_cast(s, caster, "MALOR",     "瞬間移動（迷宮才生效）。");
    else if (n == "DI")        log_cast(s, caster, "DI",        "復活屍體（營地才生效）。");
    else if (n == "KADORTO")   log_cast(s, caster, "KADORTO",   "復活灰燼（神殿才生效）。");

    else {
        log_cast(s, caster, n.c_str(), "效果尚未實作。");
    }
}

void begin_combat(CombatState& s, std::vector<CombatGroup> groups) {
    s.groups = std::move(groups);
    for (auto& g : s.groups) {
        if (g.alive_count == 0) g.alive_count = g.total_count;
        // pooled HP per monster ~ avg of hp dice
        int per = (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2;
        if (per < 1) per = 1;
        g.hp_total = per * g.alive_count;
    }
    s.phase = CombatPhase::PartyAction;
    s.outcome = CombatOutcome::Ongoing;
    s.xp_award = 0;
    s.gold_award = 0;
    s.active_party_member = 0;
    s.log.clear();
    char buf[160];
    std::snprintf(buf, sizeof(buf), "** 戰鬥開始 — %zu 群敵人出現！", s.groups.size());
    s.log.emplace_back(buf);
    for (const auto& g : s.groups) {
        std::snprintf(buf, sizeof(buf), "  %d × %s",
                      int(g.alive_count),
                      g.identified ? g.prototype.name.c_str()
                                   : g.prototype.name_unknown.c_str());
        s.log.emplace_back(buf);
    }
}

bool set_action(CombatState& s, int idx, PlayerAction a) {
    if (idx < 0 || idx >= 6) return false;
    s.actions[idx] = a;
    return true;
}

void resolve_round(CombatState& s, std::array<Character, 6>& party) {
    s.phase = CombatPhase::Resolve;
    char buf[160];

    // Player phase
    for (int i = 0; i < 6; ++i) {
        auto& a = s.actions[i];
        auto& c = party[i];
        if (c.name.empty() || c.status != Status::Ok) continue;
        if (a.kind == PlayerAction::Run) {
            // 50% chance to flee
            if (global_rng().range(1, 100) <= 50) {
                s.outcome = CombatOutcome::Fled;
                s.phase = CombatPhase::End;
                s.log.emplace_back("** 全員撤退成功！");
                return;
            }
        } else if (a.kind == PlayerAction::Parry) {
            std::snprintf(buf, sizeof(buf), "%s 採取防禦姿態。", c.name.c_str());
            s.log.emplace_back(buf);
        } else if (a.kind == PlayerAction::Fight) {
            if (a.target_group < 0 || static_cast<std::size_t>(a.target_group) >= s.groups.size()) continue;
            auto& g = s.groups[a.target_group];
            if (g.alive_count <= 0) continue;
            if (member_hits(c, g.prototype.armor_class)) {
                int dmg = member_attack_dmg(c);
                g.hp_total -= dmg;
                std::snprintf(buf, sizeof(buf), "%s 攻擊命中 %s，造成 %d 傷害。",
                              c.name.c_str(),
                              g.prototype.name_unknown.c_str(), dmg);
                s.log.emplace_back(buf);
                int avg_hp = std::max(1, (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
                int dead = g.alive_count - std::max(0, g.hp_total / avg_hp + 1);
                if (dead > 0) {
                    g.alive_count = std::max(0, g.alive_count - dead);
                    std::snprintf(buf, sizeof(buf), "  -> %d 隻被擊倒。", dead);
                    s.log.emplace_back(buf);
                }
            } else {
                std::snprintf(buf, sizeof(buf), "%s 攻擊落空。", c.name.c_str());
                s.log.emplace_back(buf);
            }
        }
    }

    // Check if all enemies dead
    bool any_alive = false;
    for (auto& g : s.groups) if (g.alive_count > 0) any_alive = true;
    if (!any_alive) {
        s.outcome = CombatOutcome::Victory;
        s.phase = CombatPhase::End;
        long_t xp = 0;
        long_t gold = 0;
        for (auto& g : s.groups) {
            xp += g.prototype.experience * g.total_count;
            gold += g.prototype.experience / 8 * g.total_count;
        }
        s.xp_award = xp;
        s.gold_award = gold;
        std::snprintf(buf, sizeof(buf), "** 勝利！ 經驗 %lld   金幣 %lld",
                      static_cast<long long>(xp), static_cast<long long>(gold));
        s.log.emplace_back(buf);
        return;
    }

    // Monster phase
    int alive_party = 0;
    for (auto& c : party) if (!c.name.empty() && c.status == Status::Ok) ++alive_party;
    if (alive_party == 0) {
        s.outcome = CombatOutcome::Defeat;
        s.phase = CombatPhase::End;
        s.log.emplace_back("** 全員倒下…");
        return;
    }

    for (auto& g : s.groups) {
        if (g.alive_count <= 0) continue;
        int swings = std::max(1, int(g.alive_count) / 2);
        for (int sw = 0; sw < swings; ++sw) {
            // pick a random alive party member
            int target = -1;
            for (int t = 0; t < 100 && target < 0; ++t) {
                int idx = global_rng().range(0, 5);
                if (!party[idx].name.empty() && party[idx].status == Status::Ok) target = idx;
            }
            if (target < 0) break;
            auto& victim = party[target];
            if (monster_hits(g.prototype, victim.armor_class)) {
                int dmg = monster_attack_dmg(g.prototype);
                victim.hp_left -= dmg;
                std::snprintf(buf, sizeof(buf), "%s 被 %s 擊中，受 %d 傷害。",
                              victim.name.c_str(),
                              g.prototype.name_unknown.c_str(), dmg);
                s.log.emplace_back(buf);
                if (victim.hp_left <= 0) {
                    victim.status = Status::Dead;
                    std::snprintf(buf, sizeof(buf), "  -> %s 倒下！", victim.name.c_str());
                    s.log.emplace_back(buf);
                }
            }
        }
    }

    s.phase = CombatPhase::PartyAction;
    s.active_party_member = 0;
}

}  // namespace wiz::core
