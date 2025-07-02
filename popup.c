/*
 * popup.c
 *
 * Generic pop up windows of the text windowing UI component of the Linux
 * Console Application Framework (lcaf) library
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

// Local Functions ------------------------------------------------------------
local int popupAction(Component *base, int ch)
{
    winMarkDestroy(base->parent);
    //winSetStale(base->parent->screen);
    return(0);
}
void popupError(Window *reference, const char *error_message, int milliseconds) 
{
    Window *screen = scrnGet(reference);
    if(screen == NULL)
        return;

    int width = strlen(error_message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win;
    if(win = winCreate(screen, start_y, start_x, height, width, "Error", false))
    {
        win->frame_type = FRAME_LIGHT_ARC;
        txtCreate(win,1,1,error_message);
        Component *base;
        if(base = tmrCreate(win,milliseconds))
        {
            base->notify_action = popupAction;
            winMoveTop(win);
        }
        else
        {
            winMarkDestroy(win);
            win = NULL;
        }
    }
}

// Pop Up Window User Functions -----------------------------------------------
void popupMessage(Window *reference, const char *message, int milliseconds) 
{
    Window *screen = scrnGet(reference);
    if(screen == NULL)
        return;

    int width = strlen(message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win;
    if(win = winCreate(screen, start_y, start_x, height, width, NULL, false))
    {
        win->frame_type = FRAME_LIGHT_ARC;
        txtCreate(win,1,1,message);
        Component *base;
        if(base = tmrCreate(win,milliseconds))
        {
            base->notify_action = popupAction;
            winMoveTop(win);
        }
        else
        {
            winMarkDestroy(win);
            win = NULL;
        }
    }
}

Window *popupYesNo(Window *reference, const char *message, Input_Window yes_no_input)
{
    Window *screen = scrnGet(reference);
    if(screen == NULL)
        return(NULL);

    int width = strlen(message) + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win;
    Component *base;
    if(win = winCreate(screen, start_y, start_x, height, width, NULL, false))
    {
        win->frame_type = FRAME_LIGHT_ARC;
        if(base = txtCreate(win,1,1,message))
        {
            win->input = yes_no_input;
            winMoveTop(win);
        }
        else
        {
            winMarkDestroy(win);
            win = NULL;
        }
    }
    return(win);
}

Component *popupGetString(Window *reference, char * label, char *value, int length, Input_Component handler)
{
    Window *screen = scrnGet(reference);
    if(screen == NULL)
        return(NULL);

    // --- Create popup window ---
    int width = strlen(label) + length + 2; // Add padding for the border
    int height = 3;
    int start_y = (screen->height - height) / 2;
    int start_x = (screen->width - width) / 2;

    Window *win = winCreate(screen, start_y, start_x, height, width, NULL, false);
    if(win == NULL)
        return(NULL);
    win->frame_type = FRAME_LIGHT_ARC;
    winMoveTop(win);

    Component *base = strCreate(win,1,1,20,label,value,length);
    if(base == NULL)
        return(NULL);
    base->notify_action = handler;

    winFirstFocus(win);
    return(base);
}
