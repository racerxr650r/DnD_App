// Header Sentry
#ifndef CharSheet_h
#define CharSheet_h

#include <locale.h> // For setlocale
#include <ncurses.h>
#include <panel.h> // For panels
#include <wchar.h>
#include <unistd.h>

// Constants ******************************************************************
// If you change these, prior character files will be incompatible
#define MAX_TEXT_FIELD_LENGTH 41
#define MAX_SKILLS 18  // Number of skills

// Keys
#define KEY_SAVE_CHARACTER '\023'
#define KEY_LOAD_CHARACTER '\014'
#define KEY_TOGGLE         '\024'
#define KEY_QUIT_PROGRAM   '\021'

#define MESSAGE_DURATION 1500

#define LIST_MAX_SIZE 200

// Unicode box-drawing characters
wchar_t frame_symbols[8][10] = {
    { L'┌', L'─', L'┐', L'│', L'┘', L'└', L'┤', L'├', L'┴', L'┬' },
    { L'╭', L'─', L'╮', L'│', L'╯', L'╰', L'┤', L'├', L'┴', L'┬' },
    { L'┏', L'━', L'┓', L'┃', L'┛', L'┗', L'┫', L'┣', L'┻', L'┳' },
    { L'╔', L'═', L'╗', L'║', L'╝', L'╚', L'╡', L'╞', L'╨', L'╥' },
    { L'╒', L'═', L'╕', L'│', L'╛', L'╘', L'╡', L'╞', L'┴', L'┬' },
    { L'┌', L'╌', L'┐', L'╎', L'┘', L'└', L'┤', L'├', L'┴', L'┬' },
    { L'╭', L'╌', L'╮', L'╎', L'╯', L'╰', L'┤', L'├', L'┴', L'┬' },
    { L'┏', L'╍', L'┓', L'╏', L'┛', L'┗', L'┫', L'┣', L'┻', L'┳' }
};

// Macros *********************************************************************
#define top_window      next
#define bottom_window   prev
#define KEY_ESC         27

// Data Types *****************************************************************
typedef int (*Create_App_Windows)(void);
typedef int (*App_Input)(int input);

struct window_t;
struct component_t;

typedef int (*Initialize_Window)(struct window_t *win);
typedef int (*Configure_Window)(struct window_t *win);
typedef void (*Set_Cursor_Window)(struct window_t *win, bool enable);
typedef void (*Frame_Window)(struct window_t *win);
typedef void (*Update_Window)(struct window_t *win);
typedef int  (*Input_Window)(struct window_t *win, int input);
typedef void (*Destroy_Window)(struct window_t *win);
typedef int (*Add_Window)(struct window_t *screen, struct window_t *win);
typedef void (*Insert_Window)(struct window_t *ref, struct window_t *win);
typedef void (*Remove_Window)(struct window_t *win);
typedef void (*Stale_Window)(struct window_t *win);
typedef void (*Set_Focus_Window)(struct window_t *win, struct component_t *component);
typedef void (*Next_Focus_Window)(struct window_t *win);
typedef void (*Prev_Focus_Window)(struct window_t *win);
typedef void (*Move_Top_Window)(struct window_t *win);
typedef void (*Move_Bottom_Window)(struct window_t *win);
typedef void (*Move_Up_Window)(struct window_t *win);
typedef void (*Move_Down_Window)(struct window_t *win);
typedef int (*Print_Window)(struct window_t *win, int row, int col, const char *format, va_list args);
typedef int (*Write_Window)(struct window_t *win, struct window_t *this);
typedef void (*Notify_Window)(struct window_t *win);

typedef void (*Update_Component)(struct component_t *component);
typedef int  (*Input_Component)(struct component_t *component,int input);
typedef void (*Focus_Component)(struct component_t *component);
typedef void (*Destroy_Component)(struct component_t *component);
typedef void (*Add_Component)(struct window_t *win, struct component_t *component);
typedef void (*Remove_Component)(struct window_t *win, struct component_t *component);
typedef void (*Set_Focus_Component)(struct component_t *component);
typedef void (*Readonly_Component)(struct component_t *component);
typedef void (*Set_Format_Component)(struct component_t *component, const char *format);
typedef void (*Notify_Component)(struct component_t *component);

// UI Window
typedef enum
{
    FRAME_NONE = -1,
    FRAME_LIGHT = 0,
    FRAME_LIGHT_ARC = 1,
    FRAME_HEAVY = 2,
    FRAME_DOUBLE = 3,
    FRAME_HYBRID = 4,
    FRAME_DASH_LIGHT = 5,
    FRAME_DASH_LIGHT_ARC = 6,
    FRAME_DASH_HEAVY = 7
}Window_Frame;

typedef enum
{
    TOP_LEFT = 0,
    HORIZONTAL,
    TOP_RIGHT,
    VERTICAL,
    BOTTOM_RIGHT,
    BOTTOM_LEFT,
    LEFT_BREAK,
    RIGHT_BREAK,
    TOP_BREAK,
    BOTTOM_BREAK
}Frame_Symbol;

typedef enum
{
    CURSOR_NONE,
    CURSOR_INSERT,
    CURSOR_OVERWRITE
}Cursor_Type;

typedef struct window_t
{
    wchar_t *buffer;
    char    *label;
    int     row;
    int     col;
    int     height;
    int     width;

    int         cur_row;
    int         cur_col;
    Cursor_Type cursor_type;

    bool            insert_key;
    bool            wrap;
    bool            stale;
    Window_Frame    frame_type;

    struct component_t *component_head;
    struct component_t *component_tail;
    struct component_t *focus;

    struct window_t *screen;
    struct window_t *prev;
    struct window_t *next;

    Initialize_Window   initialize;
    Configure_Window    configure;
    Frame_Window        frame;
    Update_Window       update;
    Input_Window        input;
    Destroy_Window      destroy;
    Add_Window          add;
    Insert_Window       insert;
    Remove_Window       remove;
    Stale_Window        set_stale;

    Set_Focus_Window    set_focus;
    Set_Cursor_Window   set_cursor;
    Next_Focus_Window   next_focus;
    Prev_Focus_Window   prev_focus;
    Move_Top_Window     move_top;
    Move_Bottom_Window  move_bottom;
    Move_Up_Window      move_up;
    Move_Down_Window    move_down;
    Print_Window        print;
    Write_Window        write;

    Input_Window  notify_input;
    Input_Window  notify_action;
    Notify_Window notify_tick;
    Notify_Window notify_stale;
    Notify_Window notify_focus;
    Notify_Window notify_update;
    Notify_Window notify_destroy;
}Window;

Window *allocate_window(int row, int col, int height, int width, char *label, bool box);
Window *create_window(Window *screen, int row, int col, int height, int width, char *label, bool box);

static inline void frame_window(Window *this)
{
    if(this != NULL)
        if(this->frame != NULL)
            this->frame(this);
    return;
}

static inline void set_focus_window(Window *this, struct component_t *component)
{
    if(this != NULL)
        if(this->set_focus != NULL)
            this->set_focus(this, component);
    return;
}

static inline void set_cursor_window(Window *this, bool enable)
{
    if(this != NULL)
        if(this->set_cursor != NULL)
            this->set_cursor(this, enable);
    return;    
}

static inline void set_stale_window(Window *this)
{
    if(this != NULL)
        if(this->set_stale != NULL)
            this->set_stale(this);
    return;
}

static inline int print_window(Window *this, int row, int col, const char *format, ...)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    va_list args;
    va_start(args, format);

    int result = this->print(this, row, col, format, args);

    va_end(args);
    return(result);
}

static inline int write_window(Window *src, Window *this)
{
    if(src == NULL || this == NULL)
        return(-1);

    int result = this->write(src, this);
    return(result);
}

// UI Component
typedef struct component_t
{
    const char  *label;
    int         row;
    int         col;
    int         height;
    int         width;
    const char  *format;
    
    Window  *parent;
    struct component_t *prev;
    struct component_t *next;

    Update_Component    update;
    Input_Component     input;
    Focus_Component     focus;

    Add_Component       add;
    Remove_Component    remove;
    Destroy_Component   destroy;

    Readonly_Component  read_only;
    Set_Format_Component set_format;

    Input_Component  notify_input;
    Input_Component  notify_action;
    Notify_Component notify_tick;
    Notify_Component notify_focus;
    Notify_Component notify_destroy;
}Component;

Component *create_component(Component *component, Window *win, int row, int col, int height, int width, const char *label);

static inline void read_only_component(Component *this)
{
    if(this != NULL)
        if(this->read_only != NULL)
            this->read_only(this);
    return;
}

static inline void set_format_component(Component *this, const char *format)
{
    if(this != NULL)
        if(this->set_format != NULL)
            this->set_format(this, format);
    return;
}

static inline int input_component(Component *this, int input)
{
    if(this != NULL)
        if(this->input != NULL)
            return(this->input(this, input));
    return(0);
}

// If you change these, prior character files will be incompatible
struct list_t;
typedef void (*Display_List)(struct list_t *list);
typedef void (*Input_List)(struct list_t *list, int input);
typedef void (*Destroy_List)(struct list_t *list);
typedef struct list_t
{
    Component   base;   
    char        *items;
    int         size;
    int         size_max;
    int         selected;
    int         top_visible;
}List;
Component *create_list(Window *win, int row, int col, int height, int width, char *label, char *items, int size_max);

struct checkbox_t;
typedef void (*Display_Checkbox)(struct checkbox_t *checkbox);
typedef void (*Input_Checkbox)(struct checkbox_t *checkbox, int input);
typedef void (*Destroy_Checkbox)(struct checkbox_t *checkbox);
typedef struct checkbox_t
{
    Component   base;
    bool        *value;
    char        *true_string;
    char        *false_string;
}Checkbox;
Component *create_checkbox(Window *win, int row, int col, int height, char *label, bool *value);

struct string_t;
typedef void (*Display_String)(struct string_t *string);
typedef void (*Input_String)(struct string_t *string, int input);
typedef void (*Destroy_String)(struct string_t *string);
typedef struct string_t
{
    Component   base;
    char        *value;
    int         cursor_offset;
    int         size_max;
}String;
Component *create_string(Window *win, int row, int col, int width, char *label, char *value, int size_max);

struct integer_t;
typedef void (*Display_Integer)(struct integer_t *integer);
typedef void (*Input_Integer)(struct integer_t *integer, int input);
typedef void (*Destroy_Integer)(struct integer_t *integer);
typedef struct integer_t
{
    Component   base;
    int         *value;
    char        field[21];
    int         cursor_offset;
}Integer;
Component *create_integer(Window *win, int row, int col, int width, char *label, int *value);

Component *create_text(Window *win, int row, int col, const char *text);

struct timer_t;
typedef void (*Set_Timer)(struct timer_t *timer, unsigned int msecs);
typedef struct timer_t
{
    Component       base;
    unsigned int    counter;

    Set_Timer   set_timer;
}Timer;

Component *create_timer(Window *win, unsigned int msecs);
static inline void set_timer(Timer *timer, unsigned int msecs)
{
    if(timer != NULL)
        if(timer->set_timer != NULL)
            timer->set_timer(timer, msecs);
    return;
}


Window *create_screen();
int run_screen(Window *screen);
int input_screen(Window **this);

static inline void initialize_screen(Window *this)
{
    if(this != NULL)
        if(this->initialize != NULL)
            this->initialize(this);
}

static inline void destroy_screen(Window *this)
{
    if(this != NULL)
        if(this->destroy != NULL)
            this->destroy(this);
}

static inline void update_screen(Window *this)
{
    if(this != NULL)
        if(this->update != NULL)
            this->update(this);
}

static inline void write_screen(Window *src)
{
    if(src != NULL)
        if(src->write != NULL)
            src->write(NULL,src);
}

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
    char    armor[20][MAX_TEXT_FIELD_LENGTH];
    char    weapons[20][MAX_TEXT_FIELD_LENGTH];
    char    equipment[100][MAX_TEXT_FIELD_LENGTH];
    int     copper;
    int     silver;
    int     electrum;
    int     gold;
    int     platinum;
    char    treasure_items[200][MAX_TEXT_FIELD_LENGTH];
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

    // Proficiencies
    Skills      skills;
    char        tools[20][MAX_TEXT_FIELD_LENGTH];
    char        armor[20][MAX_TEXT_FIELD_LENGTH];
    char        weapons[20][MAX_TEXT_FIELD_LENGTH];

    // Combat
    int         hp_max;
    int         hp_current;
    int         hp_temp;
    int         hit_dice;
    int         exhaustion;
    int         death_saves;
    bool        inspiration;
    Attack      attack[5];

    // Magic
    Magic       magic;

    // Inventory
    Inventory   inventory;

    // Notes
    char        notes[65535];
} Character;

#endif //CarSheet_h