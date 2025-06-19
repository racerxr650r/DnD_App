/*
 * DnD.c
 *
 * Linux console Dungeons and Dragons player character sheet application
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

#include "CharSheet.h"
#include "lcaf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isdigit
#include <wctype.h>

// Internal Function Prototypes ***********************************************
int dndCalcModifier(int score, int proficiency_modifier);
void dndCalcDependincies(Window *win);

int get_int_input(int y, int x, const char *prompt);
void get_string_input(int y, int x, const char *prompt, char *buffer, int max_length);

void dndLoadCharacter(Window *win);
void dndSaveCharacter(Window *win);

Spell **dndLoadSpells(const char *filename, int *spell_count);
void dndFreeSpells(Spell **spells, int spell_count);

int dndCreateWindows(Window *screen);
int dndInput(Window *screen, int ch);

Window *dndCreateCharWin(Window *screen);
Window *dndCreateProfWin(Window *screen);
Window *dndCreateCombatWin(Window *screen);
Window *dndCreateMagicWin(Window *screen);
Window *dndCreateInvWin(Window *screen);
Window *dndCreateNotesWin(Window *screen);

// Globals ********************************************************************
Character character;

// Main Function **************************************************************
int main(int argc, char *argv[]) 
{
    int ret;
    Spell **spells = NULL;
    int num_spells = 0;

    setlocale(LC_ALL, "");

    spells = dndLoadSpells("DnD_5e_Spells.csv", &num_spells);

    // Initialize character data structure
    memset(&character, 0, sizeof(Character));

    // Create all app windows and register global app input handler
    Window *screen = scrnCreate();
    screen->configure = dndCreateWindows;
    screen->notify_input = dndInput;


    if(screen != NULL)
    {
        // Hook the update notification to calculate all dependencies
        screen->notify_update = dndCalcDependincies;

        // Run the screen
        scrnRun(screen);

        // clean up windowFW
        scrnDestroy(screen);
        ret = 0;
    }
    else
        ret = -1;

    dndFreeSpells(spells, num_spells);

    return ret;
}

// Function Definitions *******************************************************
int dndCreateWindows(Window *screen)
{
    // Create the windows in the opposite of the Z ordering
    //if(create_notes_window(screen) != NULL)
        //if(create_inventory_window(screen) != NULL)
            //if(create_magic_window(screen) != NULL)
                //if(create_combat_window(screen) != NULL)
                    //if(create_proficiencies_window(screen) != NULL)
                        if(dndCreateCharWin(screen) != NULL)
                            return(0);
    return(-1);
}

int dndQuit(Window *win, int ch)
{
    switch(ch)
    {
        case 'Y':
        case 'y':
            return(-1);
        case 'N':
        case 'n':
        case KEY_ESC:
            winMarkDestroy(win);
        case KEY_QUIT_PROGRAM:
            return(1);
    }
    return(0);
}

int dndInput(Window *screen, int input)
{
    Window *quit_win;

    // The following keys work in any window in the app
    switch(input)
    {
        case KEY_RIGHT:
        case KEY_NPAGE:
            screen->top_window->move_bottom(screen->top_window);
            break;
        case KEY_LEFT:
        case KEY_PPAGE:
            screen->bottom_window->move_top(screen->bottom_window);
            break;
        case KEY_SAVE_CHARACTER: // Save (ctl-s)
            dndSaveCharacter(screen);
            break;
        case KEY_LOAD_CHARACTER: // Load (ctl-l)
            dndLoadCharacter(screen);
            break;
        case KEY_QUIT_PROGRAM:
            quit_win = popupYesNo(screen, "Are you sure you want to quit? (Y/N)", dndQuit);
            if(quit_win == NULL)
                return(-1);
            break;
            //return(-1);
        default:
            // Key not consumed
            return(0);
    }
    // Key consumed
    return(1);
}

// Calculations----------------------------------------------------------------
int dndCalcModifier(int score, int proficiency_modifier) 
{
    return ((score - 10) / 2) + proficiency_modifier;
}

void dndCalcDependincies(Window *win)
{
    // Do this once here so this code is not spread all over the place and
    // duplicated in several places. Order is important.

    // Update ability modifiers
    character.abilities.strength.modifier = dndCalcModifier(character.abilities.strength.score,0);
    character.abilities.dexterity.modifier = dndCalcModifier(character.abilities.dexterity.score,0);
    character.abilities.constitution.modifier = dndCalcModifier(character.abilities.constitution.score,0);
    character.abilities.intelligence.modifier = dndCalcModifier(character.abilities.intelligence.score,0);
    character.abilities.wisdom.modifier = dndCalcModifier(character.abilities.wisdom.score,0);
    character.abilities.charisma.modifier = dndCalcModifier(character.abilities.charisma.score,0);
    // Update initiative
    character.initiative = character.abilities.dexterity.modifier;
    // Update character level
    character.level = (character.xp / 300) + 1;  //Simple level calculation
    // Update proficiency bonus, need to do this before saving throws and skills
    character.proficiency_bonus = 2 + (character.level -1)/4;
    // Update Saving Throws
    character.abilities.strength.save = character.abilities.strength.modifier + (character.abilities.strength.proficient?character.proficiency_bonus:0);
    character.abilities.dexterity.save = character.abilities.dexterity.modifier + (character.abilities.dexterity.proficient?character.proficiency_bonus:0);
    character.abilities.constitution.save = character.abilities.constitution.modifier + (character.abilities.constitution.proficient?character.proficiency_bonus:0);
    character.abilities.intelligence.save = character.abilities.intelligence.modifier + (character.abilities.intelligence.proficient?character.proficiency_bonus:0);
    character.abilities.wisdom.save = character.abilities.wisdom.modifier + (character.abilities.wisdom.proficient?character.proficiency_bonus:0);
    character.abilities.charisma.save = character.abilities.charisma.modifier + (character.abilities.charisma.proficient?character.proficiency_bonus:0);
    // Update Skills
    character.skills.acrobatics.modifier = character.abilities.dexterity.modifier + (character.skills.acrobatics.proficient?character.proficiency_bonus:0);
    character.skills.animal_handling.modifier = character.abilities.wisdom.modifier + (character.skills.animal_handling.proficient?character.proficiency_bonus:0);
    character.skills.arcana.modifier = character.abilities.intelligence.modifier + (character.skills.arcana.proficient?character.proficiency_bonus:0);
    character.skills.athletics.modifier = character.abilities.strength.modifier + (character.skills.athletics.proficient?character.proficiency_bonus:0);
    character.skills.deception.modifier = character.abilities.charisma.modifier + (character.skills.deception.proficient?character.proficiency_bonus:0);
    character.skills.history.modifier = character.abilities.intelligence.modifier + (character.skills.history.proficient?character.proficiency_bonus:0);
    character.skills.insight.modifier = character.abilities.wisdom.modifier + (character.skills.insight.proficient?character.proficiency_bonus:0);
    character.skills.intimidation.modifier = character.abilities.charisma.modifier + (character.skills.intimidation.proficient?character.proficiency_bonus:0);
    character.skills.investigation.modifier = character.abilities.intelligence.modifier + (character.skills.investigation.proficient?character.proficiency_bonus:0);
    character.skills.medicine.modifier = character.abilities.wisdom.modifier + (character.skills.medicine.proficient?character.proficiency_bonus:0);
    character.skills.nature.modifier = character.abilities.intelligence.modifier + (character.skills.nature.proficient?character.proficiency_bonus:0);
    character.skills.perception.modifier = character.abilities.wisdom.modifier + (character.skills.perception.proficient?character.proficiency_bonus:0);
    character.skills.performance.modifier = character.abilities.charisma.modifier + (character.skills.performance.proficient?character.proficiency_bonus:0);
    character.skills.persuasion.modifier = character.abilities.charisma.modifier + (character.skills.persuasion.proficient?character.proficiency_bonus:0);
    character.skills.religion.modifier = character.abilities.intelligence.modifier  + (character.skills.religion.proficient?character.proficiency_bonus:0);
    character.skills.sleight_of_hand.modifier = character.abilities.dexterity.modifier + (character.skills.sleight_of_hand.proficient?character.proficiency_bonus:0);
    character.skills.stealth.modifier = character.abilities.dexterity.modifier + (character.skills.stealth.proficient?character.proficiency_bonus:0);
    character.skills.survival.modifier = character.abilities.wisdom.modifier + (character.skills.survival.proficient?character.proficiency_bonus:0);
    // Update passive skills
    character.passive_insight = character.skills.insight.modifier + 10;
    character.passive_investigation = character.skills.investigation.modifier + 10;
    character.passive_perception = character.skills.perception.modifier + 10;
    character.passive_stealth = character.skills.stealth.modifier + 10;
}

// Application Windows --------------------------------------------------------
int dndAbilityHandler(Component *component, int ch)
{
    int result = 0;

    // If there is an associated component (proficient)...
    if(component)
        if(component->next)
            if(component->next->next)
                if(component->next->next->next)
                    if(component->next->next->next->input)
                        // Send the input tot he associated saving throw profenciency
                        result = component->next->next->next->input(component->next->next->next,ch);

    return(result);
}

Window *dndCreateCharWin(Window *screen)
{
    // Create window ----------------------------------------------------------
    Window *win_base = winCreate(screen,0,0,screen->height,screen->width,"Character",true);
    if(win_base == NULL)
        return(NULL);
    win_base->frame_type = FRAME_HYBRID;

    Component *comp;

    // Character fields -------------------------------------------------------
    Window *win_character = winCreate(win_base,1,2,8,39,NULL,true);
    txtCreate(win_character,0,2,"Character");
    win_character->frame_type = FRAME_LIGHT_ARC;
    int row = 1;
    int col = 1;
    strCreate(win_character,row++,col,30," Name: ",character.name,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_character,row++,col,30,"Class: ",character.class,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_character,row++,col,30," Race: ",character.race,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_character,row++,col,30,"Bkgrd: ",character.background,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_character,row++,col,30,"Align: ",character.alignment,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_character,row++,col,30,"  Sex: ",character.sex,MAX_TEXT_FIELD_LENGTH);

    // Stats fields -----------------------------------------------------------
    Window *win_stats = winCreate(win_base,9,2,5,52,NULL,true);
    txtCreate(win_stats,0,2,"Stats");
    win_stats->frame_type = FRAME_LIGHT_ARC;
    row = 1;
    col = 1;
    intCreate(win_stats,row,col,3,"   Age: ",&character.age);
    intCreate(win_stats,row,col+15,3,"Speed: ",&character.speed);
    comp = intCreate(win_stats,row++,col+33,2,"Level: ",&character.level);
    compReadOnly(comp);
    strCreate(win_stats,row,col,6,"Height: ",character.height,MAX_TEXT_FIELD_LENGTH);
    strCreate(win_stats,row,col+15,10," Hair: ",character.hair,MAX_TEXT_FIELD_LENGTH);
    intCreate(win_stats,row++,col+33,6,"   XP: ",&character.xp);
    intCreate(win_stats,row,col,3,"Weight: ",&character.weight);
    strCreate(win_stats,row,col+15,10," Eyes: ",character.eyes,MAX_TEXT_FIELD_LENGTH);
    comp = intCreate(win_stats,row++,col+33,2," Prof: ",&character.proficiency_bonus);
    compReadOnly(comp);
    compSetFormat(comp,"%+*d");

    // Description Text Edit ---------------------------------------------------
    Window *win_desc = winCreate(win_base,14,2,9,52,NULL,true);
    txtCreate(win_desc,0,2,"Description");
    win_desc->frame_type = FRAME_LIGHT_ARC;
    row = 1;
    col = 1;
    //txteditCreate(win_desc,row,col,win_desc->height-2,win_desc->width-2,character.description,MAX_DESCR_LENGTH);

    // Abilities --------------------------------------------------------------
    Window *win_abilities = winCreate(win_base,1,42,8,36,NULL,true);
    win_abilities->frame_type = FRAME_LIGHT_ARC;
    row = 0;
    txtCreate(win_abilities,row,2,"Abilities");
    txtCreate(win_abilities,row,14,"Mod");
    txtCreate(win_abilities,row,18,"Save");
    txtCreate(win_abilities,row++,24,"Proficient");
    col = 2;
    // Strength
    comp = intCreate(win_abilities,row,col,2,"STR: ",&character.abilities.strength.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.strength.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,1,NULL, &character.abilities.strength.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.strength.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";

    // Dexerity
    comp = intCreate(win_abilities,row,col,2,"DEX: ",&character.abilities.dexterity.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.dexterity.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,1,NULL, &character.abilities.dexterity.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.dexterity.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";

    // Constitution
    comp = intCreate(win_abilities,row,col,2,"CON: ",&character.abilities.constitution.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.constitution.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,1,NULL, &character.abilities.constitution.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.constitution.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";
 
    // Intelligence
    comp = intCreate(win_abilities,row,col,2,"INT: ",&character.abilities.intelligence.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.intelligence.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,2,NULL, &character.abilities.intelligence.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.intelligence.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";

    // Wisdom
    comp = intCreate(win_abilities,row,col,2,"WIS: ",&character.abilities.wisdom.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.wisdom.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,1,NULL, &character.abilities.wisdom.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.wisdom.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";

    // Charisma
    comp = intCreate(win_abilities,row,col,2,"CHA: ",&character.abilities.charisma.score);
    comp->notify_input = dndAbilityHandler;
    comp = intCreate(win_abilities,row,col+10,2,NULL, &character.abilities.charisma.modifier);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = intCreate(win_abilities,row,col+15,1,NULL, &character.abilities.charisma.save);
    compReadOnly(comp);
    compSetFormat(comp," (%+*d)");
    comp = chkboxCreate(win_abilities,row++,col+25,1,NULL, &character.abilities.charisma.proficient);
    compReadOnly(comp);
    ((Checkbox *)comp)->true_string = "X";

    // Combat Stats -----------------------------------------------------------
    Window *win_combat = winCreate(win_base,9,55,9,23,NULL,true);
    txtCreate(win_combat,0,2,"Combat Stats");
    win_combat->frame_type = FRAME_LIGHT_ARC;
    row = 1;
    col = 1;
    comp = intCreate(win_combat,row++,col,3," Initiative: ",&character.initiative);
    compReadOnly(comp);
    intCreate(win_combat,row++,col,3," Hit Points: ",&character.hp_current);
    intCreate(win_combat,row++,col,3,"        Max: ",&character.hp_max);
    intCreate(win_combat,row++,col,3,"        Tmp: ",&character.hp_temp);
    intCreate(win_combat,row++,col,2,"Armor Class: ",&character.armor_class);
    intCreate(win_combat,row++,col,2," W/O Shield: ",&character.armor_class);
    intCreate(win_combat,row++,col,2,"   No Armor: ",&character.armor_class);

    // Languages --------------------------------------------------------------
    Window *win_languages = winCreate(win_base,18,55,5,23,NULL,true);
    txtCreate(win_languages,0,2,"Languages");
    win_languages->frame_type = FRAME_LIGHT_ARC;
    row = 1;
    col = 1;
    listCreate(win_languages,row,col,3,20,NULL,(char *)character.languages,MAX_LANUAGES,MAX_LANGUAGE_DESCRIPTION);

    //winSetFocus(win, name);
    winFirstFocus(win_base);

    return(win_base);
}

Window *dndCreateProfWin(Window *screen)
{
    Window *win = winCreate(screen,0,0,screen->height,screen->width,"Proficiencies",true);
    win->frame_type = FRAME_HYBRID;

    Component *this;
    Component *focus;
    int row = 1;

    txtCreate(win,row++,2,"Skills----------------");
    this = intCreate(win,row,2,1,"Acrobatics      ", &character.skills.acrobatics.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    focus = chkboxCreate(win,row++,21,1,NULL, &character.skills.acrobatics.proficient);

    this = intCreate(win,row,2,1,"Animal Handling ", &character.skills.animal_handling.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.animal_handling.proficient);

    this = intCreate(win,row,2,1,"Arcana          ", &character.skills.arcana.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.arcana.proficient);

    this = intCreate(win,row,2,1,"Athletics       ", &character.skills.athletics.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.athletics.proficient);

    this = intCreate(win,row,2,1,"Deception       ", &character.skills.deception.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.deception.proficient);

    this = intCreate(win,row,2,1,"History         ", &character.skills.history.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.history.proficient);

    this = intCreate(win,row,2,1,"Insight         ", &character.skills.insight.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.insight.proficient);

    this = intCreate(win,row,2,1,"Intimidation    ", &character.skills.intimidation.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.intimidation.proficient);

    this = intCreate(win,row,2,1,"Investigation   ", &character.skills.investigation.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.investigation.proficient);

    this = intCreate(win,row,2,1,"Medicine        ", &character.skills.medicine.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.medicine.proficient);

    this = intCreate(win,row,2,1,"Nature          ", &character.skills.nature.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.nature.proficient);

    this = intCreate(win,row,2,1,"Perception      ", &character.skills.perception.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.perception.proficient);

    this = intCreate(win,row,2,1,"Performance     ", &character.skills.performance.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.performance.proficient);

    this = intCreate(win,row,2,1,"Persuasion      ", &character.skills.persuasion.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.persuasion.proficient);

    this = intCreate(win,row,2,1,"Religion        ", &character.skills.religion.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.religion.proficient);

    this = intCreate(win,row,2,1,"Sleight of Hand ", &character.skills.sleight_of_hand.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.sleight_of_hand.proficient);

    this = intCreate(win,row,2,1,"Stealth         ", &character.skills.stealth.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.stealth.proficient);

    this = intCreate(win,row,2,1,"Survival        ", &character.skills.survival.modifier);
    compReadOnly(this);
    compSetFormat(this,"%s%+d");
    chkboxCreate(win,row++,21,1,NULL, &character.skills.survival.proficient);

    // Calculate the right column
    int right_column_start = (win->width / 2) + 2; // Add spacing between columns

    listCreate(win,1,right_column_start,5,40, "Tools---", (char *)character.inventory.tools, MAX_TOOLS, MAX_TOOL_DESCRIPTION);
    listCreate(win,7,right_column_start,5,40, "Armor---", (char *)character.inventory.armor, MAX_ARMOR, MAX_ARMOR_DESCRIPTION);
    listCreate(win,13,right_column_start,5,40, "Weapons--", (char *)character.inventory.weapons, MAX_WEAPONS, MAX_WEAPON_DESCRIPTION);

    // Set the focus for the window
    winSetFocus(win, focus);

    return(win);
}

Window *dndCreateCombatWin(Window *screen)
{
    Window *win = winCreate(screen,0,0,screen->height,screen->width,"Combat",true);
    win->frame_type = FRAME_HYBRID;
    return(win);

    int row = 1;

    strCreate(win,row++,1,20,"Weapon: ",character.attack[0].name,MAX_TEXT_FIELD_LENGTH);
    chkboxCreate(win,row,1,1,"Str: ",&character.attack[0].strength);
    chkboxCreate(win,row++,10,1,"Range: ",&character.attack[0].range);
    chkboxCreate(win,row,1,1,"Reach: ",&character.attack[0].reach);
    chkboxCreate(win,row++,12,1,"Prof: ",&character.attack[0].proficient);

}

Window *dndCreateMagicWin(Window *screen)
{
    Window *win = winCreate(screen,0,0,screen->height,screen->width,"Magic",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *dndCreateInvWin(Window *screen)
{
    Window *win = winCreate(screen,0,0,screen->height,screen->width,"Inventory",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *dndCreateNotesWin(Window *screen)
{
    Window *win = winCreate(screen,0,0,screen->height,screen->width,"Notes",true);
    win->frame_type = FRAME_HYBRID;

    txteditCreate(win, 1, 1, win->height-2, win->width-2, character.notes, MAX_NOTES_LENGTH);

    return(win);
}

// File I/O Functions ---------------------------------------------------------
void dndSaveCharActionHandler(Window *screen)
{
    char filename[MAX_TEXT_FIELD_LENGTH + 12]; // +12 for ".charsheet" and null terminator
    snprintf(filename, sizeof(filename), "%s.charsheet", character.name);

    FILE *file = fopen(filename, "wb"); // "wb" for writing binary data
    // If could not open file to write...
    if (file == NULL)
        popupError(screen, "Could not open file!", MESSAGE_DURATION);
    // Else opened file to write...
    else
    {
        fwrite(&character, sizeof(Character), 1, file); // Write the entire character struct
        fclose(file);
        // Display success message (clear after a short delay)
        popupMessage(screen,"Character saved!", MESSAGE_DURATION);
    }
}

int dndSaveCharInputHandler(Window *win, int ch)
{
    // Assume the input was consumed.
    int ret = 1;

    // If positive acknowledge...
    if(ch == 'Y' || ch == 'y')
    {
        dndSaveCharActionHandler(win->parent);
        winMarkDestroy(win);
    }
    // Else if negative acknowledge...
    else if(ch == 'N' || ch == 'n' || ch == KEY_ESC)
        winMarkDestroy(win);
    // Else if the user didn't tey to save again...
    else if(ch != KEY_SAVE_CHARACTER)
        ret = 0;

    return(ret);
}
    
void dndSaveCharacter(Window *screen) 
{
    if(strcmp(character.name, ""))
    {
        char filename[MAX_TEXT_FIELD_LENGTH + 12]; // +12 for ".charsheet" and null terminator
        snprintf(filename, sizeof(filename), "%s.charsheet", character.name);
        // If the file already exists...
        if(access(filename, F_OK) == 0)
            popupYesNo(screen,"Character Exists, Overwrite? (Y/N)",dndSaveCharInputHandler);
        // If the file doesn't exist...
        else
            dndSaveCharActionHandler(screen);
    }
    else
        popupError(screen,"Please enter a character name before saving",MESSAGE_DURATION);
}

int dndLoadCharActionHandler(Component *base, int ch)
{
    if(ch != KEY_ESC)
    {
        String *str = (String *)base;
        char *input_name = str->value;
        char *filename;

        asprintf(&filename, "%s.charsheet", input_name);
        FILE *file = fopen(filename, "rb");
        free(filename);
        
        // Error handling
        if (file == NULL) 
             popupError(base->parent->parent,"Character not found!", MESSAGE_DURATION);
        else 
        {
            fread(&character, sizeof(Character), 1, file);
            fclose(file);
            popupMessage(base->parent->parent,"Character loaded!", MESSAGE_DURATION);
            winMarkDestroy(base->parent);
        }
    }
    return(ch);
}

void dndLoadCharacter(Window *screen) 
{
    static char filename[MAX_TEXT_FIELD_LENGTH];
    strcpy(filename,"");
    
    popupGetString(screen, "Filename: ", filename, 20, dndLoadCharActionHandler);    
}

// Function to parse a spell from a CSV line
Spell *dndParseSpell(FILE *file)
{
    char *field = NULL;
    // Allocate and initialize a new spell
    Spell *spell = malloc(sizeof(Spell));
    if(spell == NULL)
        return NULL;
    memset(spell, 0, sizeof(Spell));

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->name, csvTrim(field), sizeof(spell->name) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->level = atoi(csvTrim(field));
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->school, csvTrim(field), sizeof(spell->school) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->casting_time, csvTrim(field), sizeof(spell->casting_time) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->duration, csvTrim(field), sizeof(spell->duration) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->range, csvTrim(field), sizeof(spell->range) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->area, csvTrim(field), sizeof(spell->area) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->attack, csvTrim(field), sizeof(spell->attack) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->save, csvTrim(field), sizeof(spell->save) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->damage_type, csvTrim(field), sizeof(spell->damage_type) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->ritual = (strcmp(csvTrim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->concentration = (strcmp(csvTrim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->verbal = (strcmp(csvTrim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->somatic = (strcmp(csvTrim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    spell->component = (strcmp(csvTrim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->material, csvTrim(field), sizeof(spell->material) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->source, csvTrim(field), sizeof(spell->source) - 1);
    free(field);
    field = NULL;

    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->details, csvTrim(field), sizeof(spell->details) - 1);
    free(field);
    field = NULL;

    // Burn this last field, not using the link
    if((field = csvReadField(file)) == NULL)
        goto End_Of_File;
    free(field);
    field = NULL;

    return spell;

End_Of_File:
    free(spell);
    return NULL;
}

// Function to load spells from the CSV file into an array
Spell **dndLoadSpells(const char *filename, int *spell_count)
{
    int i = 0;
    Spell **spells_array = NULL;
    FILE *file = fopen(filename, "r");
    if (!file)
        return NULL; // Indicate an error


    // Advance past file header
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n');

    do
    {
        spells_array = (Spell **)realloc(spells_array, (i + 1) * sizeof(Spell *));
        spells_array[i] = dndParseSpell(file);
    }while(spells_array[i++]!=NULL);
    *spell_count = i;

    fclose(file);
    return spells_array; // Return the number of spells loaded
}

void dndFreeSpells(Spell **spells, int spell_count)
{
    int i;
    for (i = 0; i < spell_count; i++)
    {
        if (spells[i] != NULL)
        {
            free(spells[i]);
            spells[i] = NULL;
        }
    }
    free(spells);
}
