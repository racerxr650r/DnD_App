/*
 * comp.c
 *
 * Component functions of the text windowing UI component of the Linux Console
 * Application Framework (lcaf) library. These functions implement the
 * base UI components
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
#include "lcaf.h"

// Base Component *************************************************************
local void compAddMethod(Window *win, Component *component)
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

local void compRemoveMethod(Window *win, Component *component)
{
    if(win == NULL || component == NULL || win->component_head == NULL || win->component_tail == NULL)
        return;

    // If this component has the focus...
    if(component->parent->focus == component)
        // Decrement the focus to the previous valid component
        component->parent->prev_focus(component->parent, true);

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

local void compDestroyMethod(Component *component)
{
    // Remove the component from the window
    component->remove(component->parent, component);
    
    if(component->window != NULL)
        if(component->window->destroy != NULL)
            component->window->destroy(component->window);

    compNotifyDestroy(component);

    free(component);
}

local void compReadOnlyMethod(Component *component)
{
    if(component == NULL || component->parent == NULL)
        return;

    //component->parent->prev_focus(component->parent,true);
    if(component->parent->focus == component)
        winNextFocus(component->parent);

    component->focus = NULL;
}

local void compSetFormatMethod(Component *component, const char *format)
{
    if(component == NULL)
        return;

    component->format = format;
}

Component *compCreate(Component *component, Window *win, int row, int col, int height, int width, const char *label)
{
    if(component == NULL || win == NULL)
        return(NULL);

    if(label != NULL)
        component->label = label;
    else
        component->label = "";

    component->parent = win;
    component->window = NULL;
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
    
    component->add = compAddMethod;
    component->remove = compRemoveMethod;
    component->destroy = compDestroyMethod;
    
    component->read_only = compReadOnlyMethod;
    component->set_format = compSetFormatMethod;

    component->notify_action = NULL;
    component->notify_input = NULL;
    component->notify_tick = NULL;
    component->notify_focus = NULL;
    component->notify_destroy = NULL;

    // Add the component to the window
    component->add(component->parent, component);

    return(component);
}

// List Component *************************************************************
#define list_item_selected(list)    &list->items[list->selected*list->max_length]

local void listUpdateMethod(Component *base)
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
    winPrint(base->parent,row++,col,"%s---",base->label);

    // Display the rows of items visible
    for(int i = list->top_visible; i < list->size && i < list->top_visible+base->height; i++)
    {
        short color = base->parent->window_color;
        if(base->parent->focus == base && i == list->selected)
            base->parent->window_color = COLOR_FOCUS;
        else
            base->parent->window_color = COLOR_FIELD;
        winPrint(base->parent,row++,col,"%-*s", base->width, &list->items[i*list->max_length]);
        base->parent->window_color = color;
    }

    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the position of the cursor
        base->parent->cur_row = base->row + list->selected - list->top_visible + 1;
        base->parent->cur_col = base->col+strlen(listSelectedItem(list));
    }
}

local int listAction(Component *base, int ch)
{
    List *list = (List *)base->context;

    // If the next selected item is at the end of the list and the list is not too big...
    if(++list->selected == list->size && list->size < list->max_items)
        // Add a blank item to the end of the list
        list->items[list->size++ * list->max_length] = '\0';
        //strncpy(&list->items[list->size++ * list->max_length],"",list->max_length-1);
    // Else if the size of the list is maxxed...
    else if(list->size == list->max_items)
    {
        --list->selected;
        popupError(base->parent->parent, "Max List Items Reached",MESSAGE_DURATION);
    }
    winMarkDestroy(base->parent);
    return(1);
}

local int listInputMethod(Component *base, int ch)
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
            get_string = popupGetString(base->parent->parent, "Enter Item: ", &list->items[list->selected*list->max_length], list->max_length, listAction);
            if(get_string == NULL)
                return(-1);
            // Stash a pointer to this component context for the action handler
            get_string->context = (void *)base;
            break;
        default:
            // Did not consume the input
            return(0);
    }
    // Consumed the input
    winSetStale(base->parent);

    return(1);
}

local void listFocusMethod(Component *base)
{
    List *list = (List *)base;

    //wmove(stdscr,base->row+list->selected-list->top_visible+1,base->col);
    base->parent->cur_row = base->row+list->selected-list->top_visible+1;
    base->parent->cur_col = base->col;
    winSetCursor(base->parent,false);
}

Component *listCreate(Window *win, int row, int col, int height, int width, char *label, char *items, int max_items, int max_length)
{
    // Check input parameters
    if(win == NULL || items == NULL || max_items <= 0 || max_length <= 0)
        return(NULL);

    // Allocate the list
    List *list = malloc(sizeof(List));
    if(list == NULL)
        return(NULL);

    // Create the component
    if((compCreate((Component *)list, win, row, col, height, width, label)) == NULL)
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

    base->update = listUpdateMethod;
    base->input = listInputMethod;
    base->focus = listFocusMethod;

    // Set the focus to this new component
    //winSetFocus(win,base);

    return((Component*)list);
}

// Checkbox Component *********************************************************
local void chkboxUpdateMethod(Component *base)
{
    Checkbox *cb = (Checkbox *)base;

    if(cb->true_string != NULL && cb->false_string != NULL)
        winPrint(base->parent,base->row,base->col,base->format,base->label,*(cb->value)?cb->true_string:cb->false_string);
}

local int chkboxInputMethod(Component *base, int ch)
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
    winSetStale(base->parent);
    return(1);
}

local void chkboxFocusMethod(Component *base)
{
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col+1;
    winSetCursor(base->parent,true);
}

Component *chkboxCreate(Window *win, int row, int col, int width, char *label, bool *value)
{
    // Check input parameters
    if(value == NULL)
        return(NULL);

    // Allocate the checkbox
    Checkbox *checkbox = malloc(sizeof(Checkbox));
    if(checkbox == NULL)
        return(NULL);

    // Create the component
    if((compCreate((Component *)checkbox, win, row, col, 1, width, label)) == NULL)
        return(NULL);
    Component *base = (Component *)checkbox;

    checkbox->value = value;
    checkbox->true_string = "X";
    checkbox->false_string = " ";
    base->format = "%s[%s]";

    base->update = chkboxUpdateMethod;
    base->input = chkboxInputMethod;
    base->focus = chkboxFocusMethod;

    // Set the focus to this new component
    //winSetFocus(win,base);

    return(base);
}

// String Component ***********************************************************
local void strUpdateMethod(Component *base)
{
    String *str = (String *)base;
    short color;

    // Print the label
    winPrint(base->parent,base->row,base->col,"%s",base->label);
    // Set up the color for the field
    color = base->parent->window_color;
    if(base->parent->focus == base)
        base->parent->window_color = COLOR_FOCUS;
    else
        base->parent->window_color = COLOR_FIELD;
    // Print the field
    winPrint(base->parent,base->row,base->col + strlen(base->label),base->format,base->width,str->value);
    // Restore the window color
    base->parent->window_color = color;
    
    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the position of the cursor
        base->parent->cur_row = base->row;
        base->parent->cur_col = base->col+strlen(base->label)+str->cursor_offset;
    }
}

local int strInputMethod(Component *base, int ch)
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
            compNotifyAction(base, ch);
            base->parent->next_focus(base->parent,true);
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
    winSetStale(base->parent);
    return(1);
}

local void strFocusMethod(Component *base)
{
    String *str = (String *)base;
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col+strlen(base->label)+str->cursor_offset;
    str->cursor_offset = strlen(str->value);
    winSetCursor(base->parent,true);
}

Component *strCreate(Window *win, int row, int col, int width, char *label, char *value, int size_max)
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
    if((compCreate(base, win, row, col, 1, width, label)) == NULL)
        return(NULL);

    string->value = value;
    string->size_max = size_max;
    string->cursor_offset = strlen(value);
    base->format = "%-*s";

    base->update = strUpdateMethod;
    base->input = strInputMethod;
    base->focus = strFocusMethod;

    // Set the focus to this new component
    //winSetFocus(win,base);
    //base->focus(base);

    return(base);
}

// Integer Component **********************************************************
local void intUpdateMethod(Component *base)
{
    Integer *integer = (Integer *)base;
    short color;

    // Print the label
    if(base->label != NULL)
        winPrint(base->parent,base->row,base->col,"%s",base->label);
    // Set up the color for the field
    color = base->parent->window_color;
    if(base->parent->focus == base && base->focus != NULL)
        base->parent->window_color = COLOR_FOCUS;
    else if (base->focus != NULL)
        base->parent->window_color = COLOR_FIELD;
    // Print the field
    winPrint(base->parent,base->row,base->col + (base->label!=NULL?strlen(base->label):0),base->format,base->width,*(integer->value));
    // Restore the window color
    base->parent->window_color = color;
 
//    winPrint(base->parent,base->row,base->col,base->format,base->label,*(integer->value));
    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the cursor position
        base->parent->cur_row = base->row;
        base->parent->cur_col = base->col + strlen(base->label) + integer->cursor_offset;
    }
}

local int intInputMethod(Component *base, int ch)
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
            base->parent->next_focus(base->parent,true);
            break;
        default:
            // Did not consume the input
            return (0);
    }

    // Consumed the input
    winSetStale(base->parent);
    return(1);
}

local void intFocusMethod(Component *base)
{
    Integer *integer = (Integer *)base;
    // Update the cursor position
    base->parent->cur_row = base->row;
    base->parent->cur_col = base->col + strlen(base->label) + integer->cursor_offset;
    // Update the string representation and cursor location
    sprintf(integer->field,"%d",*integer->value);
    integer->cursor_offset = strlen(integer->field);
    // Enable the cursor
    winSetCursor(base->parent,true);
}

Component *intCreate(Window *win, int row, int col, int width, char *label, int *value)
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
    if((compCreate(base, win, row, col, 1, width, label)) == NULL)
        return(NULL);

    integer->value = value;
    integer->cursor_offset = snprintf(integer->field,sizeof(integer->field)-1,"%*d",width,*integer->value);
    base->format = "%-*d";

    base->update = intUpdateMethod;
    base->input = intInputMethod;
    base->focus = intFocusMethod;

    // Set the focus to this new component
    //winSetFocus(win,base);

    return(base);
}

// Text Component *************************************************************
local void txtUpdateMethod(Component *base)
{
    if(base == NULL)
        return;

    winPrint(base->parent,base->row,base->col,base->label,NULL);
}

Component *txtCreate(Window *win, int row, int col, const char *text)
{
    // Check input parameters
    if(row < 0 || col < 0 || text == NULL)
        return(NULL);

    // Allocate the integer
    Component *base = malloc(sizeof(Component));
    if(base == NULL)
        return(NULL);

    // Create the component
    if((compCreate(base, win, row, col, 1, strlen(text), text)) == NULL)
        return(NULL);

    base->update = txtUpdateMethod;
    return(base);
}

// Timer Component ************************************************************
local void tmrSetMethod(Timer *timer, unsigned int msecs)
{
    if(timer == NULL)
        return;
    timer->counter = msecs;
}

local void tmrTickMethod(Component *base)
{
    Timer *timer = (Timer *)base;

    if(--timer->counter == 0)
        if(base->notify_action)
            base->notify_action(base, 0); 
}

Component *tmrCreate(Window *win, unsigned int msecs)
{
    if(win == NULL)
        return(NULL);

    Timer *timer = malloc(sizeof(Timer));
    if(timer == NULL)
        return(NULL);

    Component *base = (Component *)timer;
    if((compCreate(base, win, 0, 0, 0, 0, NULL)) == NULL)
        return(NULL);

    base->notify_tick = tmrTickMethod;
    timer->set_timer = tmrSetMethod;
    tmrSet((Timer*)base,msecs);

    return(base);
}

// Frame Component*************************************************************
local void frameUpdateMethod(Component *base)
{
    if(base == NULL)
        return;

    Frame *frame = (Frame *)base;
    Window *win = base->parent;
    int row = base->row;
    int col = base->col;

    // Top of the frame
    win->buffer[(row * win->width) + col++] = frame_symbols[frame->frame_type][TOP_LEFT];
    for(; col < base->width-1+base->col; col++)
        win->buffer[(row * win->width) + col] = frame_symbols[frame->frame_type][HORIZONTAL];
    win->buffer[(row++ * win->width) + col] = frame_symbols[frame->frame_type][TOP_RIGHT];

    // Sides of the frame
    col = base->col;
    for(; row < base->height-1+base->row; row++)
    {
        win->buffer[(row * (win->width)) + col] = frame_symbols[frame->frame_type][VERTICAL];
        win->buffer[(row * (win->width)) + col + base->width - 1] = frame_symbols[frame->frame_type][VERTICAL];
    }

    // Bottom of the frame
    win->buffer[(row*win->width)+col++] = frame_symbols[frame->frame_type][BOTTOM_LEFT];
    for(; col < base->width-1+base->col; col++)
        win->buffer[(row*win->width)+col] = frame_symbols[frame->frame_type][HORIZONTAL];
    win->buffer[(row*win->width)+col] = frame_symbols[frame->frame_type][BOTTOM_RIGHT];

    // Print label
    int offset;
    switch(frame->label_justification)
    {
        case LABEL_LEFT:
            offset = base->col + 2;
            break;
        case LABEL_CENTER:
            offset = ((win->width - strlen(base->label)) / 2) + base->col;
            break;
        case LABEL_RIGHT:
            offset = win->width - strlen(base->label) - 2 + base->col;
            break;
    }
    // Print the label in the top of the frame    
    winPrint(win, base->row, offset, "%s", base->label);
}

Component *frameCreate(Window *win, int row, int col, int height, int width, const char *label)
{
    // Check input parameters
    if(row < 0 || col < 0 || height < 2 || width < 2)
        return(NULL);

    // Allocate the integer
    Component *base = malloc(sizeof(Component));
    if(base == NULL)
        return(NULL);

    // Create the component
    if((compCreate(base, win, row, col, height, width, label)) == NULL)
        return(NULL);

    Frame *frame = (Frame *)base;

    frame->frame_type = FRAME_LIGHT_ARC;
    frame->label_justification = LABEL_CENTER;
    base->label = label;
    base->update = frameUpdateMethod;
    return(base);
}

