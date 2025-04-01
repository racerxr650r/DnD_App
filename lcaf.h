/*
 * lcaf.h
 *
 * Main header file for the Linux Console Application Framework (lcaf)
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
#ifndef LCAF_H
#define LCAF_H

#define __STDC_WANT_LIB_EXT2__ 1  //Define you want TR 24731-2:2010 extensions

#include <locale.h> // For setlocale
#include <ncurses.h>
#include <wchar.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wctype.h>
#include <string.h>

// Constants ******************************************************************
#ifndef MESSAGE_DURATION
#define MESSAGE_DURATION    1500
#endif

#ifndef KEY_TOGGLE
#define KEY_TOGGLE      '\024'
#endif

// Macros *********************************************************************
#define top_window      next
#define bottom_window   prev
#define KEY_ESC         27

// Text Window ****************************************************************
// Data/Function Types --------------------------------------------------------
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
typedef void (*Clear_Window)(struct window_t *win);
typedef int (*Print_Window)(struct window_t *win, int row, int col, const char *format, va_list args);
typedef int (*Wprint_Window)(struct window_t *win, int row, int col, const wchar_t *format, va_list args);
typedef int (*Write_Window)(struct window_t *win, struct window_t *this);
typedef void (*Notify_Window)(struct window_t *win);

typedef enum
{
    APP_UNABLE_TO_CREATE_WINDOW,
    APP_UNABLE_TO_CREATE_COMPONENT,
    APP_MEMORY_ALLOC_FAIL,
    APP_INVALID_PARAMETER,
    APP_OK = 0
}App_Status;

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

    void *context;
    size_t context_size;

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
    Clear_Window        clear_window;
    Print_Window        print;
    Wprint_Window       wprint;
    Write_Window        write;

    Input_Window  notify_input;
    Input_Window  notify_action;
    Notify_Window notify_tick;
    Notify_Window notify_stale;
    Notify_Window notify_focus;
    Notify_Window notify_update;
    Notify_Window notify_destroy;
}Window;

// Function Prototypes/Inlines ------------------------------------------------
Window *allocate_window(int row, int col, int height, int width, char *label, bool box);
Window *create_window(Window *screen, int row, int col, int height, int width, char *label, bool box);

wchar_t char_to_wchar(char c);

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

static inline void clear_window(Window *this)
{
    if(this != NULL)
        if(this->clear_window != NULL)
            this->clear_window(this);
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

static inline int wprint_window(Window *this, int row, int col, const wchar_t *format, ...)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    va_list args;
    va_start(args, format);

    int result = this->wprint(this, row, col, format, args);

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

static inline void destroy_window(Window *this)
{
    if(this != NULL)
        if(this->destroy != NULL)
            this->destroy(this);
    return;
}

// UI Component ***************************************************************
// Data/Function Types --------------------------------------------------------
typedef void (*Configure_Component)(struct component_t *component);
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

typedef struct component_t
{
    const char  *label;
    int         row;
    int         col;
    int         height;
    int         width;
    const char  *format;
    
    Window  *parent;
    Window  *window;
    
    struct component_t *prev;
    struct component_t *next;

    Configure_Component configure;
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

// Function Prototypes/Inlines ------------------------------------------------
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

static inline void configure_coponent(Component *this)
{
    if(this != NULL)
        if(this->configure != NULL)
            this->configure(this);
    return;
}

static inline void destroy_component(Component *this)
{
    if(this != NULL)
        if(this->destroy != NULL)
            this->destroy(this);
    return;
}

static inline void notify_destroy_component(Component *this)
{
    if(this != NULL)
        if(this->notify_destroy != NULL)
            this->notify_destroy(this);
    return;
}

// List Component *************************************************************
// Data/Function Types --------------------------------------------------------
struct list_t;
typedef void (*Display_List)(struct list_t *list);
typedef void (*Input_List)(struct list_t *list, int input);
typedef void (*Destroy_List)(struct list_t *list);
typedef struct list_t
{
    Component   base;   
    char        *items;
    int         size;
    int         max_items;
    int         max_length;
    int         selected;
    int         top_visible;
}List;

// Function Prototypes/Inlines ------------------------------------------------
Component *create_list(Window *win, int row, int col, int height, int width, char *label, char *items, int max_items, int max_length);

static inline char *selected_item_list(List *list)
{
    if(list == NULL)
        return(NULL);
    
    return(&list->items[list->selected*list->max_length]);
}

// Checkbox Component *********************************************************
// Data/Function Types --------------------------------------------------------
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

// Function Prototypes/Inlines ------------------------------------------------
Component *create_checkbox(Window *win, int row, int col, int height, char *label, bool *value);

// String Component ***********************************************************
// Data/Function Types --------------------------------------------------------
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

// Function Prototypes/Inlines ------------------------------------------------
Component *create_string(Window *win, int row, int col, int width, char *label, char *value, int size_max);

// Integer Component **********************************************************
// Data/Function Types --------------------------------------------------------
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

// Function Prototypes/Inlines ------------------------------------------------
Component *create_integer(Window *win, int row, int col, int width, char *label, int *value);

// List Component *************************************************************
// Function Prototypes/Inlines ------------------------------------------------
Component *create_text(Window *win, int row, int col, const char *text);

// Timer Component ************************************************************
// Data/Function Types --------------------------------------------------------
struct timer_t;
typedef void (*Set_Timer)(struct timer_t *timer, unsigned int msecs);
typedef struct timer_t
{
    Component       base;
    unsigned int    counter;

    Set_Timer   set_timer;
}Timer;

// Function Prototypes/Inlines ------------------------------------------------
Component *create_timer(Window *win, unsigned int msecs);

static inline void set_timer(Timer *timer, unsigned int msecs)
{
    if(timer != NULL)
        if(timer->set_timer != NULL)
            timer->set_timer(timer, msecs);
    return;
}

// List Component *************************************************************
// Data/Function Types --------------------------------------------------------
typedef struct text_editor_t
{
    Component   base;
    wchar_t     *buffer;
    int         buffer_size;
    wchar_t     **line;
    int         cursor_row;
    int         cursor_col;
    int         rows;
    int         cols;
    int         height;
    int         width;
    int         select_start;
    int         select_end;
    int         top_visible;
    int         left_visible;
    bool        wrap;
    int         tab_width;
}Text_Editor;

// Function Prototypes/Inlines ------------------------------------------------
Component *create_text_editor(Window *win, int row, int col, int height, int width, wchar_t *buffer, int buffer_size);

static inline void configure_text_editor(Text_Editor *editor)
{
    if(editor != NULL)
        if(editor->base.configure != NULL)
            editor->base.configure(&editor->base);
    return;
}

// Screen Interface ***********************************************************
// Function Prototypes/Inlines ------------------------------------------------
Window *create_screen(void);

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

// Popup Windows **************************************************************
// Function Prototypes/Inlines ------------------------------------------------
void display_error_popup(Window *win, const char *error_message, int milliseconds);

void display_message_popup(Window *win,const char *message, int milliseconds);

Window *yes_no_popup(Window *screen, const char *message, Input_Window yes_no_input);

Component *get_string_popup(Window *screen, char * label, char *value, int length, Input_Component handler);

#endif // LCAF_H

