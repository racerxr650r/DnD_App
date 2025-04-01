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
int calculate_modifier(int score, int proficiency_modifier);
void calculate_dependencies(Window *win);

int get_int_input(int y, int x, const char *prompt);
void get_string_input(int y, int x, const char *prompt, char *buffer, int max_length);

void load_character(Window *win);
void save_character(Window *win);
Spell **load_spells(const char *filename, int *spell_count);

int create_app_windows(Window *screen);
int app_input(Window *screen, int ch);

Window *create_character_window(Window *screen);
Window *create_proficiencies_window(Window *screen);
Window *create_combat_window(Window *screen);
Window *create_magic_window(Window *screen);
Window *create_inventory_window(Window *screen);
Window *create_notes_window(Window *screen);

// Globals ********************************************************************
Character character;

// Main Function **************************************************************
int main(int argc, char *argv[]) 
{
    int ret;
    Spell **spells = NULL;
    int num_spells = 0;

    setlocale(LC_ALL, "");
    
    spells = load_spells("DnD_5e_Spells.csv", &num_spells);

    // Initialize character data structure
    memset(&character, 0, sizeof(Character));

    // Create all app windows and register global app input handler
    Window *screen = create_screen();
    screen->configure = create_app_windows;
    screen->notify_input = app_input;


    if(screen != NULL)
    {
        // Hook the update notification to calculate all dependencies
        screen->notify_update = calculate_dependencies;

        // Run the screen
        run_screen(screen);

        // clean up windowFW
        destroy_screen(screen);
        ret = 0;
    }
    else
        ret = -1;

    return ret;
}

// Function Definitions *******************************************************
int create_app_windows(Window *screen)
{
    // Create the windows in the opposite of the Z ordering
    if(create_notes_window(screen) != NULL)
        //if(create_inventory_window(screen) != NULL)
            //if(create_magic_window(screen) != NULL)
                //if(create_combat_window(screen) != NULL)
                    //if(create_proficiencies_window(screen) != NULL)
                        //if(create_character_window(screen) != NULL)
                            return(0);
    return(-1);
}

int app_quit(Window *win, int ch)
{
    switch(ch)
    {
        case 'Y':
        case 'y':
            return(-1);
        case 'N':
        case 'n':
        case KEY_ESC:
            destroy_window(win);
        case KEY_QUIT_PROGRAM:
            return(1);
    }
    return(0);
}

int app_input(Window *screen, int input)
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
            save_character(screen);
            break;
        case KEY_LOAD_CHARACTER: // Load (ctl-l)
            load_character(screen);
            break;
        case KEY_QUIT_PROGRAM:
            quit_win = yes_no_popup(screen, "Are you sure you want to quit? (Y/N)", app_quit);
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
int calculate_modifier(int score, int proficiency_modifier) 
{
    return ((score - 10) / 2) + proficiency_modifier;
}

void calculate_dependencies(Window *win)
{
    // Do this once here so this code is not spread all over the place and
    // duplicated in several places. Order is important.

    // Update ability modifiers
    character.abilities.strength.modifier = calculate_modifier(character.abilities.strength.score,0);
    character.abilities.dexterity.modifier = calculate_modifier(character.abilities.dexterity.score,0);
    character.abilities.constitution.modifier = calculate_modifier(character.abilities.constitution.score,0);
    character.abilities.intelligence.modifier = calculate_modifier(character.abilities.intelligence.score,0);
    character.abilities.wisdom.modifier = calculate_modifier(character.abilities.wisdom.score,0);
    character.abilities.charisma.modifier = calculate_modifier(character.abilities.charisma.score,0);
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
int input_ability(Component *component, int ch)
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

Window *create_character_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Character",true);
    win->frame_type = FRAME_HYBRID;

    // Display character information in sections
    Component  *this;
    int row = 1;

    Component *name = create_string(win,row++,2,20,"Name: ",character.name,MAX_TEXT_FIELD_LENGTH);
    create_string(win,row++,2,20,"Class: ",character.class,MAX_TEXT_FIELD_LENGTH);
    create_string(win,row++,2,20,"Race: ",character.race,MAX_TEXT_FIELD_LENGTH);
    create_string(win,row++,2,20,"Background: ",character.background,MAX_TEXT_FIELD_LENGTH);
    create_string(win,row++,2,20,"Alignment: ",character.alignment,MAX_TEXT_FIELD_LENGTH);
    row++;
    create_string(win,row++,2,20,"Sex: ",character.sex,MAX_TEXT_FIELD_LENGTH);
    create_integer(win,row++,2,20,"Age: ",&character.age);
    create_string(win,row++,2,20,"Height: ",character.height,MAX_TEXT_FIELD_LENGTH);
    create_integer(win,row++,2,20,"Weight: ",&character.weight);
    create_integer(win,row++,2,20,"Speed: ",&character.speed);
    row++;
    create_integer(win,row++,2,20,"XP: ",&character.xp);
    this = create_integer(win,row++,2,20,"Proficiency: ",&character.proficiency_bonus);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    this = create_integer(win,row++,2,20,"Level: ",&character.level);
    read_only_component(this);

    row++; // Add some spacing
    create_integer(win,row++,2,20,"Armor Class: ",&character.armor_class);
    this = create_integer(win,row++,2,20,"Initiative: ",&character.initiative);
    set_format_component(this,"%s%+d");
    read_only_component(this);
    create_integer(win,row,2,3,"HP: ",&character.hp_current);
    create_integer(win,row,9,3,"\\",&character.hp_max);
    this = create_integer(win,row++,14,3,"Temp ",&character.hp_temp);
    read_only_component(this);
    set_format_component(this,"%s(%d)");

    // Calculate the right column
    int right_column_start = (win->width / 2); // Add spacing between columns
    row = 1;

    // Display character abilities
    create_text(win,row++,right_column_start,"Ability  Mod Save Proficient");
    this = create_integer(win,row,right_column_start,2,"STR: ",&character.abilities.strength.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.strength.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,1,NULL, &character.abilities.strength.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.strength.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";
    
    this = create_integer(win,row,right_column_start,2,"DEX: ",&character.abilities.dexterity.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.dexterity.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,1,NULL, &character.abilities.dexterity.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.dexterity.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";

    this = create_integer(win,row,right_column_start,2,"CON: ",&character.abilities.constitution.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.constitution.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,1,NULL, &character.abilities.constitution.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.constitution.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";

    this = create_integer(win,row,right_column_start,2,"INT: ",&character.abilities.intelligence.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.intelligence.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,2,NULL, &character.abilities.intelligence.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.intelligence.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";

    this = create_integer(win,row,right_column_start,2,"WIS: ",&character.abilities.wisdom.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.wisdom.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,1,NULL, &character.abilities.wisdom.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.wisdom.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";

    this = create_integer(win,row,right_column_start,2,"CHA: ",&character.abilities.charisma.score);
    this->notify_input = input_ability;
    this = create_integer(win,row,right_column_start+7,2,NULL, &character.abilities.charisma.modifier);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_integer(win,row,right_column_start+12,1,NULL, &character.abilities.charisma.save);
    read_only_component(this);
    set_format_component(this,"%s (%+d)");
    this = create_checkbox(win,row++,right_column_start+21,1,NULL, &character.abilities.charisma.proficient);
    read_only_component(this);
    ((Checkbox *)this)->true_string = "P";

    ++row; // Add some spacing
    // Passive Skills
    create_text(win,row++,right_column_start,"Passive Skills ---");
    this = create_integer(win,row++,right_column_start,2,"Insight:       ",&character.passive_insight);
    read_only_component(this);
    this = create_integer(win,row++,right_column_start,2,"Investigation: ",&character.passive_investigation);
    read_only_component(this);
    this = create_integer(win,row++,right_column_start,2,"Perception:    ",&character.passive_perception);
    read_only_component(this);
    this = create_integer(win,row++,right_column_start,2,"Stealth:       ",&character.passive_stealth);
    read_only_component(this);

    ++row; // Add some spacing
    create_list(win,row,right_column_start,3,20,"Languages ---",(char *)character.languages,MAX_LANUAGES,MAX_LANGUAGE_DESCRIPTION);

    set_focus_window(win, name);

    return(win);
}

Window *create_proficiencies_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Proficiencies",true);
    win->frame_type = FRAME_HYBRID;

    Component *this;
    Component *focus;
    int row = 1;

    create_text(win,row++,2,"Skills----------------");
    this = create_integer(win,row,2,1,"Acrobatics      ", &character.skills.acrobatics.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    focus = create_checkbox(win,row++,21,1,NULL, &character.skills.acrobatics.proficient);

    this = create_integer(win,row,2,1,"Animal Handling ", &character.skills.animal_handling.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.animal_handling.proficient);

    this = create_integer(win,row,2,1,"Arcana          ", &character.skills.arcana.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.arcana.proficient);

    this = create_integer(win,row,2,1,"Athletics       ", &character.skills.athletics.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.athletics.proficient);

    this = create_integer(win,row,2,1,"Deception       ", &character.skills.deception.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.deception.proficient);

    this = create_integer(win,row,2,1,"History         ", &character.skills.history.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.history.proficient);

    this = create_integer(win,row,2,1,"Insight         ", &character.skills.insight.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.insight.proficient);

    this = create_integer(win,row,2,1,"Intimidation    ", &character.skills.intimidation.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.intimidation.proficient);

    this = create_integer(win,row,2,1,"Investigation   ", &character.skills.investigation.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.investigation.proficient);

    this = create_integer(win,row,2,1,"Medicine        ", &character.skills.medicine.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.medicine.proficient);

    this = create_integer(win,row,2,1,"Nature          ", &character.skills.nature.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.nature.proficient);

    this = create_integer(win,row,2,1,"Perception      ", &character.skills.perception.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.perception.proficient);

    this = create_integer(win,row,2,1,"Performance     ", &character.skills.performance.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.performance.proficient);

    this = create_integer(win,row,2,1,"Persuasion      ", &character.skills.persuasion.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.persuasion.proficient);

    this = create_integer(win,row,2,1,"Religion        ", &character.skills.religion.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.religion.proficient);

    this = create_integer(win,row,2,1,"Sleight of Hand ", &character.skills.sleight_of_hand.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.sleight_of_hand.proficient);

    this = create_integer(win,row,2,1,"Stealth         ", &character.skills.stealth.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.stealth.proficient);

    this = create_integer(win,row,2,1,"Survival        ", &character.skills.survival.modifier);
    read_only_component(this);
    set_format_component(this,"%s%+d");
    create_checkbox(win,row++,21,1,NULL, &character.skills.survival.proficient);

    // Calculate the right column
    int right_column_start = (win->width / 2) + 2; // Add spacing between columns

    create_list(win,1,right_column_start,5,40, "Tools---", (char *)character.inventory.tools, MAX_TOOLS, MAX_TOOL_DESCRIPTION);
    create_list(win,7,right_column_start,5,40, "Armor---", (char *)character.inventory.armor, MAX_ARMOR, MAX_ARMOR_DESCRIPTION);
    create_list(win,13,right_column_start,5,40, "Weapons--", (char *)character.inventory.weapons, MAX_WEAPONS, MAX_WEAPON_DESCRIPTION);

    // Set the focus for the window
    set_focus_window(win, focus);

    return(win);
}

Window *create_combat_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Combat",true);
    win->frame_type = FRAME_HYBRID;
    return(win);

    int row = 1;

    create_string(win,row++,1,20,"Weapon: ",character.attack[0].name,MAX_TEXT_FIELD_LENGTH);
    create_checkbox(win,row,1,1,"Str: ",&character.attack[0].strength);
    create_checkbox(win,row++,10,1,"Range: ",&character.attack[0].range);
    create_checkbox(win,row,1,1,"Reach: ",&character.attack[0].reach);
    create_checkbox(win,row++,12,1,"Prof: ",&character.attack[0].proficient);

}

Window *create_magic_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Magic",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *create_inventory_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Inventory",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *create_notes_window(Window *screen)
{
    Window *win = create_window(screen,0,0,screen->height,screen->width,"Notes",true);
    win->frame_type = FRAME_HYBRID;

    create_text_editor(win, 1, 1, win->height-2, win->width-2, character.notes, MAX_NOTES_LENGTH);

    return(win);
}

// File I/O Functions ---------------------------------------------------------
void save_character_action(Window *screen)
{
    char filename[MAX_TEXT_FIELD_LENGTH + 12]; // +12 for ".charsheet" and null terminator
    snprintf(filename, sizeof(filename), "%s.charsheet", character.name);

    FILE *file = fopen(filename, "wb"); // "wb" for writing binary data
    // If could not open file to write...
    if (file == NULL)
        display_error_popup(screen, "Could not open file!", MESSAGE_DURATION);
    // Else opened file to write...
    else
    {
        fwrite(&character, sizeof(Character), 1, file); // Write the entire character struct
        fclose(file);
        // Display success message (clear after a short delay)
        display_message_popup(screen,"Character saved!", MESSAGE_DURATION);
    }
}

int save_character_input(Window *win, int ch)
{
    // Assume the input was consumed.
    int ret = 1;

    // If positive acknowledge...
    if(ch == 'Y' || ch == 'y')
    {
        save_character_action(win->screen);
        destroy_window(win);
    }
    // Else if negative acknowledge...
    else if(ch == 'N' || ch == 'n' || ch == KEY_ESC)
        destroy_window(win);
    // Else if the user didn't tey to save again...
    else if(ch != KEY_SAVE_CHARACTER)
        ret = 0;

    return(ret);
}
    
void save_character(Window *screen) 
{
    if(strcmp(character.name, ""))
    {
        char filename[MAX_TEXT_FIELD_LENGTH + 12]; // +12 for ".charsheet" and null terminator
        snprintf(filename, sizeof(filename), "%s.charsheet", character.name);
        // If the file already exists...
        if(access(filename, F_OK) == 0)
            yes_no_popup(screen,"Character Exists, Overwrite? (Y/N)",save_character_input);
        // If the file doesn't exist...
        else
            save_character_action(screen);
    }
    else
        display_error_popup(screen,"Please enter a character name before saving",MESSAGE_DURATION);
}

int load_character_action(Component *base, int ch)
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
             display_error_popup(base->parent->screen,"Character not found!", MESSAGE_DURATION);
        else 
        {
            fread(&character, sizeof(Character), 1, file);
            fclose(file);
            display_message_popup(base->parent->screen,"Character loaded!", MESSAGE_DURATION);
            destroy_window(base->parent);
        }
    }
    return(ch);
}

void load_character(Window *screen) 
{
    static char filename[MAX_TEXT_FIELD_LENGTH];
    strcpy(filename,"");
    
    get_string_popup(screen, "Filename: ", filename, 20, load_character_action);    
}

// Function to remove leading/trailing whitespace from a string
char *trim(char *str) {
    char *end;

    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }

    // Write new null terminator character
    *(end + 1) = '\0';

    return str;
}

// Function to read a CSV field, one character at a time
char *read_csv_field(FILE *file) {
    char *field = NULL;
    int field_len = 0;
    char c;
    bool in_quotes = false;

    while ((c = fgetc(file)) != EOF)
    {
        // If the character is a quote...
        if (c == '"')
        {
            // If the next character is not a quote...
            if((c = fgetc(file)) != '"')
            {
                // Toggle in quotes status, put the character back, and continue
                in_quotes = !in_quotes;
                ungetc(c, file);
                continue;
            }
        }

        // If delimiter, then field is complete
        if (c == ',' && !in_quotes)
            break;

        // Allocate another byte in field
        field = realloc(field, field_len + 1);
        if (field == NULL)
            return NULL;
        field[field_len++] = c;
    }

    // If reached the end of the file...
    if(c == EOF)
    {
        // Free field and return a NULL
        free(field);
        return NULL;
    }

    // Allocate another byte in field for the NULL terminator
    field = realloc(field, field_len + 1);
    if (field == NULL)
        return NULL;

    field[field_len] = '\0';
    
    return field;
}

// Function to parse a spell from a CSV line
Spell *parse_spell_from_csv(FILE *file)
{
    char *field = NULL;
    // Allocate and initialize a new spell
    Spell *spell = malloc(sizeof(Spell));
    if(spell == NULL)
        return NULL;
    memset(spell, 0, sizeof(Spell));

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->name, trim(field), sizeof(spell->name) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->level = atoi(trim(field));
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->school, trim(field), sizeof(spell->school) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->casting_time, trim(field), sizeof(spell->casting_time) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->duration, trim(field), sizeof(spell->duration) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->range, trim(field), sizeof(spell->range) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->area, trim(field), sizeof(spell->area) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->attack, trim(field), sizeof(spell->attack) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->save, trim(field), sizeof(spell->save) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->damage_type, trim(field), sizeof(spell->damage_type) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->ritual = (strcmp(trim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->concentration = (strcmp(trim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->verbal = (strcmp(trim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->somatic = (strcmp(trim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    spell->component = (strcmp(trim(field), "Y") == 0);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->material, trim(field), sizeof(spell->material) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->source, trim(field), sizeof(spell->source) - 1);
    free(field);
    field = NULL;

    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    strncpy(spell->details, trim(field), sizeof(spell->details) - 1);
    free(field);
    field = NULL;

    // Burn this last field, not using the link
    if((field = read_csv_field(file)) == NULL)
        goto End_Of_File;
    free(field);
    field = NULL;

    return spell;

End_Of_File:
    free(spell);
    return NULL;
}

// Function to load spells from the CSV file into an array
Spell **load_spells(const char *filename, int *spell_count)
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
        spells_array[i] = parse_spell_from_csv(file);
    }while(spells_array[i++]!=NULL);
    *spell_count = i;

    fclose(file);
    return spells_array; // Return the number of spells loaded
}
