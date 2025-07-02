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

 /* Building a library to remove dead code
  * 
  * Compile with the -fdata-sections -ffunction-sections options, that tell GCC
  * to put data and functions in separate sections. Sections are just a concept
  * in the object files, basically a stand-alone region. A single object file
  * can contain many sections.
  * 
  * Then link with --gc-sections, telling the linker to garbage-collect unused
  * sections. This will remove dead code.
  */

// Header Sentry
#ifndef LCAF_H
#define LCAF_H

#define __STDC_WANT_LIB_EXT2__ 1  //Define you want TR 24731-2:2010 extensions
#define _XOPEN_SOURCE_EXTENDED

#include <locale.h> // For setlocale
#include <wchar.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wctype.h>
#include <string.h>
#include <stdbool.h>
#include <ncursesw/curses.h>

// Constants ******************************************************************
#ifndef MESSAGE_DURATION
#define MESSAGE_DURATION    1500
#endif

#ifndef KEY_TOGGLE
#define KEY_TOGGLE      '\024'
#endif

#ifndef KEY_ESC
#define KEY_ESC         27
#endif

#ifndef KEY_RESIZE
#define KEY_RESIZE      0632
#endif

#ifndef ERR
#define ERR             (-1)
#endif

#ifndef OK
#define OK              (0)
#endif

#define COLOR_DEFAULT   1
#define COLOR_FIELD     2
#define COLOR_FOCUS     3

// Macros *********************************************************************
// Because static has too many meanings
#define local           static
#define persistant      static

// Text Window ****************************************************************
// Data/Function Types --------------------------------------------------------
typedef enum
{
    APP_OVERFLOW = -5,
    APP_UNABLE_TO_CREATE_WINDOW = -4,
    APP_UNABLE_TO_CREATE_COMPONENT = -3,
    APP_MEMORY_ALLOC_FAIL = -2,
    APP_INVALID_PARAMETER = -1,
    APP_NO_ACTION = 0,
    APP_OK = 1
}App_Status;

/**
 * @brief Enumeration of window frame types.
 *
 * This enumeration defines the different types of frames that can be drawn
 * around a window. Each frame type corresponds to a set of Unicode
 * box-drawing characters.
 */
typedef enum
{
    /** @brief No frame. */
    FRAME_NONE = -1,
    /** @brief Light single-line frame. */
    FRAME_LIGHT = 0,
    /** @brief Light single-line frame with rounded corners. */
    FRAME_LIGHT_ARC = 1,
    /** @brief Heavy single-line frame. */
    FRAME_HEAVY = 2,
    /** @brief Double-line frame. */
    FRAME_DOUBLE = 3,
    /** @brief Hybrid frame (heavy top/bottom, light sides). */
    FRAME_HYBRID = 4,
    /** @brief Light dashed frame. */
    FRAME_DASH_LIGHT = 5,
    /** @brief Light dashed frame with rounded corners. */
    FRAME_DASH_LIGHT_ARC = 6,
    /** @brief Heavy dashed frame. */
    FRAME_DASH_HEAVY = 7
}Window_Frame;

/**
 * @brief Enumeration of frame symbols.
 *
 * This enumeration defines the different symbols used to draw a window frame.
 * Each symbol represents a specific part of the frame, such as a corner or
 * a side.
 */
typedef enum
{
    /** @brief Top-left corner. */
    TOP_LEFT = 0,
    /** @brief Horizontal line. */
    HORIZONTAL,
    /** @brief Top-right corner. */
    TOP_RIGHT,
    /** @brief Vertical line. */
    VERTICAL,
    /** @brief Bottom-right corner. */
    BOTTOM_RIGHT,
    /** @brief Bottom-left corner. */
    BOTTOM_LEFT,
    /** @brief Left break (for window labels). */
    LEFT_BREAK,
    /** @brief Right break (for window labels). */
    RIGHT_BREAK,
    /** @brief Top break (for window labels). */
    TOP_BREAK,
    /** @brief Bottom break (for window labels). */
    BOTTOM_BREAK
}Frame_Symbol;

/**
 * @brief Enumeration of frame label justifications.
 *
 * This enumeration defines the different justifications of the window label
 * located in the top frame of the window.
 */
typedef enum
{
    /** @brief Left justified */
    LABEL_LEFT = 0,
    /** @brief Center justified */
    LABEL_CENTER,
    /** @brief Right justified */
    LABEL_RIGHT
}Label_Justification;

/**
 * @brief Enumeration of cursor types.
 *
 * This enumeration defines the different types of cursors that can be used
 * within a window.
 */
typedef enum
{
    /** @brief No cursor (hidden). */
    CURSOR_NONE,
    /** @brief Insert cursor (vertical bar). */
    CURSOR_INSERT,
    /** @brief Overwrite cursor (block). */
    CURSOR_OVERWRITE
}Cursor_Type;

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
typedef App_Status (*First_Focus_Window)(struct window_t *win);
typedef App_Status (*Last_Focus_Window)(struct window_t *win);
typedef App_Status (*Set_Focus_Window)(struct window_t *win, struct component_t *component);
typedef App_Status (*Next_Focus_Window)(struct window_t *win, bool top_window);
typedef App_Status (*Prev_Focus_Window)(struct window_t *win, bool top_window);
typedef void (*Move_Top_Window)(struct window_t *win);
typedef void (*Move_Bottom_Window)(struct window_t *win);
typedef void (*Move_Up_Window)(struct window_t *win);
typedef void (*Move_Down_Window)(struct window_t *win);
typedef void (*Clear_Window)(struct window_t *win);
typedef int (*Print_Window)(struct window_t *win, int row, int col, const char *format, va_list args);
typedef int (*Wprint_Window)(struct window_t *win, int row, int col, const wchar_t *format, va_list args);
typedef int (*Write_Window)(struct window_t *win, struct window_t *this);
typedef void (*Notify_Window)(struct window_t *win);

/**
 * @brief Represents a text window.
 *
 * This structure defines a text window within the Linux Console Application
 * Framework (LCAF). It contains properties for the window's position, size,
 * content, and behavior.
 */
typedef struct window_t
{
    /** @brief A pointer to the window's content buffer (wide characters). */
    wchar_t *buffer;
    /** @brief A pointer to the window's content attributes. */
    attr_t  *attrs;
    /** @brief A pointer to the window's content colors. */
    short   *colors;
    /** @brief The window's current character attributes. Any characters printed to the window will have these attributes. */
    attr_t  window_attr;
    /** @brief The window's current character colors. Any characters printed to the window will have these colors. */
    short   window_color;
    /** @brief The window's label (null-terminated string). */
    char    *label;
    /** @brief The window's label justification (left|center|right). */
    Label_Justification label_justification;
    /** @brief The row position of the window on the screen. */
    int     row;
    /** @brief The column position of the window on the screen. */
    int     col;
    /** @brief The height of the window in rows. */
    int     height;
    /** @brief The width of the window in columns. */
    int     width;

    /** @brief The current row position of the cursor within the window. */
    int         cur_row;
    /** @brief The current column position of the cursor within the window. */
    int         cur_col;
    /** @brief The type of cursor to display. */
    Cursor_Type cursor_type;

    /** @brief Flag indicating whether insert mode is enabled. */
    bool            insert_key;
    /** @brief Flag indicating whether text wrapping is enabled. */
    bool            wrap;
    /** @brief Flag indicating whether the window's content is stale and needs to be redrawn. */
    bool            stale;
    /** @brief Flag indicating the window is to be closed at the next convenient opportunity. */
    bool            mark_destroy;
    /** @brief The type of frame to draw around the window. */
    Window_Frame    frame_type;

    /** @brief A pointer to the sub window with focus */
    struct window_t *focus_window;
    /** @brief A pointer to the first component in the window's component list. */
    struct component_t *component_head;
    /** @brief A pointer to the last component in the window's component list. */
    struct component_t *component_tail;
    /** @brief A pointer to the component that currently has focus. */
    struct component_t *focus;

    /** @brief A pointer to the parent window that this window belongs to. */
    struct window_t *parent;
    /** @brief A pointer to the top sub-window. */
    struct window_t *top_window;
    /** @brief A pointer to the bottom sub-window. */
    struct window_t *bottom_window;
    /** @brief A pointer to the previous window in the window list. */
    struct window_t *prev;
    /** @brief A pointer to the next window in the window list. */
    struct window_t *next;

    /** @brief A pointer to a user-defined context for the window. */
    void *context;
    /** @brief The size of the user-defined context. */
    size_t context_size;

    /** @brief A function pointer to the window's initialization method. */
    Initialize_Window   initialize;
    /** @brief A function pointer to the window's configuration method. */
    Configure_Window    configure;
    /** @brief A function pointer to the window's frame drawing method. */
    Frame_Window        frame;
    /** @brief A function pointer to the window's update method. */
    Update_Window       update;
    /** @brief A function pointer to the window's input handling method. */
    Input_Window        input;
    /** @brief A function pointer to the window's destruction method. */
    Destroy_Window      destroy;
    /** @brief A function pointer to the window's add method. */
    Add_Window          add;
    /** @brief A function pointer to the window's insert method. */
    Insert_Window       insert;
    /** @brief A function pointer to the window's remove method. */
    Remove_Window       remove;
    /** @brief A function pointer to the window's stale marking method. */
    Stale_Window        set_stale;

    /** @brief A function pointer to the window's cursor setting method. */
    Set_Cursor_Window   set_cursor;

    /** @brief A function pointer to the window's first focus method. */
    First_Focus_Window  first_focus;
    /** @brief A function pointer to the window's last focus method */
    Last_Focus_Window   last_focus;
    /** @brief A function pointer to the window's focus setting method. */
    Set_Focus_Window    set_focus;
    /** @brief A function pointer to the window's next focus method. */
    Next_Focus_Window   next_focus;
    /** @brief A function pointer to the window's previous focus method. */
    Prev_Focus_Window   prev_focus;

    /** @brief A function pointer to the window's move to top method. */
    Move_Top_Window     move_top;
    /** @brief A function pointer to the window's move to bottom method. */
    Move_Bottom_Window  move_bottom;
    /** @brief A function pointer to the window's move up method. */
    Move_Up_Window      move_up;
    /** @brief A function pointer to the window's move down method. */
    Move_Down_Window    move_down;
    /** @brief A function pointer to the window's clear method. */
    Clear_Window        clear_window;
    /** @brief A function pointer to the window's print method. */
    Print_Window        print;
    /** @brief A function pointer to the window's wide character print method. */
    Wprint_Window       wprint;
    /** @brief A function pointer to the window's write method. */
    Write_Window        write;

    /** @brief A function pointer to the window's input notification handler. */
    Input_Window  notify_input;
    /** @brief A function pointer to the window's action notification handler. */
    Input_Window  notify_action;
    /** @brief A function pointer to the window's tick notification handler. */
    Notify_Window notify_tick;
    /** @brief A function pointer to the window's stale notification handler. */
    Notify_Window notify_stale;
    /** @brief A function pointer to the window's focus notification handler. */
    Notify_Window notify_focus;
    /** @brief A function pointer to the window's update notification handler. */
    Notify_Window notify_update;
    /** @brief A function pointer to the window's destroy notification handler. */
    Notify_Window notify_destroy;
}Window;

// Constants ------------------------------------------------------------------
extern wchar_t frame_symbols[8][10];

// Function Prototypes/Inlines ------------------------------------------------
/**
 * @brief Allocates memory for a new window structure.
 *
 * This function allocates memory for a new window, initializes its basic
 * properties (position, size, label, etc.), and sets up its function pointers
 * to their default implementations. It does not add the window to any screen.
 *
 * @param row The starting row position of the window.
 * @param col The starting column position of the window.
 * @param height The height of the window in rows.
 * @param width The width of the window in columns.
 * @param label A null-terminated string to be used as the window's label (can be NULL).
 * @param box A boolean indicating whether the window should have a border (not used in this function).
 * @return A pointer to the newly allocated and initialized window on success,
 *         or NULL on failure.
 */
Window *winAllocate(int row, int col, int height, int width, char *label, bool box);

/**
 * @brief Creates a new window and adds it to a screen.
 *
 * This function allocates memory for a new window, initializes its basic
 * properties (position, size, label, etc.), sets up its function pointers
 * to their default implementations, and then adds it to the specified screen.
 *
 * @param screen A pointer to the screen (root window) to which the new window will be added.
 * @param row The starting row position of the window.
 * @param col The starting column position of the window.
 * @param height The height of the window in rows.
 * @param width The width of the window in columns.
 * @param label A null-terminated string to be used as the window's label (can be NULL).
 * @param box A boolean indicating whether the window should have a border (not used in this function).
 * @return A pointer to the newly created and added window on success,
 *         or NULL on failure.
 */
Window *winCreate(Window *screen, int row, int col, int height, int width, char *label, bool box);

/**
 * @brief Converts a single-byte character to a wide character.
 *
 * This function converts a single-byte character to its wide character
 * representation using the current locale.
 *
 * @param c The single-byte character to convert.
 * @return The wide character representation of the input character,
 *         or L'\0' on error.
 */
wchar_t charToWchar(char c);

/**
 * @brief Copies the content of one window into another.
 * 
 * This function copies the buffer one window into another window. The row and
 * column of the source window is relative the destination window.
 */
int winCopy(Window *src, Window *dst);

/**
 * @brief Returns true if the window is a top level "screen"
 * 
 * This function returns true if the window does not have a parent. This
 * indicates that the window is a "screen". If the window does have a parent
 * the function returns false.
 */
static inline bool isScreen(Window *this)
{
    if(this == NULL)
        return(false);
    return(this->parent?false:true);
}

/**
 * @brief Initializes a window.
 *
 * This inline function calls the window's initialize method if it exists.
 *
 * @param this A pointer to the window to initialize.
 */
static inline void winInitialize(Window *this)
{
    if(this != NULL)
        if(this->initialize != NULL)
            this->initialize(this);
}

/**
 * @brief Configures a window.
 *
 * This inline function calls the window's configure method if it exists.
 *
 * @param this A pointer to the window to configure.
 */
static inline int winConfigure(Window *this)
{
    if(this != NULL)
        if(this->configure != NULL)
            return(this->configure(this));
    return(0);
}

/**
 * @brief Passes user input to a window.
 *
 * This inline function calls the window's input method if it exists.
 *
 * @param this A pointer to the window to pass input.
 * @param ch Integer value of the user input.
 */
static inline int winInput(Window *this, int ch)
{
    if(this != NULL)
        if(this->input != NULL)
            return(this->input(this,ch));
    return(0);
}

/**
 * @brief Draws the frame of a window.
 *
 * This inline function calls the window's frame method if it exists.
 *
 * @param this A pointer to the window to draw the frame on.
 */
static inline void winFrame(Window *this)
{
    if(this != NULL)
        if(this->frame != NULL)
            this->frame(this);
    return;
}

/**
 * @brief Adds a window to a screen.
 *
 * This inline function calls the screen's add method to add a window to the screen.
 *
 * @param screen A pointer to the screen to add the window to.
 * @param win A pointer to the window to add.
 */
static inline void winAdd(Window *screen, Window *win)
{
    if(screen != NULL && win != NULL)
        if(screen->add != NULL)
            screen->add(screen, win);
    return;
}

/**
 * @brief Inserts a window into the window list.
 *
 * This inline function calls the reference window's insert method to insert a window
 * into the list of windows.
 *
 * @param ref A pointer to the reference window to insert the new window before.
 * @param win A pointer to the window to insert.
 */
static inline void winInsert(Window *ref, Window *win)
{
    if(ref != NULL && win != NULL)
        if(ref->insert != NULL)
            ref->insert(ref, win);
    return;
}

/**
 * @brief Removes a window from the window list.
 *
 * This inline function calls the window's remove method to remove it from the list.
 *
 * @param win A pointer to the window to remove.
 */
static inline void winRemove(Window *win)
{
    if(win != NULL)
        if(win->remove != NULL)
            win->remove(win);
    return;
}

/**
 * @brief Enables or disables the cursor for a window.
 *
 * This inline function calls the window's set_cursor method to enable or disable
 * the cursor.
 *
 * @param this A pointer to the window.
 * @param enable A boolean indicating whether to enable (true) or disable (false) the cursor.
 */
static inline void winSetCursor(Window *this, bool enable)
{
    if(this != NULL)
        if(this->set_cursor != NULL)
            this->set_cursor(this, enable);
    return;    
}

/**
 * @brief Marks a window as stale.
 *
 * This inline function calls the window's set_stale method to mark it as stale,
 * indicating that it needs to be redrawn.
 *
 * @param this A pointer to the window to mark as stale.
 */
static inline void winSetStale(Window *this)
{
    if(this != NULL)
        if(this->set_stale != NULL)
            this->set_stale(this);
    return;
}

/**
 * @brief Sets the focus to the first component within a window.
 *
 * This inline function calls the window's fisrt_focus method to set the focus to
 * the first component of this window and sub-windows.
 *
 * @param this A pointer to the window.
 */
static inline App_Status winFirstFocus(Window *this)
{
    if(this != NULL)
        if(this->first_focus != NULL)
            return(this->first_focus(this));
    return(APP_NO_ACTION);
}

/**
 * @brief Sets the focus to the last component within a window.
 *
 * This inline function calls the window's last_focus method to set the focus to
 * the last component of this window and sub-windows.
 *
 * @param this A pointer to the window.
 */
static inline App_Status winLastFocus(Window *this)
{
    if(this != NULL)
        if(this->last_focus != NULL)
            return(this->last_focus(this));
    return(APP_NO_ACTION);
}

/**
 * @brief Sets the focus to a component within a window.
 *
 * This inline function calls the window's set_focus method to set the focus to a
 * specific component.
 *
 * @param this A pointer to the window.
 * @param component A pointer to the component to receive focus.
 */
static inline App_Status winSetFocus(Window *this, struct component_t *component)
{
    if(this != NULL)
        if(this->set_focus != NULL)
            return(this->set_focus(this, component));
    return(APP_NO_ACTION);
}

/**
 * @brief Moves the focus to the next component in a window.
 *
 * This inline function calls the window's next_focus method to move the focus to
 * the next component in the window's component list.
 *
 * @param this A pointer to the window.
 */
static inline App_Status winNextFocus(Window *this)
{
    if(this == NULL)
        return(APP_INVALID_PARAMETER);
    if(this->next_focus != NULL)
        return(this->next_focus(this,true));
    return(APP_NO_ACTION);
}

/**
 * @brief Moves the focus to the previous component in a window.
 *
 * This inline function calls the window's prev_focus method to move the focus to
 * the previous component in the window's component list.
 *
 * @param this A pointer to the window.
 */
static inline App_Status winPrevFocus(Window *this)
{
    if(this != NULL)
        if(this->prev_focus != NULL)
            return(this->prev_focus(this,true));
    return(APP_NO_ACTION);
}

/**
 * @brief Moves a window to the top of the window stack.
 *
 * This inline function calls the window's move_top method to move it to the top
 * of the window stack.
 *
 * @param this A pointer to the window to move.
 */
static inline void winMoveTop(Window *this)
{
    if(this != NULL)
        if(this->move_top != NULL)
            this->move_top(this);
    return;
}

/**
 * @brief Moves a window to the bottom of the window stack.
 *
 * This inline function calls the window's move_bottom method to move it to the
 * bottom of the window stack.
 *
 * @param this A pointer to the window to move.
 */
static inline void winMoveBottom(Window *this)
{
    if(this != NULL)
        if(this->move_bottom != NULL)
            this->move_bottom(this);
    return;
}

/**
 * @brief Moves a window up one position in the window stack.
 *
 * This inline function calls the window's move_up method to move it up one
 * position in the window stack.
 *
 * @param this A pointer to the window to move.
 */
static inline void winMoveUp(Window *this)
{
    if(this != NULL)
        if(this->move_up != NULL)
            this->move_up(this);
    return;
}

/**
 * @brief Moves a window down one position in the window stack.
 *
 * This inline function calls the window's move_down method to move it down one
 * position in the window stack.
 *
 * @param this A pointer to the window to move.
 */
static inline void winMoveDown(Window *this)
{
    if(this != NULL)
        if(this->move_down != NULL)
            this->move_down(this);
    return;
}

/**
 * @brief Clears the content of a window.
 *
 * This inline function calls the window's clear_window method to clear its
 * content.
 *
 * @param this A pointer to the window to clear.
 */
static inline void winClear(Window *this)
{
    if(this != NULL)
        if(this->clear_window != NULL)
            this->clear_window(this);
    return;
}

/**
 * @brief Prints formatted text to a window.
 *
 * This inline function prints formatted text to the window's buffer at the
 * specified row and column. It uses variable arguments similar to printf.
 *
 * @param this A pointer to the window to print to.
 * @param row The row to start printing at.
 * @param col The column to start printing at.
 * @param format The format string, similar to printf.
 * @param ... Variable arguments to be formatted.
 * @return The number of characters written to the window.
 */
static inline int winPrint(Window *this, int row, int col, const char *format, ...)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    va_list args;
    va_start(args, format);

    int result = this->print(this, row, col, format, args);

    va_end(args);
    return(result);
}

/**
 * @brief Prints formatted wide character text to a window.
 *
 * This inline function prints formatted wide character text to the window's
 * buffer at the specified row and column. It uses variable arguments similar
 * to wprintf.
 *
 * @param this A pointer to the window to print to.
 * @param row The row to start printing at.
 * @param col The column to start printing at.
 * @param format The wide character format string, similar to wprintf.
 * @param ... Variable arguments to be formatted.
 * @return The number of wide characters written to the window.
 */
static inline int winWprint(Window *this, int row, int col, const wchar_t *format, ...)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    va_list args;
    va_start(args, format);

    int result = this->wprint(this, row, col, format, args);

    va_end(args);
    return(result);
}

/**
 * @brief Writes the content of one window to another.
 *
 * This inline function copies the content of one window (this) to another window
 * (src). It is used for composing windows.
 *
 * @param src A pointer to the destination window.
 * @param this A pointer to the source window.
 * @return The number of characters copied, or -1 on error.
 */
static inline int winWrite(Window *src, Window *this)
{
    if(src == NULL || this == NULL)
        return(-1);

    int result = this->write(src, this);
    return(result);
}

/**
 * @brief Sets a window to be destroyed.
 *
 * This inline function sets the window's destroy_window flag. This causes the
 * window to be closed at the next convienent opportunity.
 *
 * @param this A pointer to the window to destroy.
 */
static inline void winMarkDestroy(Window *this)
{
    if(this != NULL)
        this->mark_destroy = true;
    return;
}

/**
 * @brief Notifies the window of an action event.
 *
 * This inline function calls the window's action notification handler if it exists.
 * An action event is typically triggered by a user action, such as pressing a key
 * or selecting an option.
 *
 * @param this A pointer to the window to notify.
 * @param ch The character or key code associated with the action.
 */
static inline void winNotifyAction(Window *this, int ch)
{
    if(this != NULL)
        if(this->notify_action != NULL)
            this->notify_action(this, ch);
    return;
}

/**
 * @brief Notifies the window of an input event.
 *
 * This inline function calls the window's input notification handler if it exists.
 * An input event is typically triggered by a user pressing a key.
 *
 * @param this A pointer to the window to notify.
 * @param ch The character or key code associated with the input.
 */
static inline void winNotifyInput(Window *this, int ch)
{
    if(this != NULL)
        if(this->notify_input != NULL)
            this->notify_input(this, ch);
    return;
}

/**
 * @brief Notifies the window of a tick event.
 *
 * This inline function calls the window's tick notification handler if it exists.
 * A tick event is a periodic event, typically used for time-based updates.
 *
 * @param this A pointer to the window to notify.
 */
static inline void winNotifyTick(Window *this)
{
    if(this != NULL)
        if(this->notify_tick != NULL)
            this->notify_tick(this);
    return;
}

/**
 * @brief Notifies the window that it has become stale.
 *
 * This inline function calls the window's stale notification handler if it exists.
 * A window becomes stale when its content needs to be redrawn.
 *
 * @param this A pointer to the window to notify.
 */
static inline void winNotifyStale(Window *this)
{
    if(this != NULL)
        if(this->notify_stale != NULL)
            this->notify_stale(this);
    return;
}

/**
 * @brief Notifies the window that it has gained focus.
 *
 * This inline function calls the window's focus notification handler if it exists.
 * A window gains focus when it becomes the active window for user input.
 *
 * @param this A pointer to the window to notify.
 */
static inline void winNotifyFocus(Window *this)
{
    if(this != NULL)
        if(this->notify_focus != NULL)
            this->notify_focus(this);
    return;
}

/**
 * @brief Notifies the window that it needs to be updated.
 *
 * This inline function calls the window's update notification handler if it exists.
 * This is called when the window's content may have changed and needs to be
 * refreshed.
 *
 * @param this A pointer to the window to notify.
 */
static inline void winNotifyUpdate(Window *this)
{
    if(this != NULL)
        if(this->notify_update != NULL)
            this->notify_update(this);
    return;
}

/**
 * @brief Notifies the window that it is about to be destroyed.
 *
 * This inline function calls the window's destroy notification handler if it exists.
 * This is called just before the window is destroyed, allowing for cleanup.
 *
 * @param this A pointer to the window to notify.
 */
static inline void winNotifyDestroy(Window *this)
{
    if(this != NULL)
        if(this->notify_destroy != NULL)
            this->notify_destroy(this);
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

/**
 * @brief Represents a user interface component.
 *
 * This structure defines the base type for all UI components within the
 * Linux Console Application Framework (LCAF). It contains common properties
 * and function pointers for managing the component's behavior and appearance.
 */
typedef struct component_t
{
    /**
     * @brief The label associated with the component.
     *
     * This is a null-terminated string that can be used to identify or
     * describe the component in the UI.
     */
    const char  *label;
    /**
     * @brief The row position of the component within its parent window.
     */
    int         row;
    /**
     * @brief The column position of the component within its parent window.
     */
    int         col;
    /**
     * @brief The height of the component in rows.
     */
    int         height;
    /**
     * @brief The width of the component in columns.
     */
    int         width;
    /**
     * @brief The format string used for displaying the component's value.
     *
     * This is a null-terminated string similar to that used by printf.
     */
    const char  *format;
    
    /**
     * @brief A pointer to the parent window of this component.
     */
    Window  *parent;
    /**
     * @brief A pointer to a child window of this component.
     */
    Window  *window;
    
    /** 
     * @brief A pointer to a user-defined context for the window. 
     */
    void *context;
    /**
     * @brief The size of the user-defined context.
     */
    size_t context_size;

    /**
     * @brief A pointer to the previous component in the parent window's component list.
     */
    struct component_t *prev;
    /**
     * @brief A pointer to the next component in the parent window's component list.
     */
    struct component_t *next;

    /**
     * @brief A function pointer to the component's configuration method.
     *
     * This method is called to perform any necessary setup or configuration
     * for the component.
     */
    Configure_Component configure;
    /**
     * @brief A function pointer to the component's update method.
     *
     * This method is called to update the component's visual representation.
     */
    Update_Component    update;
    /**
     * @brief A function pointer to the component's input handling method.
     *
     * This method is called to process user input for the component.
     */
    Input_Component     input;
    /**
     * @brief A function pointer to the component's focus method.
     *
     * This method is called when the component receives focus.
     */
    Focus_Component     focus;

    /**
     * @brief A function pointer to the component's add method.
     *
     * This method is called to add the component to a window.
     */
    Add_Component       add;
    /**
     * @brief A function pointer to the component's remove method.
     *
     * This method is called to remove the component from a window.
     */
    Remove_Component    remove;
    /**
     * @brief A function pointer to the component's destroy method.
     *
     * This method is called to free any resources allocated by the component.
     */
    Destroy_Component   destroy;

    /**
     * @brief A function pointer to the component's read-only method.
     *
     * This method is called to set the component to read-only mode.
     */
    Readonly_Component  read_only;
    /**
     * @brief A function pointer to the component's set format method.
     *
     * This method is called to set the format string for the component.
     */
    Set_Format_Component set_format;

    /**
     * @brief A function pointer to the component's input notification handler.
     *
     * This method is called when the component receives input.
     */
    Input_Component  notify_input;
    /**
     * @brief A function pointer to the component's action notification handler.
     *
     * This method is called when an action event occurs for the component.
     */
    Input_Component  notify_action;
    /**
     * @brief A function pointer to the component's tick notification handler.
     *
     * This method is called periodically for time-based updates.
     */
    Notify_Component notify_tick;
    /**
     * @brief A function pointer to the component's focus notification handler.
     *
     * This method is called when the component gains focus.
     */
    Notify_Component notify_focus;
    /**
     * @brief A function pointer to the component's destroy notification handler.
     *
     * This method is called just before the component is destroyed.
     */
    Notify_Component notify_destroy;
}Component;

// Function Prototypes/Inlines ------------------------------------------------
/**
 * @brief Creates a new component and adds it to a window.
 *
 * This function allocates memory for a new component, initializes its basic
 * properties (label, position, size, etc.), sets up its function pointers
 * to their default implementations, and adds it to the specified window.
 *
 * @param component A pointer to the pre-allocated component structure to initialize.
 * @param win A pointer to the window to which the component will be added.
 * @param row The starting row position of the component within the window.
 * @param col The starting column position of the component within the window.
 * @param height The height of the component in rows.
 * @param width The width of the component in columns.
 * @param label A null-terminated string to be used as the component's label (can be NULL).
 * @return A pointer to the newly created and added component on success,
 *         or NULL on failure.
 */
Component *compCreate(Component *component, Window *win, int row, int col, int height, int width, const char *label);

/**
 * @brief Sets a component to read-only mode.
 * 
 * This function calls the component's `read_only` method, if it exists, to
 * make the component read-only. This typically means the component will not
 * accept user input or allow modifications to its value.
 *
 * @param this A pointer to the component to make read-only.
 */
static inline void compReadOnly(Component *this)
{
    if(this != NULL)
        if(this->read_only != NULL)
            this->read_only(this);
    return;
}

/**
 * @brief Sets the display format for a component.
 * 
 * This function calls the component's `set_format` method, if it exists, to
 * set the format string used for displaying the component's value. The format
 * string is similar to that used by `printf`.
 *
 * @param this A pointer to the component to set the format for.
 * @param format The format string to use for displaying the component's value.
 */
static inline void compSetFormat(Component *this, const char *format)
{
    if(this != NULL)
        if(this->set_format != NULL)
            this->set_format(this, format);
    return;
}

/**
 * @brief Handles input for a component.
 * 
 * This function calls the component's `input` method, if it exists, to
 * process user input for the component.
 *
 * @param this A pointer to the component to handle input for.
 * @param input The input character or key code.
 * @return The result of the component's input handler, or 0 if the component
 *         does not have an input handler.
 */
static inline int compInput(Component *this, int input)
{
    if(this != NULL)
        if(this->input != NULL)
            return(this->input(this, input));
    return(0);
}

/**
 * @brief Configures a component.
 * 
 * This function calls the component's `configure` method, if it exists, to
 * perform any necessary setup or configuration for the component.
 *
 * @param this A pointer to the component to configure.
 */
static inline void compConfigure(Component *this)
{
    if(this != NULL)
        if(this->configure != NULL)
            this->configure(this);
    return;
}

/**
 * @brief Sets a component as the current focus.
 * 
 * This function calls the component's `focus` method, if it exists, to
 * perform any necessary action when the component gets the focus.
 *
 * @param this A pointer to the component to configure.
 */
static inline void compSetFocus(Component *this)
{
    if(this != NULL)
        if(this->focus != NULL)
            this->focus(this);
    return;
}

/**
 * @brief Destroys a component.
 * 
 * This function calls the component's `destroy` method, if it exists, to
 * free any resources allocated by the component and remove it from the UI.
 *
 * @param this A pointer to the component to destroy.
 */
static inline void compDestroy(Component *this)
{
    if(this != NULL)
        if(this->destroy != NULL)
            this->destroy(this);
    return;
}

/**
 * @brief Notifies a component of an action event.
 * 
 * This function calls the component's `notify_action` method, if it exists, to
 * inform the component of an action event. An action event is typically
 * triggered by a user action, such as pressing a key or selecting an option.
 *
 * @param this A pointer to the component to notify.
 * @param ch The character or key code associated with the action.
 */
static inline void compNotifyAction(Component *this, int ch)
{
    if(this != NULL)
        if(this->notify_action != NULL)
            this->notify_action(this, ch);
    return;
}

/**
 * @brief Notifies a component set focus event.
 * 
 * This function calls the component's `notify_action` method, if it exists, to
 * inform the component of an action event. An action event is typically
 * triggered by a user action, such as pressing a key or selecting an option.
 *
 * @param this A pointer to the component to notify.
 */
static inline void compNotifyFocus(Component *this)
{
    if(this != NULL)
        if(this->notify_focus != NULL)
            this->notify_focus(this);
    return;
}

/**
 * @brief Notifies a component that it is about to be destroyed.
 * 
 * This function calls the component's `notify_destroy` method, if it exists, to
 * inform the component that it is about to be destroyed. This allows the
 * component to perform any necessary cleanup before being removed from the UI.
 *
 * @param this A pointer to the component to notify.
 */
static inline void compNotifyDestroy(Component *this)
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
/**
 * @brief Creates a new list component.
 *
 * This function allocates and initializes a list component, which allows the
 * user to select from a list of items. The list can be displayed within a
 * window and navigated using the arrow keys.
 *
 * @param win A pointer to the window to which the list will be added.
 * @param row The starting row position of the list within the window.
 * @param col The starting column position of the list within the window.
 * @param height The height of the list in rows.
 * @param width The width of the list in columns.
 * @param label A null-terminated string to be used as the list's label.
 * @param items A pointer to a pre-allocated character array containing the list items.
 *              Each item should be null-terminated, and items are stored sequentially.
 * @param max_items The maximum number of items that can be stored in the list.
 * @param max_length The maximum length of each item in the list.
 * @return A pointer to the newly created list component on success,
 *         or NULL on failure.
 */
Component *listCreate(Window *win, int row, int col, int height, int width, char *label, char *items, int max_items, int max_length);

/**
 * @brief Gets a pointer to the currently selected item in a list.
 *
 * This inline function returns a pointer to the character array representing
 * the currently selected item in the list.
 *
 * @param list A pointer to the list component.
 * @return A pointer to the selected item's string, or NULL if the list is NULL.
 */
static inline char *listSelectedItem(List *list)
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
/**
 * @brief Creates a new checkbox component.
 *
 * This function allocates and initializes a checkbox component, which allows
 * the user to toggle a boolean value on or off. The checkbox can be displayed
 * within a window and interacted with using the spacebar or a toggle key.
 *
 * @param win A pointer to the window to which the checkbox will be added.
 * @param row The starting row position of the checkbox within the window.
 * @param col The starting column position of the checkbox within the window.
 * @param width The width of the checkbox in columns.
 * @param label A null-terminated string to be used as the checkbox's label.
 * @param value A pointer to the boolean value that the checkbox will control.
 * @return A pointer to the newly created checkbox component on success,
 *         or NULL on failure.
 */
Component *chkboxCreate(Window *win, int row, int col, int width, char *label, bool *value);

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
/**
 * @brief Creates a new string component.
 *
 * This function allocates and initializes a string component, which allows
 * the user to input and edit a string of text. The string can be displayed
 * within a window and interacted with using the keyboard.
 *
 * @param win A pointer to the window to which the string component will be added.
 * @param row The starting row position of the string component within the window.
 * @param col The starting column position of the string component within the window.
 * @param width The width of the string component in columns.
 * @param label A null-terminated string to be used as the string component's label.
 * @param value A pointer to a pre-allocated character array to store the string value.
 * @param size_max The maximum size of the string value buffer.
 * @return A pointer to the newly created string component on success,
 *         or NULL on failure.
 */
Component *strCreate(Window *win, int row, int col, int width, char *label, char *value, int size_max);

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
/**
 * @brief Creates a new integer component.
 *
 * This function allocates and initializes an integer component, which allows
 * the user to input and edit an integer value. The integer can be displayed
 * within a window and interacted with using the keyboard.
 *
 * @param win A pointer to the window to which the integer component will be added.
 * @param row The starting row position of the integer component within the window.
 * @param col The starting column position of the integer component within the window.
 * @param width The width of the integer component in columns.
 * @param label A null-terminated string to be used as the integer component's label.
 * @param value A pointer to the integer value that the component will control.
 * @return A pointer to the newly created integer component on success,
 *         or NULL on failure.
 */
Component *intCreate(Window *win, int row, int col, int width, char *label, int *value);

// Text Component *************************************************************
// Function Prototypes/Inlines ------------------------------------------------
/**
 * @brief Creates a new text component.
 *
 * This function allocates and initializes a text component, which displays
 * static text within a window. It is typically used for labels or other
 * non-interactive text elements.
 *
 * @param win A pointer to the window to which the text component will be added.
 * @param row The starting row position of the text component within the window.
 * @param col The starting column position of the text component within the window.
 * @param text A null-terminated string containing the text to display.
 * @return A pointer to the newly created text component on success,
 *         or NULL on failure.
 */
Component *txtCreate(Window *win, int row, int col, const char *text);

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
/**
 * @brief Creates a new timer component.
 *
 * This function allocates and initializes a timer component, which can be used
 * to trigger an action after a specified number of milliseconds. The timer
 * component does not have a visual representation; it operates in the background.
 *
 * @param win A pointer to the window to which the timer will be associated.
 * @param msecs The number of milliseconds before the timer triggers.
 * @return A pointer to the newly created timer component on success,
 *         or NULL on failure.
 */
Component *tmrCreate(Window *win, unsigned int msecs);

/**
 * @brief Sets the timer's duration.
 *
 * This inline function sets the number of milliseconds the timer will run
 * before triggering its action.
 *
 * @param timer A pointer to the timer component.
 * @param msecs The number of milliseconds to set the timer to.
 */
static inline void tmrSet(Timer *timer, unsigned int msecs)
{
    if(timer != NULL)
        if(timer->set_timer != NULL)
            timer->set_timer(timer, msecs);
    return;
}

// Frame Component ************************************************************
// Data/Function Types --------------------------------------------------------
typedef struct frame_t
{
    Component           base;
    Window_Frame        frame_type;
    Label_Justification label_justification;
}Frame;

// Function Prototypes/Inlines ------------------------------------------------
Component *frameCreate(Window *win, int row, int col, int height, int width, const char *label);

// Console Component **********************************************************
// Data Types -----------------------------------------------------------------
typedef struct 
{
    Component   base;
    wchar_t     *prompt;
    wchar_t     *command;
    wchar_t     *buffer;
    int         buffer_rows;
    int         command_size;
    wchar_t     **history;
    int         history_depth;
    int         history_max;
    int         cursor_offset;
}Console;

// Function Prototypes/Inlines ------------------------------------------------
Component *consoleCreate(Window *win, int row, int col, int height, int width, int buffer_lines, wchar_t *command, int command_size);

static inline void destroyConsole(Console *console)
{
    if(console != NULL)
        if(console->base.destroy != NULL)
            console->base.destroy(&console->base);
    return;
}

// Text Edit Component ********************************************************
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
/**
 * @brief Creates a new text editor component.
 *
 * This function allocates and initializes a text editor component, which allows
 * for multi-line text editing within a window. It manages a buffer for the text,
 * cursor position, and visible area.
 *
 * @param win A pointer to the window to which the text editor will be added.
 * @param row The starting row position of the text editor within the window.
 * @param col The starting column position of the text editor within the window.
 * @param height The height of the text editor in rows.
 * @param width The width of the text editor in columns.
 * @param buffer A pointer to a pre-allocated wide character buffer to store the text.
 * @param buffer_size The size of the buffer in wide characters.
 * @return A pointer to the newly created text editor component on success,
 *         or NULL on failure.
 */
Component *txteditCreate(Window *win, int row, int col, int height, int width, wchar_t *buffer, int buffer_size);

/**
 * @brief Configures the text editor's internal line array.
 *
 * This inline function calls the text editor's configure method, which is
 * responsible for parsing the text buffer and building an array of pointers
 * to the start of each line. This is necessary after any modification to the
 * buffer.
 *
 * @param editor A pointer to the text editor to configure.
 */
static inline void txteditConfigure(Text_Editor *editor)
{
    if(editor != NULL)
        if(editor->base.configure != NULL)
            editor->base.configure(&editor->base);
    return;
}

// Screen Interface ***********************************************************
// Function Prototypes/Inlines ------------------------------------------------
/**
 * @brief Creates and initializes a new screen (root window).
 *
 * This function allocates memory for a new screen structure, initializes the
 * underlying ncurses library, and sets up the screen's basic properties.
 *
 * @return A pointer to the newly created screen (root window) on success,
 *         or NULL on failure.
 */
Window *scrnCreate(void);

/**
 *@brief Gets a pointer to the screen instance.
 *
 * This function returns the a pointer to the screen instance for the given
 * window.
 * 
 * @return A pointer to the root window for this window.
 */
static inline Window *scrnGet(Window *win)
{
    while(win->parent)
        win = win->parent;
    return(win);
}

/**
 * @brief Runs the main application loop for the screen.
 *
 * This function starts the main event loop for the application, handling
 * user input, screen updates, and window management. It continues to run
 * until the application is terminated.
 *
 * @param screen A pointer to the screen (root window) to run.
 * @return 0 on normal termination, or -1 on error.
 */
int scrnRun(Window *screen);

/**
 * @brief Handles input for the screen (root window).
 *
 * This function is intended to be a placeholder for a screen-level input handler.
 * In the current implementation, it does not perform any actions.
 *
 * @param this A pointer to a pointer to the screen (root window).
 * @return 0.
 * @deprecated This function is not used in the current implementation and may be removed in the future.
 */
int scrnInput(Window **this);

/**
 * @brief Initializes the screen (root window).
 *
 * This inline function calls the screen's initialize method if it exists.
 *
 * @param this A pointer to the screen (root window) to initialize.
 */
static inline void scrnInitialize(Window *this)
{
    if(this != NULL)
        if(this->initialize != NULL)
            this->initialize(this);
}

/**
 * @brief Destroys the screen (root window).
 *
 * This inline function calls the screen's destroy method if it exists.
 *
 * @param this A pointer to the screen (root window) to destroy.
 */
static inline void scrnDestroy(Window *this)
{
    if(this != NULL)
        if(this->destroy != NULL)
            this->destroy(this);
}

/**
 * @brief Updates the screen (root window).
 *
 * This inline function calls the screen's update method if it exists.
 *
 * @param this A pointer to the screen (root window) to update.
 */
static inline void scrnUpdate(Window *this)
{
    if(this != NULL)
        if(this->update != NULL)
            this->update(this);
}

/**
 * @brief Writes the screen (root window) to the terminal.
 *
 * This inline function calls the screen's write method if it exists.
 *
 * @param src A pointer to the screen (root window) to write.
 */
static inline void scrnWrite(Window *src)
{
    if(src != NULL)
        if(src->write != NULL)
            src->write(NULL,src);
}


// Popup Windows **************************************************************
// Function Prototypes/Inlines ------------------------------------------------
/**
 * @brief Displays an error message in a popup window.
 *
 * This function creates a temporary popup window to display an error message
 * to the user. The window will automatically close after a specified duration.
 *
 * @param reference A pointer to the screen (root window) on which to display the popup.
 * @param error_message A null-terminated string containing the error message to display.
 * @param milliseconds The number of milliseconds to display the popup window.
 */
void popupError(Window *reference, const char *error_message, int milliseconds);

/**
 * @brief Displays a message in a popup window.
 *
 * This function creates a temporary popup window to display a message to the
 * user. The window will automatically close after a specified duration.
 *
 * @param reference A pointer to the screen (root window) on which to display the popup.
 * @param message A null-terminated string containing the message to display.
 * @param milliseconds The number of milliseconds to display the popup window.
 */
void popupMessage(Window *reference, const char *message, int milliseconds);

/**
 * @brief Displays a Yes/No confirmation popup window.
 *
 * This function creates a popup window that displays a message and prompts
 * the user to confirm with a Yes or No response.
 *
 * @param reference A pointer to the screen (root window) on which to display the popup.
 * @param message A null-terminated string containing the message to display.
 * @param yes_no_input A function pointer to the input handler for the Yes/No response.
 * @return A pointer to the newly created popup window on success, or NULL on failure.
 */
Window *popupYesNo(Window *reference, const char *message, Input_Window yes_no_input);

/**
 * @brief Displays a popup window to get a string input from the user.
 *
 * This function creates a popup window that prompts the user to enter a string.
 * It includes a label, an input field, and an optional handler for when the
 * user completes the input.
 *
 * @param reference A pointer to the screen (root window) on which to display the popup.
 * @param label A null-terminated string to be used as the input prompt label.
 * @param value A pointer to a pre-allocated character array to store the input string.
 * @param length The maximum length of the input string buffer.
 * @param handler A function pointer to an optional input handler that will be called when the user completes the input.
 * @return A pointer to the newly created string component on success, or NULL on failure.
 */
Component *popupGetString(Window *reference, char * label, char *value, int length, Input_Component handler);

// Hardware Abstraction *******************************************************
// Function Prototypes --------------------------------------------------------
/**
 * @brief Retrieves the size of the display.
 * 
 * This function retrieves the dimensions of the display in rows and columns.
 *
 * @param rows A pointer to an integer variable where the number of rows will be stored.
 * @param cols A pointer to an integer variable where the number of columns will be stored.
 * @return 0 on success, -1 on failure.
 */
int halGetDisplaySize(int *rows, int *cols);

/**
 * @brief Initializes the hardware abstraction layer (HAL).
 * 
 * This function initializes the HAL, which provides an interface to the
 * underlying hardware for input and output.  It typically initializes the
 * ncurses library.
 *
 * @return 0 on success, -1 on failure.
 */
int halInitialize(void);

/**
 * @brief Shuts down the hardware abstraction layer (HAL).
 * 
 * This function shuts down the HAL, releasing any resources it holds.
 * It typically shuts down the ncurses library.
 */
void halShutDown(void);

/**
 * @brief Gets user input from the terminal.
 * 
 * This function reads a single character of input from the terminal.
 *
 * @return The character code of the input, or ERR on error.
 */
int halGetInput(void);

/**
 * @brief Shows or hides the cursor.
 * 
 * This function shows or hides the cursor on the terminal.
 *
 * @param enable A boolean value indicating whether to show (true) or hide (false) the cursor.
 */
void halShowCursor(bool enable);

/**
 * @brief Sets the cursor type.
 * 
 * This function sets the type of cursor to be displayed on the terminal.
 *
 * @param type The type of cursor to set (CURSOR_INSERT or CURSOR_OVERWRITE).
 * @return A positive value if the cursor type was successfully set, 0 otherwise.
 */
int halSetCursorType(Cursor_Type type);

/**
 * @brief Sets the cursor position.
 * 
 * This function sets the position of the cursor on the terminal.
 *
 * @param row The row number (0-based) to set the cursor to.
 * @param col The column number (0-based) to set the cursor to.
 */
void halSetCursorPosition(int row, int col);

/**
 * @brief Writes a buffer to the display.
 * 
 * This function writes the content of a wide-character buffer to the display.
 *
 * @param buffer A pointer to the wide-character buffer to write.
 * @return The number of characters written, or 0 on error.
 */
int halWriteDisplay(wchar_t *buffer, uint32_t *attrs, short *colors);

/**
 * @brief Refreshes the display.
 * 
 * This function refreshes the terminal display, ensuring that any changes made
 * are visible to the user.
 */
void halRefreshDisplay(void);

// CSV File Parsing ***********************************************************
/**
 * @brief Removes leading and trailing whitespace characters from a string.
 * 
 * This function trims whitespace characters (spaces, tabs, newlines, and carriage returns)
 * from the beginning and end of the input string. It modifies the string in place.
 *
 * @param str A pointer to the null-terminated string to be trimmed.
 * @return A pointer to the beginning of the trimmed string (which may be the same as the input pointer).
 */
char *csvTrim(char *str);

/**
 * @brief Reads a single field from a CSV file.
 * 
 * This function reads characters from a CSV file, one at a time, until a delimiter (comma)
 * is encountered or the end of the file is reached. It handles quoted fields, allowing
 * commas within quotes.
 *
 * @param file A pointer to the FILE stream to read from.
 * @return A pointer to a dynamically allocated string containing the field's content.
 *         Returns NULL if the end of the file is reached or if a memory allocation error occurs.
 *         The caller is responsible for freeing the allocated memory.
 */
char *csvReadField(FILE *file);

#endif // LCAF_H

