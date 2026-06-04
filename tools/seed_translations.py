#!/usr/bin/env python3
"""Seed core UI translations into strings_zh_tw.json.

Covers the most common short strings: titles, attribute labels, menu actions,
common prompts. Anything substantial is left blank for human translation.
"""
import json
import sys
from pathlib import Path


SEED = {
    # Titles / locations
    "castle": "城堡",
    "tavern": "酒館",
    "shop": "商店",
    "temple": "神殿",
    "inn": "旅館",
    "maze": "迷宮",
    "training_grounds": "訓練場",
    "camp": "營地",
    "edge_of_town": "城鎮邊緣",
    "boltac_s_trading_post": "波塔克貿易站",
    "gilgamesh_s_tavern": "吉爾伽美什酒館",
    "adventurer_s_inn": "冒險者旅館",
    "cant_s_temple": "坎特神殿",

    # Attribute labels
    "strength": "力量",
    "iq": "智力",
    "piety": "信仰",
    "vitality": "體力",
    "agility": "敏捷",
    "luck": "幸運",
    "age": "年齡",
    "ac": "AC",
    "hp": "HP",
    "exp": "經驗",
    "gold": "金幣",
    "level": "等級",
    "alignment": "陣營",
    "armor": "盔甲",
    "race": "種族",
    "class": "職業",
    "name": "名字",
    "status": "狀態",
    "hits": "生命",

    # Common prompts
    "confirm_y_n": "確認 (Y/N)？",
    "press_return_to_continue": "按 [RETURN] 繼續",
    "choose_a_race": "選擇種族 >",
    "choose_a_class": "選擇職業 >",
    "choose_an_alignment": "選擇陣營 >",
    "amt_of_gold": "金額？>",
    "who_will_join": "誰要加入？>",
    "who_will_leave": "誰要離開",

    # Common menu options
    "a": "A",
    "b": "B",
    "c": "C",
    "d": "D",
    "e": "E",
    "y": "是",
    "n": "否",

    # Common errors / status
    "who": "**誰？**",
    "out": "**外出中**",
    "bad_alignment": "**陣營不合**",
    "not_here": "**不在這裡**",
    "lost": "迷失",
    "poison": "中毒",
    "dead": "死亡",
    "ashes": "化灰",

    # Combat
    "murmur": "低語 - ",
    "chant": "唱誦 - ",
    "pray": "祈禱 - ",
    "invoke": "施法！",

    # Misc
    "yes": "是",
    "no": "否",
}


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: seed_translations.py <catalogue.json>", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    data = json.loads(path.read_text(encoding="utf-8"))
    seeded = 0
    for key, zh in SEED.items():
        if key in data and isinstance(data[key], dict) and not data[key].get("zh_TW"):
            data[key]["zh_TW"] = zh
            seeded += 1
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"seeded {seeded} translations (of {len(SEED)} attempted)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
