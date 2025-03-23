// Compile: gcc -o DnD CharSheet.c -DNCURSES_WIDECHAR=1 -lncursesw -lpanel
#define __STDC_WANT_LIB_EXT2__ 1  //Define you want TR 24731-2:2010 extensions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isdigit
#include "CharSheet.h"

// Internal Function Prototypes ***********************************************
int calculate_modifier(int score, int proficiency_modifier);
void calculate_dependencies(Window *win);

//void get_input(WINDOW *win);
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
        if(create_inventory_window(screen) != NULL)
            if(create_magic_window(screen) != NULL)
                if(create_combat_window(screen) != NULL)
                    if(create_proficiencies_window(screen) != NULL)
                        if(create_character_window(screen) != NULL)
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
    Window *win = create_window(screen,0,0,LINES,COLS,"Character",true);
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
    Window *win = create_window(screen,0,0,LINES,COLS,"Proficiencies",true);
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
    Window *win = create_window(screen,0,0,LINES,COLS,"Combat",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *create_magic_window(Window *screen)
{
    Window *win = create_window(screen,0,0,LINES,COLS,"Magic",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *create_inventory_window(Window *screen)
{
    Window *win = create_window(screen,0,0,LINES,COLS,"Inventory",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

Window *create_notes_window(Window *screen)
{
    Window *win = create_window(screen,0,0,LINES,COLS,"Notes",true);
    win->frame_type = FRAME_HYBRID;
    return(win);
}

// Windows Frameworks UI ******************************************************
int initialize_screen_method(Window *this)
{
    // Configure ncurses
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color(); 
    
    return(0);
}

void destroy_screen_method(Window *this)
{
    // Turn off ncurses
    endwin();

    // Destroy all the child windows of this screen
    while(this->top_window)
        this->top_window->destroy(this->top_window);

    // Free the screen data structure
    free(this);
}

void update_screen_method(Window *this)
{
    Window *win = this->bottom_window;
    bool    stale = false;

    // If the screen has an update notification...
    if(this->notify_update)
        this->notify_update(this);

    while(win != NULL)
    {
        // If this window or a previous window is stale...
        if(win->stale || stale)
        {
            win->update(win);
            win->write(this,win);
            stale = true;
        }
        win = win->next;
    }
}

int write_screen_method(Window *dest, Window *src)
{
    if(src == NULL)
        return(0);

    // Write the root window to the screen
    int count = mvaddwstr(0,0,src->buffer);

    // Set the cursor type
    switch (src->cursor_type)
    {
        case CURSOR_NONE:
            curs_set(0); // Hide the cursor
            break;
        case CURSOR_INSERT:
            curs_set(1); // Show the cursor
            // Set to beem cursor
            if (is_term_resized(LINES, COLS) == FALSE)
            {
                printf("\033[5 q"); // Set to beem cursor
                fflush(stdout);
            }
            move(src->cur_row, src->cur_col);
            break;
        case CURSOR_OVERWRITE:
            curs_set(1); // Show the cursor
            // Set to block cursor
            if (is_term_resized(LINES, COLS) == FALSE)
            {
                printf("\033[1 q"); // Set to block cursor
                fflush(stdout);
            }
            move(src->cur_row, src->cur_col);
            break;
        default:
            curs_set(0); // Hide the cursor
            break;
    }
    // Set the cursor position
    move(src->cur_row,src->cur_col);

    // Refresh the screen
    refresh();
    // Mark the screen as fresh
    src->stale = false;

    return(count);
}

void tick(Window *screen)
{
    if(screen == NULL)
        return;

    Window *win = screen->bottom_window;
    while(win != NULL)
    {
        if(win->notify_tick)
            win->notify_tick(win);

        Component *component = win->component_head;
        while(component)
        {
            if(component->notify_tick)
                component->notify_tick(component);
            component = component->next;
        }
        win = win->next;
    }
}

int run_screen(Window *screen)
{
    if(screen == NULL)
        return(-1);

    int result;

    // If the screen has an initialize handler...
    if(screen->initialize != NULL)
        if(screen->initialize(screen) < 0)
            return(-1);

    // If the screen has a configure handler...
    if(screen->configure)
        if(screen->configure(screen) < 0)
            return(-1);
        
    do
    {
        Window *top;
        int ch;

        top = screen->top_window;
        // If there is no top window to receive input...
        if(top == NULL)
            return(-1);

        // Update the screen
        update_screen(screen);
        // Write the screen to hardware
        write_screen(screen);

        // Get the next key
        nodelay(stdscr,TRUE);
        while((ch = getch())==ERR)
        {
            // Delay for 1ms
            usleep(900);
            tick(screen);
            if(screen->stale)
            {
                // Update the screen
                update_screen(screen);
                // Write the screen to hardware
                write_screen(screen);
            }
        }
        nodelay(stdscr,FALSE);

        // If the screen has been resized...
        if(ch == KEY_RESIZE)
        {
            // Save the state of the user handlers
            Configure_Window configure = screen->configure;
            void *notify_input = screen->notify_input;
            void *notify_update = screen->notify_update;
            void *notify_destroy = screen->notify_destroy;
            void *notify_focus = screen->notify_focus;
            void *notify_stale = screen->notify_stale;
            void *notify_action = screen->notify_action;
            void *notify_tick = screen->notify_tick;

            // Tear it all down and rebuild for new screen size
            destroy_screen(screen);
            if((screen = create_screen()) < 0)
                return(-1);

            // Restore the state of the user handlers
            screen->configure = configure;
            screen->notify_action = notify_action;
            screen->notify_input = notify_input;
            screen->notify_update = notify_update;
            screen->notify_destroy = notify_destroy;
            screen->notify_focus = notify_focus;
            screen->notify_stale = notify_stale;
            screen->notify_tick = notify_tick;

            // If configure handler has been defined...
            if(screen->configure)
                if(screen->configure(screen) < 0)
                    return(-1);

            result = 1;
        }
        else
        {
            // If the window has an input handler...
            if(top->input)
                result = top->input(top,ch);
            // Else no key is consumed...
            else
                result = 0;

            // If the screen has an input handler...
            if(result == 0 && screen->input)
                result = screen->input(screen,ch);
        }
    } while (result >= 0);

    return(0);
}

Window *create_screen()
{
    Window *screen = NULL;

    // Initialize Ncurses
    initscr();

    // If allocating the screen buffer is successful...
    if((screen = allocate_window(0, 0, LINES, COLS, NULL, false)) != NULL)
    {
        screen->initialize = initialize_screen_method;
        screen->update = update_screen_method;
        screen->write = write_screen_method;
        screen->destroy = destroy_screen_method;

        return(screen);
    }

    endwin();
    return(NULL);
}

void frame_window_method(Window *this)
{
    if(this == NULL || this->frame_type == FRAME_NONE)
        return;

    int row = 0;
    int col = 0;

    // Top of the frame
    this->buffer[col] = frame_symbols[this->frame_type][TOP_LEFT];
    for(col = 1; col < this->width-1; col++)
        this->buffer[col] = frame_symbols[this->frame_type][HORIZONTAL];
    this->buffer[col] = frame_symbols[this->frame_type][TOP_RIGHT];

    // Sides of the frame
    for(row = 1; row < this->height-1; row++)
    {
        this->buffer[row * (this->width)] = frame_symbols[this->frame_type][VERTICAL];
        this->buffer[(row * (this->width))+this->width-1] = frame_symbols[this->frame_type][VERTICAL];
    }

    // Bottom of the frame
    col = 0;
    this->buffer[(row*this->width)+col] = frame_symbols[this->frame_type][BOTTOM_LEFT];
    for(col = 1; col < this->width-1; col++)
        this->buffer[(row*this->width)+col] = frame_symbols[this->frame_type][HORIZONTAL];
    this->buffer[(row*this->width)+col] = frame_symbols[this->frame_type][BOTTOM_RIGHT];
}    

void update_window(Window *this)
{
    if(this == NULL)
        return;

    // Clear the window
    for(int i=0;i<(this->height*this->width);i++)
        this->buffer[i] = L' ';

    frame_window(this);

    // If this window has a label...
    if(this->label != NULL)
    {
        // Display the window label in the top middle
        int offset = (this->width - strlen(this->label)) / 2;
        print_window(this, 0, offset, "%s", this->label);
        if(this->frame_type != FRAME_NONE)
        {
            this->buffer[offset-1] = frame_symbols[this->frame_type][LEFT_BREAK];
            this->buffer[offset+strlen(this->label)] = frame_symbols[this->frame_type][RIGHT_BREAK];
        }
    }

    // Walk the list of components and display each one
    Component *component = this->component_head;
    while(component)
    {
        if(component->update != NULL)
            component->update(component);
        component = component->next;
    }

    // If there is a component with focus...
    /*if(this->focus != NULL)
    {
        // Set the focus component
        if(this->focus->focus != NULL)
            this->focus->focus(this->focus);
    }
    // Else there is no component with focus...
    else if(this->set_cursor)
        this->set_cursor(this, false);*/

    // If this window has an update notification...
    if(this->notify_update)
        this->notify_update(this);
}

int write_window_method(Window *win, Window *this)
{
    if (win == NULL || this == NULL || win->buffer == NULL || this->buffer == NULL)
        return 0;

    int copy_count = 0;
    // Determine the copy area
    int min_row = 0;
    int max_row = this->height;
    if(max_row > win->height - this->row)
        max_row = win->height - this->row;

    int max_col = this->width;
    if(max_col > win->width - this->col)
        max_col = win->width - this->col;

    // Write the content of this window into the destination window.
    for(int row = min_row; row < max_row; row++)
    {
        int source_pos = row * this->width;
        int dest_pos = ((row + this->row) * win->width) + this->col;
        memcpy(&win->buffer[dest_pos], &this->buffer[source_pos], max_col * sizeof(wchar_t));
        copy_count += max_col;
    }

    // Update the cursor state and location
    win->cursor_type = this->cursor_type;
    win->cur_row = this->cur_row + this->row;
    win->cur_col = this->cur_col + this->col;    

    // Mark the source window fresh
    this->stale = false;
    // Mark the destination window stale
    set_stale_window(win);   

    return copy_count;
}

int input_window(Window *this, int ch)
{
    int result;

    if(this == NULL)
        return(0);

    // The window's component with focus attempts to consume input
    Component *component = this->focus;
    // If the window has a component with focus...
    if(component)
        // If the component has an input handler...
        if(component->input)
        {
            // If this component has an input notification...
            if(component->notify_input)
                // If the input notification consumed the key...
                if(result = component->notify_input(component,ch))
                    return(result);

            // If the input is consumed by the component...
            if(result = component->input(component,ch))
                return(result);
        }

    // If there is an input notification...
    if(this->notify_input)
        // If the input is consumed...
        if(result = this->notify_input(this,ch))
            return(result);

    // The window attempts to consume the input
    switch (ch)
    {
        case KEY_IC:
            this->insert_key ^= true;
            this->cursor_type = this->insert_key ? CURSOR_INSERT : CURSOR_OVERWRITE;
            break;
        case KEY_BTAB:
            this->prev_focus(this);
            break;
        case '\t':
        case KEY_ENTER:
            this->next_focus(this);
            break;
        default:
            // Did not consume the input
            return(0);
    }

    // Mark the window as stale
    set_stale_window(this);

    return(1);
}

int add_window(Window *screen, Window *this)
{
    if(this == NULL || screen == NULL)
        return(-1);

    Window *tail = screen->top_window;

    // If there is already a window in the list...
    if(tail != NULL)
    {
        tail->next = this;
        this->prev = tail;
        this->next = NULL;
    }
    // Else this is the first window to be added
    else
    {
        screen->bottom_window = this;
        this->next = NULL;
        this->prev = NULL;
    }

    screen->top_window = this;
    set_stale_window(this);
    this->screen = screen;

    if(this->notify_focus)
        this->notify_focus(this);

    return(0);
}

void insert_window(Window *ref, Window *this)
{
    if(this == NULL || ref == NULL)
        return;

    // If the reverence window is not the bottom window...
    if(ref->prev)
        ref->prev->next = this;
    // Else this is now the bottom window...
    else
        ref->screen->bottom_window = this;

    this->prev = ref->prev;
    this->next = ref;
    ref->prev = this;
    this->screen = ref->screen;
    set_stale_window(this);
}

void remove_window(Window *this)
{
    if(this == NULL || this->screen == NULL)
        return;

    Window *screen = this->screen;

    // If this is the bottom window...
    if(this == screen->bottom_window)
    {
        // If there is more windows in the list...
        if(this->next)
        {
            this->next->prev = NULL;
            screen->bottom_window = this->next;
        }
        // Else this is the last window in the list...
        else
        {
            screen->bottom_window = NULL;
            screen->top_window = NULL;
        }
    }
    // Else if this is the top window...
    else if(this == screen->top_window)
    {
        screen->top_window->prev->next = NULL;
        screen->top_window = screen->top_window->prev;

        // Notify the new top window
        if(screen->top_window->notify_focus)
            screen->top_window->notify_focus(screen->top_window);
    }
    // Else if this is a window in the middle of the list...
    else if(this->prev != NULL && this->next != NULL)
    {
        this->prev->next = this->next;
        this->next->prev = this->prev;
    }

    set_stale_window(screen->bottom_window);
}

void stale_window_method(Window *this)
{
    if(this == NULL)
        return;

    // Mark the associated screen stale
    if(this->screen)
        this->screen->stale = true;

    // Mark this window and all of them above it stale
    while(this)
    {
        this->stale = true;
        
        // If this window has a stale notification...
        if(this->notify_stale)
            this->notify_stale(this);

        this = this->next;
    }
}

void destroy_window_method(Window *this)
{
    if(this == NULL)
        return;

    // If this window has a destroy notification...
    if(this->notify_destroy)
        this->notify_destroy(this);
    
    // Destroy all of the components
    Component *component = this->component_head;
    while(component)
    {
        component->destroy(component);
        component = component->next;
    }

    // Mark all the windows as stale
    set_stale_window(this->screen->bottom_window);

    // Remove the window from the list
    this->remove(this);

    // Free the window buffer
    free(this->buffer);
    // Free the window data structure
    free(this);
}

void set_cursor_window_method(Window *this, bool enable)
{
    if(this == NULL)
        return;

    this->cursor_type = enable ? (this->insert_key ? CURSOR_INSERT : CURSOR_OVERWRITE) : CURSOR_NONE;
    set_stale_window(this);
}

void set_focus_window_method(Window *this, Component *component)
{
    if(this == NULL || component == NULL || component->focus == NULL)
        return;

    // Set the window focus
    this->focus = component;

    // Set the component focus
    if(component->focus)
        component->focus(component);
    if(component->notify_focus)
        component->notify_focus(component);

    // Mark the window to be redrawn
    set_stale_window(this);
}

void next_focus_window(Window *this)
{
    if(this == NULL)
        return;

    // If there are no components for this window...
    if(this->component_head == NULL || this->component_tail == NULL)
        return;

    // Find the next valid component
    Component *curr;
    // If there is a current focus...
    if(curr = this->focus)
    {
        // While there is a next component in the list...
        while(curr = curr->next)
        {
            // If this component can handle focus...
            if(curr->focus)
            {
                this->focus = curr;
                curr->focus(curr);
                set_stale_window(this);
                return;
            }
        }
    }

    curr = this->component_head;

    // Iterate through the list of components from head to tail
    do
    {
        // If component can handle focus...
        if(curr->focus)
        {
            this->focus = curr;
            curr->focus(curr);
            set_stale_window(this);
            return;
        }
    }while(curr = curr->next);

    // Else there are no valid components
    this->focus = NULL;
}

void prev_focus_window(Window *this)
{
    if(this == NULL)
        return;

    // If there are no components for this window...
    if(this->component_head == NULL || this->component_tail == NULL)
        return;

        // Find the previous valid component
    Component *curr;
    // If there is a current focus...
    if(curr = this->focus)
    {
        // While there is a previous component in the list...
        while(curr = curr->prev)
        {
            // If this component can handle focus...
            if(curr->focus)
            {
                this->focus = curr;
                curr->focus(curr);
                set_stale_window(this);
                return;
            }
        }
    }

    // Else start from the tail of the component list
    curr = this->component_tail;
    // Iterate through the list of components from tail to head
    do
    {
        // If this is not the current focus and the component can handle focus...
        if(curr != this->focus && curr->focus)
        {
            this->focus = curr;
            curr->focus(curr);
            set_stale_window(this);
            return;
        }
    }while(curr = curr->prev);

    // Else there are no valid components
    this->focus = NULL;
}

void move_top_window(Window *this)
{
    if(this == NULL || this == this->screen->top_window)
        return;

    this->remove(this);
    this->add(this->screen,this);
    set_stale_window(this);
}

void move_bottom_window(Window *this)
{
    if(this == NULL || this == this->screen->bottom_window)
        return;

    this->remove(this);
    this->insert(this->screen->bottom_window,this);
    set_stale_window(this);
}

void move_up_window(Window *this)
{
    if(this == NULL || this == this->screen->top_window)
        return;

    this->remove(this);
    // If the next window is not the top window...
    if(this->next->next)
        this->insert(this->next->next,this);
    // Else the next window is the top window...
    else
        this->add(this->screen,this);

    set_stale_window(this);
}

void move_down_window(Window *this)
{
    if(this == NULL || this == this->screen->bottom_window)
        return;

    this->remove(this);
    this->insert(this->prev,this);
    set_stale_window(this);
}

// Function to convert a single-byte char to a wide character
wchar_t char_to_wchar(char c) {
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t result = mbrtowc(&wc, &c, 1, &state);

    if (result == (size_t)-1 || result == (size_t)-2) {
        // Handle conversion error (e.g., invalid multibyte sequence)
        fprintf(stderr, "Error converting char to wchar_t\n");
        return L'\0'; // Return null wide character on error
    }

    return wc;
}

int print_window_method(Window *this, int row, int col, const char *format, va_list args)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    char *formatted_string = NULL;
    int formatted_string_size = 0;

    // Create a temporary formatted string
    int result = vasprintf(&formatted_string, format, args);
    if(result == -1 || formatted_string == NULL)
    {
        //va_end(args);
        return(0);
    }
    formatted_string_size = result;

    int total_written = 0;
    int current_row = row;
    int current_col = col;
    for (int i = 0; i < formatted_string_size; i++)
    {
        char c = formatted_string[i];

        if(c == '\n')
        {
            current_row++;
            current_col = col;
        }
        else if(c == '\r')
        {
            current_col = col;
        }
        else
        {
           // Calculate the position in the buffer. The buffer is a string of size width x height
            int pos = (current_row * this->width) + current_col;

            //Check if this position is valid.
            if(pos < 0 || pos >= (this->height * this->width))
            {
               continue; // Do not write if past the end of the buffer.
            }

            if(current_col >= this->width)
            {
              // If we have reached the end of a line, then we will not write it.
              // However, if we are wrapping, then we will handle this later.
              if(!this->wrap)
                continue;
            }

            // Write one character to the buffer
            this->buffer[pos] = char_to_wchar(c);
            current_col++;
            total_written++;
        }
    }

    // If the window was written to...
    if(total_written > 0)
        set_stale_window(this);

    //Free the formatted string
    free(formatted_string);
    return total_written;
}

Window *allocate_window(int row, int col, int height, int width, char *label, bool box)
{
    Window *window = malloc(sizeof(Window));
    if(window == NULL)
        return(NULL);

    // Set window configuration
    window->row = row;
    window->col = col;
    window->height = height;
    window->width = width;
    window->cur_row = 0;
    window->cur_col = 0;
    window->cursor_type = CURSOR_NONE;
    window->insert_key = true;
    window->label = label;
    window->focus = NULL;
    window->wrap = false;
    window->stale = true;
    window->frame_type = FRAME_NONE;
    window->screen = NULL;

    window->initialize = NULL;
    window->configure = NULL;
    window->frame = frame_window_method;
    window->update = update_window;
    window->write = write_window_method;
    window->input = input_window;
    window->destroy = destroy_window_method;

    window->add = add_window;
    window->insert = insert_window;
    window->remove = remove_window;
    window->set_stale = stale_window_method;
    window->set_cursor = set_cursor_window_method;
    window->set_focus = set_focus_window_method;
    window->next_focus = next_focus_window;
    window->prev_focus = prev_focus_window;
    window->move_top = move_top_window;
    window->move_bottom = move_bottom_window;
    window->move_up = move_up_window;
    window->move_down = move_bottom_window;
    window->print = print_window_method;

    window->notify_action = NULL;
    window->notify_input = NULL;
    window->notify_tick = NULL;
    window->notify_stale = NULL;
    window->notify_focus = NULL;
    window->notify_update = NULL;
    window->notify_destroy = NULL;

    window->next = NULL;
    window->prev = NULL;
    window->component_head = NULL;
    window->component_tail = NULL;

    // Create the ncurses window
    if(height != 0 || width != 0)
    {
        window->buffer = malloc(((height * width)+1)*sizeof(wchar_t));
        if(window->buffer == NULL)
        {
            free(window);
            return(NULL);
        }
        window->buffer[height * width] = L'\0';
    }
    else
        window->buffer = NULL;

    return(window);
}

Window *create_window(Window *screen, int row, int col, int height, int width, char *label, bool box)
{
    Window *window = allocate_window(row, col, height, width, label, box);

    // If the window was successfully allocated and configured...
    if(window != NULL)
        window->add(screen, window);

    return(window);
}

// Components -----------------------------------------------------------------

void add_component(Window *win, Component *component)
{
    if(win == NULL || component == NULL)
        return;

    if(win->component_head == NULL)
    {
        win->component_head = component;
        win->component_tail = component;
    }
    else
    {
        win->component_tail->next = component;
        component->prev = win->component_tail;
        win->component_tail = component;
    }
}

void remove_component(Window *win, Component *component)
{
    if(win == NULL || component == NULL || win->component_head == NULL || win->component_tail == NULL)
        return;

    // If this component has the focus...
    if(component->parent->focus == component)
        // Decrement the focus to the previous valid component
        component->parent->prev_focus(component->parent);

    // If the component is at the head of the list...
    if(win->component_head == component)
    {
        win->component_head = component->next;
        // If there is more than one component...
        if(win->component_head != NULL)
            win->component_head->prev = NULL;
        // Else this is the only component in the list...
        else
            win->component_tail = NULL;
    }
    // Else if the component is at the tail of the list...
    else if(win->component_tail == component)
    {
        win->component_tail = component->prev;
        win->component_tail->next = NULL;
    }
    // Else the component is in the middle of the list...
    else
    {
        component->prev->next = component->next;
        component->next->prev = component->prev;
    }
    component->next = NULL;
    component->prev = NULL;    
}

void destroy_component(Component *component)
{
    // Remove the component from the window
    component->remove(component->parent, component);
    free(component);
}

void read_only_component_method(Component *component)
{
    if(component == NULL || component->parent == NULL)
        return;

    component->parent->prev_focus(component->parent);
    component->focus = NULL;
}

void set_format_component_method(Component *component, const char *format)
{
    if(component == NULL)
        return;

    component->format = format;
}

Component *create_component(Component *component, Window *win, int row, int col, int height, int width, const char *label)
{
    if(component == NULL || win == NULL)
        return(NULL);

    if(label != NULL)
        component->label = label;
    else
        component->label = "";

    component->parent = win;
    component->row = row;
    component->col = col;
    component->height = height;
    component->width = width;
    component->format = NULL;

    component->prev = NULL;
    component->next = NULL;

    component->update = NULL;
    component->input = NULL;
    component->focus = NULL;
    
    component->add = add_component;
    component->remove = remove_component;
    component->destroy = destroy_component;
    
    component->read_only = read_only_component_method;
    component->set_format = set_format_component_method;

    component->notify_action = NULL;
    component->notify_input = NULL;
    component->notify_tick = NULL;
    component->notify_focus = NULL;
    component->notify_destroy = NULL;

    // Add the component to the window
    component->add(component->parent, component);

    return(component);
}

#define list_item_selected(list)    &list->items[list->selected*list->max_length]

void update_list(Component *base)
{
    List *list = (List *)base;

    // If the selected item below the currently displayed section of list...
    if(list->top_visible > list->selected)
        list->top_visible = list->selected;

    // If the selected item is above the currently displayed section of the list...
    if(list->selected > list->top_visible + base->height - 1)
        if((list->top_visible = list->selected - base->height + 1)<0)
            list->top_visible = 0;

    // Display the label
    int row = base->row;
    int col = base->col;
    print_window(base->parent,row++,col,"%s---",base->label);

    // Display the rows of items visible
    for(int i = list->top_visible; i < list->size && i < list->top_visible+base->height; i++)
        print_window(base->parent,row++,col,"%.*s", base->width, &list->items[i*list->max_length]);

    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the position of the cursor
        base->parent->cur_row = base->row + list->selected - list->top_visible + 1;
        base->parent->cur_col = base->col+strlen(selected_item_list(list));
    }
}

int list_action(Component *base, int ch)
{
    List *list = (List *)base->prev;

    // If the next selected item is at the end of the list and the list is not too big...
    if(++list->selected == list->size && list->size < list->max_items)
        // Add a blank item to the end of the list
        list->items[list->size++ * list->max_length] = '\0';
        //strncpy(&list->items[list->size++ * list->max_length],"",list->max_length-1);
    // Else if the size of the list is maxxed...
    else if(list->size == list->max_items)
    {
        --list->selected;
        display_error_popup(base->parent->screen, "Max List Items Reached",MESSAGE_DURATION);
    }
    destroy_window(base->parent);
    return(1);
}

int input_list(Component *base, int ch)
{
    List *list = (List *)base;
    Component *get_string;

    switch(ch)
    {
        case KEY_UP:
            if(--list->selected < 0)
                list->selected = 0;
            break;
        case KEY_DOWN:
            if(++list->selected > list->size - 1)
                list->selected = list->size - 1;
            break;
        case '\n':
        case '\r':
            get_string = get_string_popup(base->parent->screen, "Enter Item: ", &list->items[list->selected*list->max_length], list->max_length, list_action);
            if(get_string == NULL)
                return(-1);

            // Stash a pointer to this component context for the action handler
            get_string->prev = base;
            // Pass base-parent as the first parameter
            /*get_string_input(base->row+list->selected-list->top_visible+1, base->col, "Enter Item:", &list->items[list->selected*list->size_max], MAX_TEXT_FIELD_LENGTH);
            if(++list->selected == list->size && list->size < LIST_MAX_SIZE)
                strncpy(&list->items[list->size++ * list->size_max],"",MAX_TEXT_FIELD_LENGTH-1);
            else if(list->size == LIST_MAX_SIZE)
            {
                --list->selected;
                display_error_popup(base->parent->screen, "Max List Items Reached",MESSAGE_DURATION);
            }*/
            break;
        default:
            // Did not consume the input
            return(0);
    }
    // Consumed the input
    set_stale_window(base->parent);

    return(1);
}

void focus_list(Component *base)
{
    List *list = (List *)base;

    //wmove(stdscr,base->row+list->selected-list->top_visible+1,base->col);
    base->parent->cur_row = base->row+list->selected-list->top_visible+1;
    base->parent->cur_col = base->col;
    set_cursor_window(base->parent,true);
}

Component *create_list(Window *win, int row, int col, int height, int width, char *label, char *items, int max_items, int max_length)
{
    // Check input parameters
    if(win == NULL || items == NULL || max_items <= 0 || max_length <= 0)
        return(NULL);

    // Allocate the list
    List *list = malloc(sizeof(List));
    if(list == NULL)
        return(NULL);

    // Create the component
    if((create_component((Component *)list, win, row, col, height, width, label)) == NULL)
        return(NULL);
    Component *base = (Component *)list;
    
    // Set list configuration
    list->max_items = max_items;
    list->max_length = max_length;
    list->items = items;
    list->selected = 0;
    list->top_visible = 0;

    // Determine how many items are in the list
    int i = 0;
    while(list->items[i++ * list->max_length] != 0);
    list->size = i;

    base->update = update_list;
    base->input = input_list;
    base->focus = focus_list;

    // Set the focus to this new component
    set_focus_window(win,base);

    return((Component*)list);
}

void update_checkbox(Component *base)
{
    Checkbox *cb = (Checkbox *)base;

    if(cb->true_string != NULL && cb->false_string != NULL)
        print_window(base->parent,base->row,base->col,base->format,base->label,*(cb->value)?cb->true_string:cb->false_string);
}

int input_checkbox(Component *base, int ch)
{
    Checkbox *cb = (Checkbox *)base;

    switch(ch)
    {
        case ' ':
        case KEY_TOGGLE:
            *cb->value = !(*cb->value);
            break;
        default:
            // Did not consume the input
            return(0);
    }
    // Consumed the input
    set_stale_window(base->parent);
    return(1);
}

void focus_checkbox(Component *base)
{
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col+1;
    set_cursor_window(base->parent,true);
}

Component *create_checkbox(Window *win, int row, int col, int width, char *label, bool *value)
{
    // Check input parameters
    if(value == NULL)
        return(NULL);

    // Allocate the checkbox
    Checkbox *checkbox = malloc(sizeof(Checkbox));
    if(checkbox == NULL)
        return(NULL);

    // Create the component
    if((create_component((Component *)checkbox, win, row, col, 1, width, label)) == NULL)
        return(NULL);
    Component *base = (Component *)checkbox;

    checkbox->value = value;
    checkbox->true_string = "X";
    checkbox->false_string = " ";
    base->format = "%s[%s]";

    base->update = update_checkbox;
    base->input = input_checkbox;
    base->focus = focus_checkbox;

    // Set the focus to this new component
    set_focus_window(win,base);

    return(base);
}

void update_string(Component *base)
{
    String *str = (String *)base;

    // Draw the label and field
    print_window(base->parent,base->row,base->col,base->format,base->label,base->width,str->value);
    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the position of the cursor
        base->parent->cur_row = base->row;
        base->parent->cur_col = base->col+strlen(base->label)+str->cursor_offset;
    }
}

int input_string(Component *base, int ch)
{
    String *str = (String *)base;

    switch(ch)
    {
        case KEY_HOME:
            str->cursor_offset = 0;
            break;
        case KEY_END:
            str->cursor_offset = strlen(str->value);
            break;
        case KEY_LEFT:
            if(str->cursor_offset > 0)
                --str->cursor_offset;
            break;
        case KEY_RIGHT:
            if(str->cursor_offset < strlen(str->value))
                ++str->cursor_offset;
            break;
        case KEY_BACKSPACE:
        if (str->cursor_offset > 0)
        {
            memmove(&str->value[str->cursor_offset - 1], &str->value[str->cursor_offset], strlen(&str->value[str->cursor_offset]) + 1);
            --str->cursor_offset;
        }
        break;
        case KEY_DC:
            if(str->cursor_offset < strlen(str->value))
                memmove(&str->value[str->cursor_offset],&str->value[str->cursor_offset+1], strlen(&str->value[str->cursor_offset]));
            break;
        case '\n':
        case '\r':
        case KEY_ENTER:
        case KEY_ESC:
            if(base->notify_action != NULL)
                base->notify_action(base, ch);
            break;
        default:
            if(isprint(ch))
            {
                if(str->cursor_offset < str->size_max)
                {
                    if(base->parent->insert_key)
                        strcpy(&str->value[str->cursor_offset+1],&str->value[str->cursor_offset]);
                    str->value[str->cursor_offset++] = ch;
                }
            }
            else
            {
                // Did not consume the input
                return(0);
            }
    }
    // Consumed the input
    set_stale_window(base->parent);
    return(1);
}

void focus_string(Component *base)
{
    String *str = (String *)base;
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col+strlen(base->label)+str->cursor_offset;
    str->cursor_offset = strlen(str->value);
    set_cursor_window(base->parent,true);
}

Component *create_string(Window *win, int row, int col, int width, char *label, char *value, int size_max)
{
    // Check input parameters
    if(value == NULL || size_max <= 0)
        return(NULL);

    // Allocate the string
    String *string = malloc(sizeof(String));
    if(string == NULL)
        return(NULL);
    Component *base = (Component *)string;

    // Create the component
    if((create_component(base, win, row, col, 1, width, label)) == NULL)
        return(NULL);

    string->value = value;
    string->size_max = size_max;
    string->cursor_offset = strlen(value);
    base->format = "%s%.*s";

    base->update = update_string;
    base->input = input_string;
    base->focus = focus_string;

    // Set the focus to this new component
    set_focus_window(win,base);
    //base->focus(base);

    return(base);
}

void update_integer(Component *base)
{
    Integer *integer = (Integer *)base;

    print_window(base->parent,base->row,base->col,base->format,base->label,*(integer->value));
    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the cursor position
        base->parent->cur_row = base->row;
        base->parent->cur_col = base->col + strlen(base->label) + integer->cursor_offset;
    }
}

int input_integer(Component *base, int ch)
{
    Integer *integer = (Integer *)base;

    switch (ch)
    {
        case KEY_BACKSPACE:
        case KEY_DC:
            if (integer->cursor_offset > 0)
            {
                integer->field[--integer->cursor_offset] = '\0';
                *integer->value = atoi(integer->field);
            }
            break;
        case '0' ... '9':
            if (integer->cursor_offset < sizeof(integer->field) - 1)
            {
                integer->field[integer->cursor_offset++] = ch;
                integer->field[integer->cursor_offset] = '\0';
                *integer->value = atoi(integer->field);
                integer->cursor_offset = snprintf(integer->field,sizeof(integer->field)-1,"%d",*integer->value);
            }
            break;
        case '\n':
        case '\r':
        case KEY_ENTER:
            if(base->notify_action)
                base->notify_action(base, *integer->value);
            
            base->parent->next_focus(base->parent);
            break;
        default:
            // Did not consume the input
            return (0);
    }

    // Consumed the input
    set_stale_window(base->parent);
    return(1);
}

void focus_integer(Component *base)
{
    Integer *integer = (Integer *)base;
    // Update the cursor position
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col + strlen(base->label) + integer->cursor_offset;
    // Update the string representation and cursor location
    sprintf(integer->field,"%d",*integer->value);
    integer->cursor_offset = strlen(integer->field);
    // Enable the cursor
    set_cursor_window(base->parent,true);
}

Component *create_integer(Window *win, int row, int col, int width, char *label, int *value)
{
    // Check input parameters
    if(row < 0 || col < 0 || width <= 0 || value == NULL)
        return(NULL);

    // Allocate the integer
    Integer *integer = malloc(sizeof(Integer));
    if(integer == NULL)
        return(NULL);
    Component *base = (Component *)integer;

    // Create the component
    if((create_component(base, win, row, col, 1, width, label)) == NULL)
        return(NULL);

    integer->value = value;
    integer->cursor_offset = snprintf(integer->field,sizeof(integer->field)-1,"%d",*integer->value);
    base->format = "%s%d";

    base->update = update_integer;
    base->input = input_integer;
    base->focus = focus_integer;

    // Set the focus to this new component
    set_focus_window(win,base);

    return(base);
}

void update_text(Component *base)
{
    if(base == NULL)
        return;

    print_window(base->parent,base->row,base->col,base->label,NULL);
}

Component *create_text(Window *win, int row, int col, const char *text)
{
    // Check input parameters
    if(row < 0 || col < 0 || text == NULL)
        return(NULL);

    // Allocate the integer
    Component *base = malloc(sizeof(Component));
    if(base == NULL)
        return(NULL);

    // Create the component
    if((create_component(base, win, row, col, 1, strlen(text), text)) == NULL)
        return(NULL);

    base->update = update_text;
    return(base);
}

void set_timer_method(Timer *timer, unsigned int msecs)
{
    if(timer == NULL)
        return;
    timer->counter = msecs;
}

void tick_timer_method(Component *base)
{
    Timer *timer = (Timer *)base;

    if(--timer->counter == 0)
        if(base->notify_action)
            base->notify_action(base, 0); 
}

Component *create_timer(Window *win, unsigned int msecs)
{
    if(win == NULL)
        return(NULL);

    Timer *timer = malloc(sizeof(Timer));
    if(timer == NULL)
        return(NULL);

    Component *base = (Component *)timer;
    if((create_component(base, win, 0, 0, 0, 0, NULL)) == NULL)
        return(NULL);

    base->notify_tick = tick_timer_method;
    timer->set_timer = set_timer_method;
    set_timer((Timer*)base,msecs);

    return(base);
}

// Generic pop up windows------------------------------------------------------
int display_popup_action(Component *base, int ch)
{
    destroy_window(base->parent);
    return(0);
}
void display_error_popup(Window *screen, const char *error_message, int milliseconds) 
{
    int width = strlen(error_message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win = create_window(screen, start_y, start_x, height, width, "Error", false);
    win->frame_type = FRAME_LIGHT_ARC;
    create_text(win,1,1,error_message);
    Component *base = create_timer(win,milliseconds);
    base->notify_action = display_popup_action;    
}

void display_message_popup(Window *screen, const char *message, int milliseconds) 
{
    int width = strlen(message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win = create_window(screen, start_y, start_x, height, width, NULL, false);
    win->frame_type = FRAME_LIGHT_ARC;
    create_text(win,1,1,message);
    Component *base = create_timer(win,milliseconds);
    base->notify_action = display_popup_action;    
}

Window *yes_no_popup(Window *screen, const char *message, Input_Window yes_no_input)
{
    int width = strlen(message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win;
    Component *base;
    if(win = create_window(screen, start_y, start_x, height, width, NULL, false))
    {
        win->frame_type = FRAME_LIGHT_ARC;
        if(base = create_text(win,1,1,message))
            win->input = yes_no_input;
        else
        {
            destroy_window(win);
            win = NULL;
        }
    }
    return(win);
}

Component *get_string_popup(Window *screen, char * label, char *value, int length, Input_Component handler)
{
    // --- Create popup window ---
    int width = strlen(label) + length + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win = create_window(screen, start_y, start_x, height, width, NULL, false);
    if(win == NULL)
        return(NULL);
    win->frame_type = FRAME_LIGHT_ARC;

    Component *base = create_string(win,1,1,20,label,value,length);
    if(base == NULL)
        return(NULL);
    base->notify_action = handler;

    return(base);
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
