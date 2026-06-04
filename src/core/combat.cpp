#include "core/combat.h"

#include <algorithm>
#include <cstdio>

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
        } else if (a.kind == PlayerAction::Spell) {
            // Very lite spell resolver — supports a handful of W1 staples.
            const std::string& name = a.spell_name;
            auto& rng = global_rng();
            if (name == "HALITO") {
                if (a.target_group >= 0 &&
                    static_cast<std::size_t>(a.target_group) < s.groups.size()) {
                    auto& g = s.groups[a.target_group];
                    int dmg = rng.dice(1, 8);
                    g.hp_total -= dmg;
                    std::snprintf(buf, sizeof(buf),
                                  "%s 詠唱 HALITO — %s 受 %d 火焰傷害。",
                                  c.name.c_str(),
                                  g.prototype.name_unknown.c_str(), dmg);
                    s.log.emplace_back(buf);
                    int avg_hp = std::max(1,
                        (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
                    int dead = g.alive_count - std::max(0, g.hp_total / avg_hp + 1);
                    if (dead > 0) {
                        g.alive_count = std::max(0, g.alive_count - dead);
                        std::snprintf(buf, sizeof(buf), "  -> %d 隻被燒倒。", dead);
                        s.log.emplace_back(buf);
                    }
                }
            } else if (name == "MAHALITO") {
                if (a.target_group >= 0 &&
                    static_cast<std::size_t>(a.target_group) < s.groups.size()) {
                    auto& g = s.groups[a.target_group];
                    int dmg = rng.dice(4, 6);
                    g.hp_total -= dmg;
                    std::snprintf(buf, sizeof(buf),
                                  "%s 詠唱 MAHALITO — %s 受 %d 群體火焰傷害。",
                                  c.name.c_str(),
                                  g.prototype.name_unknown.c_str(), dmg);
                    s.log.emplace_back(buf);
                    int avg_hp = std::max(1,
                        (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
                    int dead = g.alive_count - std::max(0, g.hp_total / avg_hp + 1);
                    if (dead > 0) {
                        g.alive_count = std::max(0, g.alive_count - dead);
                        std::snprintf(buf, sizeof(buf), "  -> %d 隻被燒倒。", dead);
                        s.log.emplace_back(buf);
                    }
                }
            } else if (name == "KATINO") {
                if (a.target_group >= 0 &&
                    static_cast<std::size_t>(a.target_group) < s.groups.size()) {
                    auto& g = s.groups[a.target_group];
                    int dead = std::min<int>(g.alive_count, rng.dice(1, 4));
                    g.alive_count = std::max(0, g.alive_count - dead);
                    std::snprintf(buf, sizeof(buf),
                                  "%s 詠唱 KATINO — %s 中 %d 隻沉睡。",
                                  c.name.c_str(),
                                  g.prototype.name_unknown.c_str(), dead);
                    s.log.emplace_back(buf);
                }
            } else if (name == "DIOS") {
                int heal = rng.dice(1, 8);
                c.hp_left = std::min<int>(c.hp_max, c.hp_left + heal);
                std::snprintf(buf, sizeof(buf),
                              "%s 詠唱 DIOS — 自身 +%d HP。",
                              c.name.c_str(), heal);
                s.log.emplace_back(buf);
            } else if (name == "BADIOS") {
                if (a.target_group >= 0 &&
                    static_cast<std::size_t>(a.target_group) < s.groups.size()) {
                    auto& g = s.groups[a.target_group];
                    int dmg = rng.dice(1, 8);
                    g.hp_total -= dmg;
                    std::snprintf(buf, sizeof(buf),
                                  "%s 詠唱 BADIOS — %s 受 %d 傷害。",
                                  c.name.c_str(),
                                  g.prototype.name_unknown.c_str(), dmg);
                    s.log.emplace_back(buf);
                    int avg_hp = std::max(1,
                        (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
                    int dead = g.alive_count - std::max(0, g.hp_total / avg_hp + 1);
                    if (dead > 0) {
                        g.alive_count = std::max(0, g.alive_count - dead);
                    }
                }
            } else if (name == "TILTOWAIT") {
                for (auto& g : s.groups) {
                    if (g.alive_count <= 0) continue;
                    int dmg = rng.dice(10, 15);
                    g.hp_total -= dmg;
                    std::snprintf(buf, sizeof(buf),
                                  "  TILTOWAIT — %s 受 %d 核爆。",
                                  g.prototype.name_unknown.c_str(), dmg);
                    s.log.emplace_back(buf);
                    int avg_hp = std::max(1,
                        (g.prototype.hp_dice_n * (g.prototype.hp_dice_d + 1)) / 2);
                    int dead = g.alive_count - std::max(0, g.hp_total / avg_hp + 1);
                    if (dead > 0) {
                        g.alive_count = std::max(0, g.alive_count - dead);
                    }
                }
                std::snprintf(buf, sizeof(buf), "%s 詠唱 TILTOWAIT！", c.name.c_str());
                s.log.emplace_back(buf);
            } else {
                std::snprintf(buf, sizeof(buf),
                              "%s 詠唱 %s — 效果尚未實作。",
                              c.name.c_str(), name.c_str());
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
