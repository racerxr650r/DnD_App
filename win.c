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
local void winFrameMethod(Window *this)
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

local void winUpdateMethod(Window *this)
{
    if(this == NULL)
        return;

    // Clear the window
    winClear(this);

    winFrame(this);

    // If this window has a label...
    if(this->label != NULL)
    {
        // Display the window label in the top middle
        int offset;
        switch(this->label_justification)
        {
            case LABEL_LEFT:
                offset = 2;
                break;
            case LABEL_CENTER:
                offset = (this->width - strlen(this->label)) / 2;
                break;
            case LABEL_RIGHT:
                offset = this->width - strlen(this->label) - 2;
                break;
        }
        // Print the label in the top of the frame    
        winPrint(this, 0, offset, "%s", this->label);
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

    // Update any sub-windows
    Window *sub_win = this->bottom_window;
    bool    stale = this->stale;
    while(sub_win != NULL)
    {
        // If this window or a previous window is stale...
        if(sub_win->stale || stale)
        {
            sub_win->update(sub_win);
            sub_win->write(this,sub_win);
            stale = true;
        }
        sub_win = sub_win->prev;
    }

    // If this window has an update notification...
    winNotifyUpdate(this);
}

int winCopy(Window *dest, Window *src)
{
    int copy_count = 0;
    // Determine the copy areas
    int min_row = 0;
    int max_row = src->height;
    if(max_row > dest->height - src->row)
        max_row = dest->height - src->row;

    int max_col = src->width;
    if(max_col > dest->width - src->col)
        max_col = dest->width - src->col;

    // Write the content of src window into the destination window.
    for(int row = min_row; row < max_row; row++)
    {
        int source_pos = row * src->width;
        int dest_pos = ((row + src->row) * dest->width) + src->col;
        memcpy(&dest->buffer[dest_pos], &src->buffer[source_pos], max_col * sizeof(wchar_t));
        memcpy(&dest->attrs[dest_pos], &src->attrs[source_pos], max_col * sizeof(attr_t));
        memcpy(&dest->colors[dest_pos], &src->colors[source_pos], max_col * sizeof(short));
        copy_count += max_col;
    }

    return(copy_count);
}

local int winWriteMethod(Window *win, Window *this)
{
    if (win == NULL || this == NULL || win->buffer == NULL || this->buffer == NULL)
        return 0;

    int copy_count = winCopy(win,this);

    // If this window has a component with focus...
    if(this->focus || isScreen(this->parent))
    {
        // Update the cursor state and location
        win->cursor_type = this->cursor_type;
        win->cur_row = this->cur_row + this->row;
        win->cur_col = this->cur_col + this->col;    
    }

    // Mark the source window fresh
    this->stale = false;
    // Mark the destination window stale
    winSetStale(win);   

    return copy_count;
}

local int winInputMethod(Window *this, int ch)
{
    int result;

    if(this == NULL)
        return(0);

    // The window's component with focus attempts to consume input
    Component *component = this->focus;
    // If this window doesn't currently have a component with focus...
    if(!component)
    {
        Window *win = this->top_window;
        // Scan the sub-windows of a component with focus
        while(win)
        {
            // If this sub-window handled the input...
            if(win->focus)
            {
                component = win->focus;
                break;
            }
            win = win->next;
        }
    }

    // If a component with focus was found...
    if(component)
    {
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
            this->prev_focus(this,true);
            break;
        case '\t':
        case KEY_ENTER:
            this->next_focus(this,true);
            break;
        default:
            // Did not consume the input
            return(0);
    }

    // Mark the window as stale
    winSetStale(this);

    return(1);
}

local int winAddMethod(Window *parent, Window *this)
{
    if(this == NULL || parent == NULL)
        return(-1);

    Window *last = parent->bottom_window;

    // If there is already a window in the list...
    if(last != NULL)
    {
        last->next = this;
        this->prev = last;
        this->next = NULL;
    }
    // Else this is the first window to be added
    else
    {
        parent->top_window = this;
        this->next = NULL;
        this->prev = NULL;
    }

    parent->bottom_window = this;
    winSetStale(this);
    this->parent = parent;

    if(this->notify_focus)
        this->notify_focus(this);

    return(0);
}

local void winInsertMethod(Window *ref, Window *this)
{
    if(this == NULL || ref == NULL)
        return;

    // If the reverence window is not the top window...
    if(ref->prev)
        ref->prev->next = this;
    // Else this is now the top window...
    else
        ref->parent->top_window = this;

    this->prev = ref->prev;
    this->next = ref;
    ref->prev = this;
    this->parent = ref->parent;
    winSetStale(this);
}

local void winRemoveMethod(Window *this)
{
    if(this == NULL || this->parent == NULL)
        return;

    Window *screen = this->parent;

    // If this is the bottom window...
    if(this == screen->bottom_window)
    {
        // If there is more windows in the list...
        if(this->prev)
        {
            this->prev->next = NULL;
            screen->bottom_window = this->prev;
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
        screen->top_window->next->prev = NULL;
        screen->top_window = screen->top_window->next;

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

    winSetStale(screen->bottom_window);
}

local void winSetStaleMethod(Window *this)
{
    if(this == NULL)
        return;

    // Mark the associated screen stale
    if(this->parent)
        this->parent->stale = true;

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

local void winDestroyMethod(Window *this)
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
        compDestroy(curr_component);
    }

    if(this->parent)
        // Mark all the windows as stale
        winSetStale(this->parent->bottom_window);

    // Remove the window from the list
    winRemove(this);
    //this->remove(this);

    // Free the window buffer
    free(this->buffer);
    // Free the window attributes
    free(this->attrs);
    // Free the window colors
    free(this->colors);
    // Free the window data structure
    free(this);
}

local void winSetCursorMethod(Window *this, bool enable)
{
    if(this == NULL)
        return;

    this->cursor_type = enable ? (this->insert_key ? CURSOR_INSERT : CURSOR_OVERWRITE) : CURSOR_NONE;
    winSetStale(this);
}

local App_Status winFirstFocusMethod(Window *this)
{
    if(this == NULL)
        return(APP_INVALID_PARAMETER);

    // If there are components for this window...
    Component *comp = this->component_head;
    while(comp)
    {
        if(winSetFocus(this,comp) == APP_OK)
            return(APP_OK);
        comp = comp->next;
    }
    // Else there are no components for this window...
    Window *win = this->top_window;
    // Iterate through the list of sub-windows from head to tail
    while(win)
    {
        if(winFirstFocus(win) == APP_OK)
            return(APP_OK);
        win = win->next;
    }
    return(APP_NO_ACTION);
}

local App_Status winLastFocusMethod(Window *this)
{
    if (this == NULL)
        return(APP_INVALID_PARAMETER);

    // Try to set focus to the last focusable component in this window
    Component *comp = this->component_tail;
    while (comp)
    {
        if (winSetFocus(this, comp) == APP_OK)
            return (APP_OK);
        comp = comp->prev;
    }

    // If no component in this window took focus, try sub-windows in reverse order
    Window *win = this->bottom_window;
    while (win)
    {
        if (winLastFocusMethod(win) == APP_OK) // Recursive call
            return (APP_OK);
        win = win->prev;
    }
    return(APP_NO_ACTION);
}

local App_Status winSetFocusMethod(Window *this, Component *component)
{
    if(this == NULL || component == NULL || component->focus == NULL)
        return(APP_INVALID_PARAMETER);

    // Check if this component is a valid member of this window...
    Component *curr_comp = this->component_head;
    while(curr_comp)
    {
        if(curr_comp == component)
            goto VALID_COMPONENT;
        curr_comp = curr_comp->next;
    }
    return(APP_NO_ACTION);

    // Confirmed that the component is valid
    VALID_COMPONENT:    

    // Set the window focus
    this->focus = component;

    // Set the component focus
    compSetFocus(component);
    compNotifyFocus(component);

    // Mark the window to be redrawn
    winSetStale(this);
    return(APP_OK);
}

local App_Status winNextFocusMethod(Window *this, bool top_window)
{
    if(this == NULL)
        return(APP_INVALID_PARAMETER);

    // Does this window currently have the focus?
    Component *curr = this->focus;
    if(curr != NULL)
    {
        // While there is a next component in the list...
        while(curr = curr->next)
        {
            // If this component can handle focus...
            if(curr->focus)
            {
                winSetFocus(this,curr);
                return(APP_OK);
            }
        }
        // Ran past the last component for this window
        this->focus = NULL;
        if(top_window)
        {
            Window *win = this->top_window;
            // Iterate through the list of sub-windows from head to tail
            while(win)
            {
                // If that window set the focus...
                if(winFirstFocus(win) == APP_OK)
                    return(APP_OK);
                win = win->next;
            }
            // Ran past the last sub-window
            return(winFirstFocus(this));
        }
        return(APP_OVERFLOW);
    }

    // This window does not currently have the focus
    Window *win = this->top_window;
    // Does one of the sub-windows have focus?
    // Iterate through the list of sub-windows from head to tail
    while(win)
    {
        App_Status status = win->next_focus(win,false);
        // If that window set the focus...
        if(status == APP_OK)
            return(APP_OK);
        // Else if that window just overflowed the focus...
        else if(status == APP_OVERFLOW)
        {
            win = win->next;
            // If there is another sub-window...
            if(win)
            {
                // While there is another sub-window...
                while(win)
                {
                    // If that sub-window set the focus...
                    if(winFirstFocus(win) == APP_OK)
                        return(APP_OK);
                    win = win->next;
                }
            }
            // Else that was the last of the sub-windows
            // so set the first focus of this window
            // If this is the top window setting next focus...
            if(top_window)
                return(winFirstFocus(this));
            // Else return to a parent window setting next focus...
            return(APP_OVERFLOW);
        }
        win = win->next;
    }
    return(APP_NO_ACTION);
}

local App_Status winPrevFocusMethod(Window *this, bool top_window)
{
    if(this == NULL)
        return(APP_INVALID_PARAMETER);

    // Does this window currently have the focus?
    Component *curr = this->focus;
    if(curr != NULL)
    {
        // While there is a prev component in the list...
        while(curr = curr->prev)
        {
            // If this component can handle focus...
            if(curr->focus)
            {
                winSetFocus(this,curr);
                return(APP_OK);
            }
        }
        // Ran past the first component for this window
        this->focus = NULL;
        if(top_window)
        {
            Window *win = this->bottom_window;
            // Iterate through the list of sub-windows from tail to head
            while(win)
            {
                // If that window set the focus...
                if(winLastFocus(win) == APP_OK)
                    return(APP_OK);
                win = win->prev;
            }
            // Ran past the first sub-window
            return(winLastFocus(this));
        }
        return(APP_OVERFLOW);
    }

    // This window does not currently have the focus
    Window *win = this->top_window;
    // Does one of the sub-windows have focus?
    // Iterate through the list of sub-windows from top tp bottom
    while(win)
    {
        App_Status status = win->prev_focus(win,false);
        // If that window set the focus...
        if(status == APP_OK)
            return(APP_OK);
        // Else if that window just overflowed the focus...
        else if(status == APP_OVERFLOW)
        {
            win = win->prev;
            // If there is another sub-window...
            if(win)
            {
                // While there is another sub-window...
                while(win)
                {
                    // If that sub-window set the focus...
                    if(winLastFocus(win) == APP_OK)
                        return(APP_OK);
                    win = win->prev;
                }
            }
            // Else that was the last of the sub-windows
            // so set the first focus of this window
            // If this is the top window setting next focus...
            if(top_window)
                return(winLastFocus(this));
            // Else return to a parent window setting next focus...
            return(APP_OVERFLOW);
        }
        win = win->next;
    }
    return(APP_NO_ACTION);
}

local void winMoveTopMethod(Window *this)
{
    if(this == NULL || this == this->parent->top_window)
        return;

    this->remove(this);
    this->insert(this->parent->top_window,this);
    winSetStale(this);
}

local void winMoveBottomMethod(Window *this)
{
    if(this == NULL || this == this->parent->bottom_window)
        return;

    this->remove(this);
    this->add(this->parent,this);
    winSetStale(this);
}

local void winMoveUpMethod(Window *this)
{
    if(this == NULL || this == this->parent->top_window)
        return;

    this->remove(this);
    // If the next window is not the top window...
    if(this->next->next)
        this->insert(this->next->next,this);
    // Else the next window is the top window...
    else
        this->add(this->parent,this);

    winSetStale(this);
}

local void winMoveDownMethod(Window *this)
{
    if(this == NULL || this == this->parent->bottom_window)
        return;

    this->remove(this);
    this->insert(this->prev,this);
    winSetStale(this);
}

local void winClearMethod(Window *this)
{
    if(this == NULL)
        return;

    // Clear the window
    for(int i=0;i<(this->height*this->width);i++)
        this->buffer[i] = L' ';
}

// Function to convert a single-byte char to a wide character
wchar_t charToWchar(char c) 
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

local int winPrintMethod(Window *this, int row, int col, const char *format, va_list args)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;
    
    va_list args2;
    va_copy(args2, args);

    // Allocate memory for the output string
    int formatted_string_size = vsnprintf(NULL,0,format,args);
    if(formatted_string_size <= 0)
        return(0);
    char *formatted_string = malloc((formatted_string_size + 1) * sizeof(char));
    if(formatted_string == NULL)
        return(0);

    vsnprintf(formatted_string,formatted_string_size+1,format,args2);
    va_end(args2);

    // Create a temporary formatted string
    //int result = vasprintf(&formatted_string, format, args);
    //if(result == -1 || formatted_string == NULL)
    //    return(0);
    //formatted_string_size = result;

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
            this->buffer[pos] = charToWchar(c);
            this->attrs[pos] = this->window_attr;
            this->colors[pos] = this->window_color;
            current_col++;
            total_written++;
        }
    }

    // If the window was written to...
    if(total_written > 0)
        winSetStale(this);

    //Free the formatted string
    free(formatted_string);
    return total_written;
}

local int winWprintMethod(Window *this, int row, int col, const wchar_t *format, va_list args)
{
    if (this == NULL || format == NULL || this->buffer == NULL)
        return 0;

    // Create a temporary formatted wide string
    wchar_t *formatted_string = NULL;
    int     formatted_string_size = 64535;

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
            this->attrs[pos] = this->window_attr;
            this->colors[pos] = this->window_color;
            current_col++;
            total_written++;
        }
    }

    // If the window was written to...
    if(total_written > 0)
        winSetStale(this);

    //Free the formatted string
    free(formatted_string);
    return total_written;
}

Window *winAllocate(int row, int col, int height, int width, char *label, bool box)
{
    Window *window = malloc(sizeof(Window));
    if(window == NULL)
        return(NULL);

    // Set window configuration
    window->window_attr = WA_NORMAL;
    window->window_color = COLOR_DEFAULT;
    window->row = row;
    window->col = col;
    window->height = height;
    window->width = width;
    window->cur_row = 0;
    window->cur_col = 0;
    window->cursor_type = CURSOR_NONE;
    window->label = label;
    window->label_justification = LABEL_CENTER;
    window->focus = NULL;
    window->insert_key = true;
    window->wrap = false;
    window->stale = true;
    window->mark_destroy = false;
    window->frame_type = FRAME_NONE;
    window->parent = NULL;

    window->initialize = NULL;
    window->configure = NULL;
    window->frame = winFrameMethod;
    window->update = winUpdateMethod;
    window->write = winWriteMethod;
    window->input = winInputMethod;
    window->destroy = winDestroyMethod;

    window->add = winAddMethod;
    window->insert = winInsertMethod;
    window->remove = winRemoveMethod;

    window->set_stale = winSetStaleMethod;

    window->set_cursor = winSetCursorMethod;

    window->first_focus = winFirstFocusMethod;
    window->last_focus = winLastFocusMethod;
    window->set_focus = winSetFocusMethod;
    window->next_focus = winNextFocusMethod;
    window->prev_focus = winPrevFocusMethod;

    window->move_top = winMoveTopMethod;
    window->move_bottom = winMoveBottomMethod;
    window->move_up = winMoveUpMethod;
    window->move_down = winMoveDownMethod;

    window->clear_window = winClearMethod;

    window->print = winPrintMethod;
    window->wprint = winWprintMethod;
    
    window->notify_action = NULL;
    window->notify_input = NULL;
    window->notify_tick = NULL;
    window->notify_stale = NULL;
    window->notify_focus = NULL;
    window->notify_update = NULL;
    window->notify_destroy = NULL;

    window->top_window = NULL;
    window->bottom_window = NULL;
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
        
        window->attrs = malloc((height * width)*sizeof(attr_t));
        if(window->attrs == NULL)
        {
            free(window->buffer);
            free(window);
            return(NULL);
        }

        window->colors = malloc((height * width)*sizeof(short));
        if(window->colors == NULL)
        {
            free(window->attrs);
            free(window->buffer);
            free(window);
            return(NULL);
        }

        int i;
        for(i=0;i<(height*width);++i)
        {
            window->buffer[i] = L' ';
            window->attrs[i] = window->window_attr;
            window->colors[i] = window->window_color;
        }
        window->buffer[i] = L'\0';
    }
    else
        window->buffer = NULL;

    return(window);
}

Window *winCreate(Window *screen, int row, int col, int height, int width, char *label, bool box)
{
    Window *window = winAllocate(row, col, height, width, label, box);

    // If the window was successfully allocated and configured...
    if(window != NULL)
        window->add(screen, window);

    return(window);
}
