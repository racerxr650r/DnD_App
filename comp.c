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

void destroy_component_method(Component *component)
{
    // Remove the component from the window
    component->remove(component->parent, component);
    
    if(component->window != NULL)
        destroy_window(component->window);

    notify_destroy_component(component);

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
    
    component->add = add_component;
    component->remove = remove_component;
    component->destroy = destroy_component_method;
    
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

// List Component *************************************************************
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

// Checkbox Component *********************************************************
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

// String Component ***********************************************************
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

// Integer Component **********************************************************
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

// Text Component *************************************************************
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

// Timer Component ************************************************************
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
