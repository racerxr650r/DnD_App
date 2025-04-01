/*
 * CharSheet.h
 *
 * Constants and data types for the Linux console Dungeons and Dragons
 * player character sheet application
 *
 * Created: 03/31/2025
 * Author : john anderson
 *
 * Copyright (C) 2025 by John Anderson <racerxr650r@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */ 

// Header Sentry
#ifndef CharSheet_h
#define CharSheet_h

#include <stdbool.h>
#include <wchar.h>

// Constants ******************************************************************
// If you change these, prior character files will be incompatible
#define MAX_TOOLS                   10
#define MAX_TOOL_DESCRIPTION        40
#define MAX_ARMOR                   10
#define MAX_ARMOR_DESCRIPTION       40
#define MAX_WEAPONS                 15
#define MAX_WEAPON_DESCRIPTION      40
#define MAX_EQUIPMENT               30
#define MAX_EQUIPMENT_DESCRIPTION   40
#define MAX_TREASURE                40
#define MAX_TREASURE_DESCRIPTION    40
#define MAX_LANUAGES                10
#define MAX_LANGUAGE_DESCRIPTION    40

#define MAX_TEXT_FIELD_LENGTH 41
#define MAX_SKILLS 18  // Number of skills

#define MAX_NOTES_LENGTH            65535

// Keys
#define KEY_SAVE_CHARACTER '\023'
#define KEY_LOAD_CHARACTER '\014'
#define KEY_TOGGLE         '\024'
#define KEY_QUIT_PROGRAM   '\021'

#define MESSAGE_DURATION 1500

#define LIST_MAX_SIZE 200

// D&D Character Data Types ***************************************************
// Abilities -----
typedef enum
{
    STRENGTH,
    DEXTERITY,
    CONSTITUTION,
    INTELLIGENCE,
    WISDOM,
    CHARISMA
}Ability_Index;

typedef struct
{
    int     score;
    bool    proficient;
    int     modifier;
    int     save;
} Ability;

typedef struct 
{
    Ability strength;
    Ability dexterity;
    Ability constitution;
    Ability intelligence;
    Ability wisdom;
    Ability charisma;
} Abilities;

// Skills -----
typedef struct 
{
    int     modifier;
    bool    proficient;
}Skill;

typedef struct
{
    Skill   acrobatics;
    Skill   animal_handling;
    Skill   arcana;
    Skill   athletics;
    Skill   deception;
    Skill   history;
    Skill   insight;
    Skill   intimidation;
    Skill   investigation;
    Skill   medicine;
    Skill   nature;
    Skill   perception;
    Skill   performance;
    Skill   persuasion;
    Skill   religion;
    Skill   sleight_of_hand;
    Skill   stealth;
    Skill   survival;
}Skills;

// Inventory -----
typedef struct
{
    char    tools[MAX_TOOLS][MAX_TOOL_DESCRIPTION];
    char    armor[MAX_ARMOR][MAX_ARMOR_DESCRIPTION];
    char    weapons[MAX_WEAPONS][MAX_WEAPON_DESCRIPTION];
    char    equipment[MAX_EQUIPMENT][MAX_EQUIPMENT_DESCRIPTION];
    int     copper;
    int     silver;
    int     electrum;
    int     gold;
    int     platinum;
    char    treasure_items[MAX_TREASURE][MAX_TREASURE_DESCRIPTION];
}Inventory;

// Spells -----
typedef struct 
{
    char    name[MAX_TEXT_FIELD_LENGTH];
    int     level;
    char    school[MAX_TEXT_FIELD_LENGTH];
    char    casting_time[MAX_TEXT_FIELD_LENGTH];
    char    duration[MAX_TEXT_FIELD_LENGTH];
    char    range[MAX_TEXT_FIELD_LENGTH];
    char    area[MAX_TEXT_FIELD_LENGTH];
    char    attack[MAX_TEXT_FIELD_LENGTH];
    char    save[MAX_TEXT_FIELD_LENGTH];
    char    damage_type[MAX_TEXT_FIELD_LENGTH];
    bool    ritual;
    bool    concentration;
    bool    verbal;
    bool    somatic;
    bool    component;
    char    material[MAX_TEXT_FIELD_LENGTH];
    char    source[MAX_TEXT_FIELD_LENGTH];
    char    details[16384];
    int     damage_dice_count;
    int     damage_dice_sides;
}Spell;

typedef struct 
{
    char    name[MAX_TEXT_FIELD_LENGTH];
    int     level;
    bool    known;
    char    description[16384];
}Spells;

// Magic and Physical Attacks -----
typedef struct 
{
    int     primary_ability_modifier;
    int     spell_save;
    int     spell_attack;
    int     sorcery_points;
    int     max_known_spells;
    int     known_spells;
    int     spell_slots[9];
    Spells  spells[200];
}Magic;

typedef struct 
{
    char    name[MAX_TEXT_FIELD_LENGTH];
    bool    strength;
    bool    range;
    bool    reach;
    bool    proficient;
    int     magic;
    int     attack_bonus;
    int     damage_bonus;
    int     damage_dice_count;
    int     damage_dice_sides;
    char    damage_type[MAX_TEXT_FIELD_LENGTH];
}Attack;

// Character -----
typedef struct 
{
    // Character
    char        player_name[MAX_TEXT_FIELD_LENGTH];
    char        name[MAX_TEXT_FIELD_LENGTH];
    char        class[MAX_TEXT_FIELD_LENGTH];
    char        race[MAX_TEXT_FIELD_LENGTH];
    char        background[MAX_TEXT_FIELD_LENGTH];
    char        alignment[MAX_TEXT_FIELD_LENGTH];
    char        sex[MAX_TEXT_FIELD_LENGTH];
    int         age;
    char        height[MAX_TEXT_FIELD_LENGTH];
    int         weight;
    int         level;
    int         xp;
    int         proficiency_bonus;
    int         armor_class;
    int         speed;
    int         initiative;
    char        languages[20][MAX_TEXT_FIELD_LENGTH];
    int         passive_perception;
    int         passive_stealth; 
    int         passive_insight;
    int         passive_investigation;
    Abilities   abilities;

    int         hp_current;
    int         hp_max;
    int         hp_temp;
    int         hit_dice;

    // Proficiencies
    Skills      skills;
    int         exhaustion;
    int         death_saves;
    bool        inspiration;
    Attack      attack[5];

    // Magic
    Magic       magic;

    // Inventory
    Inventory   inventory;

    // Notes
    wchar_t     notes[MAX_NOTES_LENGTH];
} Character;

#endif //CarSheet_h