#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isdigit
#include "CharSheet.h"

// Internal Function Prototypes ***********************************************
void display_error_popup(const char *error_message, int milliseconds);
void display_message_popup(Window *win,const char *message, int milliseconds);

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
int main() 
{
    int ret;
    Spell **spells = NULL;
    int num_spells = 0;

    setlocale(LC_ALL, "");
    
    spells = load_spells("DnD_5e_Spells.csv", &num_spells);

    // Initialize character data structure
    memset(&character, 0, sizeof(Character));

    // Create all app windows and register global app input handler
    Window *screen = create_screen(create_app_windows, app_input);

    if(screen != NULL)
    {
        // Hook the update notification to calculate all dependencies
        screen->notification_update = calculate_dependencies;

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

int app_input(Window *screen, int input)
{
    // The following keys work in any window in the app
    switch(input)
    {
        case KEY_RIGHT:
            screen->top_window->move_bottom(screen->top_window);
            break;
        case KEY_LEFT:
            screen->bottom_window->move_top(screen->bottom_window);
            break;
        case KEY_SAVE_CHARACTER: // Save (ctl-s)
            save_character(screen);
            break;
        case KEY_LOAD_CHARACTER: // Load (ctl-l)
            load_character(screen);
            break;
        case KEY_QUIT_PROGRAM:
            return(-1);
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
    this->notification_input = input_ability;
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
    this->notification_input = input_ability;
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
    this->notification_input = input_ability;
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
    this->notification_input = input_ability;
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
    this->notification_input = input_ability;
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
    this->notification_input = input_ability;
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
    create_list(win,row,right_column_start,3,20,"Languages ---",(char *)character.languages,LIST_MAX_SIZE);

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

    create_list(win,1,right_column_start,5,40, "Tools---", (char *)character.tools, LIST_MAX_SIZE);
    create_list(win,7,right_column_start,5,40, "Armor---", (char *)character.armor, LIST_MAX_SIZE);
    create_list(win,13,right_column_start,5,40, "Weapons--", (char *)character.weapons, LIST_MAX_SIZE);

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
    if(this->notification_update)
        this->notification_update(this);

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

    // Get the current cursor location
    int row, col;
    getyx(stdscr,row,col);

    // Write the root window to the screen
    int count = mvaddwstr(0,0,src->buffer);
    // Mark the screen as fresh
    src->stale = false;

    // Restore the cursor location
    move(row,col);

    // Refresh the screen
    refresh();

    return(count);
}

void tick(Window *screen)
{
    if(screen == NULL)
        return;

    Window *win = screen->bottom_window;
    while(win != NULL)
    {
        if(win->notification_tick)
            win->notification_tick(win);

        Component *component = win->component_head;
        while(component)
        {
            if(component->notification_tick)
                component->notification_tick(component);
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
            Initialize_Window create_app_windows = screen->initialize;
            Input_Window app_input = screen->input;
            void *notification_input = screen->notification_input;
            void *notification_update = screen->notification_update;
            void *notification_destroy = screen->notification_destroy;
            void *notification_focus = screen->notification_focus;
            void *notification_stale = screen->notification_stale;
            void *notification_action = screen->notification_action;
            void *notification_tick = screen->notification_tick;

            // Tear it all down and rebuild for new screen size
            destroy_screen(screen);
            screen = create_screen(create_app_windows, app_input);
            screen->notification_action = notification_action;
            screen->notification_input = notification_input;
            screen->notification_update = notification_update;
            screen->notification_destroy = notification_destroy;
            screen->notification_focus = notification_focus;
            screen->notification_stale = notification_stale;
            screen->notification_tick = notification_tick;
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

Window *create_screen(Initialize_Window create_app_windows, Input_Window app_input)
{
    Window *screen = NULL;

    if(create_app_windows == NULL)
        return(NULL);

    // Initialize Ncurses
    initscr();

    // If allocating the screen buffer is successful...
    if((screen = allocate_window(0, 0, LINES, COLS, NULL, false)) != NULL)
    {
        screen->update = update_screen_method;
        screen->write = write_screen_method;
        screen->input = app_input;
        screen->initialize = create_app_windows;
        screen->destroy = destroy_screen_method;

        // If creating the application windows is successful...
        if(!screen->initialize(screen))
        {
            // Configure ncurses
            raw();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);
            start_color();
            // init_pair(1, COLOR_CYAN, COLOR_BLACK)
            return(screen);
        }
        else
            free(screen);
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

    if(this->label != NULL)
    {
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
        if(component->display != NULL)
            component->display(component);
        component = component->next;
    }

    // Set the focus component
    if(this->focus != NULL)
        if(this->focus->focus != NULL)
            this->focus->focus(this->focus);

    // If this window has an update notification...
    if(this->notification_update)
        this->notification_update(this);
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

    int min_col = 0;
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
            if(component->notification_input)
                // If the input notification consumed the key...
                if(result = component->notification_input(component,ch))
                    return(result);

            // If the input is consumed by the component...
            if(result = component->input(component,ch))
                return(result);
        }

    // If there is an input notification...
    if(this->notification_input)
        // If the input is consumed...
        if(result = this->notification_input(this,ch))
            return(result);

    // The window attempts to consume the input
    switch (ch)
    {
        case KEY_BTAB:
            this->prev_focus(this);
            break;
        case '\t':
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

    if(this->notification_focus)
        this->notification_focus(this);

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
        if(screen->top_window->notification_focus)
            screen->top_window->notification_focus(screen->top_window);
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
        if(this->notification_stale)
            this->notification_stale(this);

        this = this->next;
    }
}

void destroy_window(Window *this)
{
    if(this == NULL)
        return;

    // If this window has a destroy notification...
    if(this->notification_destroy)
        this->notification_destroy(this);
    
    // Mark all the windows as stale
    set_stale_window(this->screen->bottom_window);

    // Remove the window from the list
    this->remove(this);
    // Free the window buffer
    free(this->buffer);
    // Free the window data structure
    free(this);
}

void set_focus_window_method(Window *this, Component *component)
{
    if(this == NULL || component == NULL || component->focus == NULL)
        return;

    // Set the window focus
    this->focus = component;

    // Set the component focus
    component->focus(component);
    if(component->notification_focus)
        component->notification_focus(component);

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
    window->label = label;
    window->focus = NULL;
    window->wrap = false;
    window->stale = true;
    window->frame_type = FRAME_NONE;
    window->screen = NULL;

    window->initialize = NULL;
    window->frame = frame_window_method;
    window->update = update_window;
    window->write = write_window_method;
    window->input = input_window;
    window->destroy = destroy_window;

    window->add = add_window;
    window->insert = insert_window;
    window->remove = remove_window;
    window->set_stale = stale_window_method;
    window->set_focus = set_focus_window_method;
    window->next_focus = next_focus_window;
    window->prev_focus = prev_focus_window;
    window->move_top = move_top_window;
    window->move_bottom = move_bottom_window;
    window->move_up = move_up_window;
    window->move_down = move_bottom_window;
    window->print = print_window_method;

    window->notification_action = NULL;
    window->notification_input = NULL;
    window->notification_tick = NULL;
    window->notification_stale = NULL;
    window->notification_focus = NULL;
    window->notification_update = NULL;
    window->notification_destroy = NULL;

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
        win->component_head->prev = NULL;
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

Component *create_component(Component *component, Window *win, int row, int col, int height, int width, char *label)
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

    component->display = NULL;
    component->input = NULL;
    component->focus = NULL;
    
    component->add = add_component;
    component->remove = remove_component;
    component->destroy = destroy_component;
    
    component->read_only = read_only_component_method;
    component->set_format = set_format_component_method;

    component->notification_action = NULL;
    component->notification_input = NULL;
    component->notification_tick = NULL;
    component->notification_focus = NULL;
    component->notification_destroy = NULL;

    // Add the component to the window
    component->add(component->parent, component);

    return(component);
}

void display_list(Component *base)
{
    List *list = (List *)base;

    int cursor_x,cursor_y;
    getyx(stdscr,cursor_y,cursor_x);

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

    //base->parent->print(base->parent,row++,col,"%s---",base->label);
    print_window(base->parent,row++,col,"%s---",base->label);


    // Display the rows of items visible
    for(int i = list->top_visible; i < list->size && i < list->top_visible+base->height; i++)
        print_window(base->parent,row++,col,"%.*s", base->width, &list->items[i*list->size_max]);
        //base->parent->print(base->parent,row++,col,"%.*s", base->width, &list->items[i*list->size_max]);

    wmove(stdscr,cursor_y,cursor_x);
}

int input_list(Component *base, int ch)
{
    List *list = (List *)base;

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
            // Pass base-parent as the first parameter
            get_string_input(base->row+list->selected-list->top_visible+1, base->col, "Enter Item:", &list->items[list->selected*list->size_max], MAX_TEXT_FIELD_LENGTH);
            if(++list->selected == list->size && list->size < LIST_MAX_SIZE)
                strncpy(&list->items[list->size++ * list->size_max],"",MAX_TEXT_FIELD_LENGTH-1);
            else if(list->size == LIST_MAX_SIZE)
            {
                --list->selected;
                display_error_popup("Max List Items Reached",MESSAGE_DURATION);
            }
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

    wmove(stdscr,base->row+list->selected-list->top_visible+1,base->col);
}

Component *create_list(Window *win, int row, int col, int height, int width, char *label, char *items, int size_max)
{
    // Check input parameters
    if(win == NULL || items == NULL || size_max <= 0)
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
    list->size_max = size_max;
    list->items = items;
    list->selected = 0;
    list->top_visible = 0;

    // Determine how many items are in the list
    int i = 0;
    while(list->items[i++ * list->size_max] != 0);
    list->size = i;

    base->display = display_list;
    base->input = input_list;
    base->focus = focus_list;

    // Set the focus to this new component
    base->focus(base);

    return((Component*)list);
}

void display_checkbox(Component *base)
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
    wmove(stdscr,base->row,base->col+1);
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

    base->display = display_checkbox;
    base->input = input_checkbox;
    base->focus = focus_checkbox;

    // Set the focus to this new component
    base->focus(base);

    return(base);
}

void display_string(Component *base)
{
    String *str = (String *)base;

    print_window(base->parent,base->row,base->col,base->format,base->label,base->width,str->value);
}

int input_string(Component *base, int ch)
{
    String *str = (String *)base;

    switch(ch)
    {
        case '\n':
        case '\r':
            // Pass base-parent as the first parameter
            get_string_input(base->row, base->col, "Enter Value: ", str->value, MAX_TEXT_FIELD_LENGTH);
            break;
        default:
            // Did not consume the input
            return(0);
    }
    // Consumed the input
    set_stale_window(base->parent);
    return(1);
}

void focus_string(Component *base)
{
    wmove(stdscr,base->row,base->col+strlen(base->label));
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
    base->format = "%s%.*s";

    base->display = display_string;
    base->input = input_string;
    base->focus = focus_string;

    // Set the focus to this new component
    base->focus(base);

    return(base);
}

void display_integer(Component *base)
{
    Integer *integer = (Integer *)base;

    print_window(base->parent,base->row,base->col,base->format,base->label,*(integer->value));
}

int input_integer(Component *base, int ch)
{
    Integer *integer = (Integer *)base;

    switch(ch)
    {
        case '\n':
        case '\r':
            // Pass base-parent as the first parameter
            *integer->value = get_int_input(base->row, base->col, "Enter Value: ");
            break;
        default:
            // Did not consume the input
            return(0);
    }
    // Consumed the input
    set_stale_window(base->parent);
    return(1);
}

void focus_integer(Component *base)
{
    wmove(stdscr,base->row,base->col+strlen(base->label));
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
    base->format = "%s%d";

    base->display = display_integer;
    base->input = input_integer;
    base->focus = focus_integer;

    // Set the focus to this new component
    base->focus(base);

    return(base);
}

void display_text(Component *base)
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
    if((create_component(base, win, row, col, 1, strlen(text), (char *)text)) == NULL)
        return(NULL);

    base->display = display_text;
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
        if(base->notification_action)
            base->notification_action(base, 0); 
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

    base->notification_tick = tick_timer_method;
    timer->set_timer = set_timer_method;
    set_timer((Timer*)base,msecs);

    return(base);
}

void display_error_popup(const char *error_message, int milliseconds) 
{
    int width = strlen(error_message) + 9; // Add padding for the border
    int height = 3;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;

    WINDOW *error_win = newwin(height, width, start_y, start_x);
    box(error_win, 0, 0);
    mvwprintw(error_win, 1, 1, "Error: %s", error_message);
    wrefresh(error_win);

    PANEL *error_panel = new_panel(error_win); // Create a panel for the window
    update_panels();
    doupdate();
    curs_set(0); // Hide the cursor

    napms(milliseconds);  // Pause for 1 second

    // --- Remove the error popup ---
    del_panel(error_panel);
    delwin(error_win);
    curs_set(1); // Show the cursor
    update_panels();
    doupdate();
}

void popup_tick(Window *win)
{
    static int counter = 3000;

    if(--counter == 0)
    {
        counter = 3000;
        destroy_window(win);
    }
}

int popup_action_handler(Component *base, int ch)
{
    destroy_window(base->parent);
    return(0);
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
    base->notification_action = popup_action_handler;    
}

// Generic Input Fields--------------------------------------------------------
int get_int_input(int y, int x, const char *prompt) 
{
    WINDOW *win = stdscr;
    char buffer[20];  // Buffer to store the input string
    int value = 0;

    echo(); // Enable echoing of input (for numbers, it's helpful)
    mvwprintw(win, y, x, "%s ", prompt); // Display the prompt
    wgetnstr(win, buffer, sizeof(buffer) -1);  // Get input string (safe version)
    noecho(); // Disable echoing

    // Convert the string to an integer.  Error handling is crucial here!
    if (sscanf(buffer, "%d", &value) != 1) 
    {
        // Handle input error (e.g., non-numeric input)
        mvwprintw(win, LINES - 1, 0, "Invalid input. Please enter a number. Press any key...");
        wgetch(win); // Wait for a keypress
        mvwprintw(win, LINES - 1, 0, "%*s", COLS, ""); // Clear the error message line.
        wrefresh(win);
        return 0; // Or some other error value, like -1.  0 is often a valid score, though.
    }
    return value;
}

void get_string_input(int y, int x, const char *prompt, char *buffer, int max_length)
{
    WINDOW *win = stdscr;
    char temp_buffer[256]; // Use a temporary buffer, in case max_length is small

    echo();
    mvwprintw(win, y, x, "%s ", prompt);
    wgetnstr(win, temp_buffer, sizeof(temp_buffer) -1 ); // Get input, prevent buffer overflow
    noecho();

    // Copy the input to the provided buffer, truncating if necessary.
    strncpy(buffer, temp_buffer, max_length - 1);
    buffer[max_length - 1] = '\0'; // Ensure null termination!  Crucial.
}

// File I/O Functions ---------------------------------------------------------
void save_character(Window *screen) 
{
    if(strcmp(character.name, ""))
    {
        char filename[MAX_TEXT_FIELD_LENGTH + 12]; // +12 for ".charsheet" and null terminator
        snprintf(filename, sizeof(filename), "%s.charsheet", character.name);

        FILE *file = fopen(filename, "wb"); // "wb" for writing binary data
        if (file == NULL)
        {
            // Error handling: Could not open file
            display_error_popup("Could not open file!", MESSAGE_DURATION);
        }

        fwrite(&character, sizeof(Character), 1, file); // Write the entire character struct
        fclose(file);

        // Display success message (clear after a short delay)
        display_message_popup(screen,"Character saved!", MESSAGE_DURATION);
    }
    else
        display_error_popup("Please enter a name before saving",MESSAGE_DURATION);
}

void load_character(Window *screen) 
{
    char filename[MAX_TEXT_FIELD_LENGTH + 12];
    char input_name[MAX_TEXT_FIELD_LENGTH];

    // --- Create popup window ---
    WINDOW *popup_win = newwin(3, 60, (LINES - 5) / 2, (COLS - 60) / 2);
    box(popup_win, 0, 0);
    keypad(popup_win, TRUE);
    PANEL *popup_panel = new_panel(popup_win);
    update_panels();
    doupdate();

    mvwprintw(popup_win, 1, 2, "Enter character name to load: ");
    echo(); // Enable input echoing
    wgetnstr(popup_win, input_name, sizeof(input_name) - 1);
    noecho();

    snprintf(filename, sizeof(filename), "%s.charsheet", input_name);

    FILE *file = fopen(filename, "rb");
    // Error handling
    if (file == NULL) 
        display_error_popup("Character not found!", MESSAGE_DURATION);
    else 
    {
        fread(&character, sizeof(Character), 1, file);
        fclose(file);
        display_message_popup(screen,"Character loaded!", MESSAGE_DURATION);
    }

    // --- Remove popup ---
    del_panel(popup_panel);
    delwin(popup_win);
    update_panels();
    doupdate();
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
