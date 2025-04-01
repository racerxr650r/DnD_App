/*
 * win.c
 *
 * Window functions of the text windowing UI component of the Linux Console
 * Application Framework (lcaf) library. These functions implement the
 * text windows
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

// Constants ******************************************************************
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

// Function Definitions *******************************************************
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
    clear_window(this);

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
        Component *curr_component = component;
        component = component->next;
        destroy_component(curr_component);
    }

    if(this->screen)
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
                set_focus_window(this,curr);
                //this->focus = curr;
                //curr->focus(curr);
                //set_stale_window(this);
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
            set_focus_window(this,curr);
            //this->focus = curr;
            //curr->focus(curr);
            //set_stale_window(this);
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
                set_focus_window(this,curr);
                //this->focus = curr;
                //curr->focus(curr);
                //set_stale_window(this);
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
            set_focus_window(this,curr);
            //this->focus = curr;
            //curr->focus(curr);
            //set_stale_window(this);
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

void clear_window_method(Window *this)
{
    if(this == NULL)
        return;

    // Clear the window
    for(int i=0;i<(this->height*this->width);i++)
        this->buffer[i] = L' ';
}

// Function to convert a single-byte char to a wide character
wchar_t char_to_wchar(char c) 
{
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

int wprint_window_method(Window *this, int row, int col, const wchar_t *format, va_list args)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    // Create a temporary formatted wide string
    wchar_t *formatted_string = NULL;
    int     formatted_string_size = 64535;
    /*va_list args_copy;
    va_copy(args_copy, args);
    int formatted_string_size = vswprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (formatted_string_size < 0) {
        return 0; // Error in formatting
    }*/

    // Allocate memory for the formatted string
    formatted_string = (wchar_t *)malloc((formatted_string_size + 1) * sizeof(wchar_t));
    if (formatted_string == NULL) {
        return 0; // Memory allocation error
    }
    int length = vswprintf(formatted_string, formatted_string_size + 1, format, args);

    int total_written = 0;
    int current_row = row;
    int current_col = col;
    for (int i = 0; i < length; i++)
    {
        wchar_t c = formatted_string[i];

        if(c == L'\n')
        {
            current_row++;
            current_col = col;
        }
        else if(c == L'\r')
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
            this->buffer[pos] = c;
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
    window->clear_window = clear_window_method;
    window->print = print_window_method;
    window->wprint = wprint_window_method;
    
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

    // Create the window buffer
    if(height != 0 || width != 0)
    {
        window->buffer = malloc(((height * width)+1)*sizeof(wchar_t));
        if(window->buffer == NULL)
        {
            free(window);
            return(NULL);
        }
        memset(window->buffer, L' ', (height * width)*sizeof(wchar_t));
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
