/*
 * scrn.c
 *
 * Screen functions of the text windowing UI component of the Linux Console
 * Application Framework (lcaf) library. These functions implement the
 * base screen/window that acts as the interface to the driver/hardware that
 * implement the display and user input.
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

local int scrnInitializeMethod(Window *this)
{
    // Configure ncurses
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color(); 
    
    return(0);
}

local void scrnDestroyMethod(Window *this)
{
    // Turn off ncurses
    endwin();

    // Destroy all the child windows of this screen
    while(this->top_window)
        if(this->top_window->destroy)
            this->top_window->destroy(this->top_window);

    // Free the screen data structure
    free(this->buffer);
    free(this);
}

local void scrnUpdateMethod(Window *this)
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

local int scrnWriteMethod(Window *dest, Window *src)
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

Window *scrnCreate()
{
    Window *screen = NULL;

    // Initialize Ncurses
    initscr();

    // If allocating the screen buffer is successful...
    if((screen = winAllocate(0, 0, LINES, COLS, NULL, false)) != NULL)
    {
        screen->initialize = scrnInitializeMethod;
        screen->update = scrnUpdateMethod;
        screen->write = scrnWriteMethod;
        screen->destroy = scrnDestroyMethod;

        return(screen);
    }

    // Failure
    endwin();
    return(NULL);
}

local void scrnTick(Window *screen)
{
    if(screen == NULL)
        return;

    // Delay for 1ms
    usleep(900);

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

    // If notifying windows and components updated something...
    if(screen->stale)
    {
        // Update the screen
        scrnUpdate(screen);
        // Write the screen to hardware
        scrnWrite(screen);
    }
}

local int scrnResize(Window *screen)
{
    // Save the state of the user handlers
    Initialize_Window initialize = screen->initialize;
    Configure_Window configure = screen->configure;
    void *notify_input = screen->notify_input;
    void *notify_update = screen->notify_update;
    void *notify_destroy = screen->notify_destroy;
    void *notify_focus = screen->notify_focus;
    void *notify_stale = screen->notify_stale;
    void *notify_action = screen->notify_action;
    void *notify_tick = screen->notify_tick;

    // Tear it all down and rebuild for new screen size
    scrnDestroy(screen);
    if((screen = scrnCreate()) < 0)
        return(-1);

    // Restore the state of the user handlers
    screen->initialize = initialize;
    screen->configure = configure;
    screen->notify_action = notify_action;
    screen->notify_input = notify_input;
    screen->notify_update = notify_update;
    screen->notify_destroy = notify_destroy;
    screen->notify_focus = notify_focus;
    screen->notify_stale = notify_stale;
    screen->notify_tick = notify_tick;

    return(winConfigure(screen));
}

void scrnDestroyWindows(Window *screen)
{
    // Scan the windows and destroy the marked ones
    Window *win = screen->bottom_window;
    while(win != NULL)
    {
        Window *next = win->next;
        if(win->mark_destroy)
            if(win->destroy != NULL)
                win->destroy(win);
        win = next;
    }
}

int scrnRun(Window *screen)
{
    if(screen == NULL)
        return(-1);

    // If the screen has an initialize handler...
    winInitialize(screen);
    // If the screen has a configure handler...
    winConfigure(screen);
        
    int result;
    do
    {
        // Scan the windows and destroy the marked ones
        scrnDestroyWindows(screen);
        
        Window *top = screen->top_window;
        // If there is no top window to receive input...
        if(top == NULL)
            return(-1);

        // Update the screen
        scrnUpdate(screen);
        // Write the screen to hardware
        scrnWrite(screen);

        // Get the next key
        nodelay(stdscr,TRUE);
        int ch;
        while((ch = getch())==ERR)
            scrnTick(screen);
        nodelay(stdscr,FALSE);

        // If the screen has been resized...
        if(ch == KEY_RESIZE)
            result = scrnResize(screen);
        else
        {
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

