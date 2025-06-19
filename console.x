/*
 * console.c
 *
 * Console component of the text windowing UI section of the Linux Console
 * Application Framework (lcaf) library
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
#define DEFAULT_DEPTH       20
#define DEFAULT_PROMPT      "> "

// Console Component Methods --------------------------------------------------
local void consoleUpdateMethod(Component *base)
{
    Text_Editor *editor = (Text_Editor *)base;

    // Clear the window
    winClear(base->window);

    // Draw the text window
    for(int i = 0; i < editor->rows; i++)
        winWprint(base->window, i, 0, L"%ls", editor->line[i] + editor->left_visible);

    // If this component has focus...
    if(base->parent->focus == base)
    {
        // Update the position of the cursor in the component window
        base->window->cur_row = editor->cursor_row - editor->top_visible;
        base->window->cur_col = editor->cursor_col - editor->left_visible;
    }

    // Wrte the component window to the parent window of the component
    winWrite(base->parent,base->window);

    winPrint(base->parent,base->parent->height-1, 2, "Row-%d/%d Col-%d/%d Line-%d Buffer-%d",editor->cursor_row,editor->rows,editor->cursor_col,editor->cols,texteditLineLen(editor,editor->cursor_row),wcslen(editor->buffer));
}

local int consoleInputMethod(Component *base, int ch)
{
    Text_Editor *editor = (Text_Editor *)base;

    switch(ch)
    {
        case KEY_HOME:
            // Move the cursor to the first column and pan the window
            txteditSetCursor(editor,editor->cursor_row,0);
            break;
        case KEY_END:
            // Move the cursor to the end of the line
            txteditSetCursor(editor,editor->cursor_row,texteditLineLen(editor,editor->cursor_row));
            break;
        case KEY_LEFT:
            // If there is room the move left...
            if(editor->cursor_col > 0)
                txteditSetCursor(editor,editor->cursor_row,editor->cursor_col-1);
            // Else if there is room to move up...
            else if(editor->cursor_row > 0)
                txteditSetCursor(editor,editor->cursor_row-1,texteditLineLen(editor,editor->cursor_row-1));
            break;
        case KEY_RIGHT:
            // If there is room to move right...
            if(editor->cursor_col < texteditLineLen(editor,editor->cursor_row))
                txteditSetCursor(editor,editor->cursor_row,editor->cursor_col+1);
            // Else if the is room to move down...
            else if(editor->cursor_row < editor->rows-1)
                txteditSetCursor(editor,editor->cursor_row+1,0);
            break;
        case KEY_UP:
            // If there is room to move up...
            if(editor->cursor_row > 0)
                txteditSetCursor(editor,editor->cursor_row-1,editor->cursor_col);
            break;
        case KEY_DOWN:
            // If there is room to move down...
            if(editor->cursor_row < editor->rows-1)
                txteditSetCursor(editor,editor->cursor_row+1,editor->cursor_col);
            break;
        case KEY_BACKSPACE:
            // If nothing is currently selected...
            if(editor->select_start == -1 && editor->select_end == -1)
            {
                if(editor->buffer < CURRENT_BUFFER_PTR(editor))
                    editor->select_start = editor->select_end = CURRENT_BUFFER_OFFSET(editor)-1;
                else
                    break;
            }
            txteditDeleteText(editor);
            break;
        case KEY_DC:
            // If nothing is currently selected...
            if(editor->select_start == -1 && editor->select_end == -1)
                editor->select_start = editor->select_end = CURRENT_BUFFER_OFFSET(editor);
            txteditDeleteText(editor);
            break;
        case '\n':
        case KEY_ENTER:
            if(base->parent->insert_key)
                txteditInsertText(editor,L"\n");
            else
                txteditReplaceText(editor,L"\n");
            break;
        case KEY_ESC:
            break;
        default:
            // If this is a printable character...
            if(isprint(ch))
            {
                wchar_t key_character[] = {charToWchar(ch), L'\0'};
                // If insert is enabled...
                if(base->parent->insert_key)
                    txteditInsertText(editor,key_character);
                // Else replace is enabled...
                else
                    txteditReplaceText(editor,key_character);
            }
            else
                // Did not consume the input
                return(0);
    }
    // Consumed the input
    winSetStale(base->parent);
    return(1);
}

local void consoleFocusMethod(Component *base)
{
    Text_Editor *editor = (Text_Editor *)base;
    // Set the cursor for the component window
    base->window->cur_col = editor->cursor_col - editor->left_visible;
    base->window->cur_row = editor->cursor_row - editor->top_visible;
    winSetCursor(base->window,true);
}

local void consoleConfigureMethod(Component *base)
{
    if(base == NULL)
        return;

    Text_Editor *editor = (Text_Editor *)base;

    editor->rows = 0;
    editor->cols = 0;

    // If there is an existing array of lines...
    if(editor->line)
        free(editor->line);

    editor->line = malloc(sizeof(wchar_t *));
    editor->line[0] = editor->buffer;
    
    int i = 0;
    int column = 0;
    while(i < editor->buffer_size && editor->buffer[i] != L'\0')
    {
        // If newline...
        if(editor->buffer[i] == L'\n')
        {
            editor->line = realloc(editor->line,sizeof(wchar_t *)*(editor->rows+1));
            editor->line[editor->rows++] = &editor->buffer[i+1];
            column = 0;
        }
        // Else if this is not a carriage return...
        else if (iswprint(editor->buffer[i]))
        {
            //if(column == 0)
            //    editor->line[editor->rows] = &editor->buffer[i];

            if(!editor->rows)
                editor->rows = 1;
            ++column;
            
            if(column > editor->cols)
                editor->cols = column;
        }
        ++i;
    }
    // If we have reached the end of the buffer...
    if(i == editor->buffer_size)
        editor->buffer[i-1] = L'\0';
}

local void consoleDestroyMethod(Component *base)
{
    Console *console = (Console *)base;
    if(base->window->destroy)
        base->window->destroy(base->window);
    free(console->buffer);
    free(console);
}

// Text Edit Component User Functions -----------------------------------------
Component *consoleCreate(Window *win, int row, int col, int height, int width, int buffer_lines, wchar_t *command, int command_size)
{
    // Check input parameters
    if(row < 0 || col < 0 || height <= 0 || width <= 0 || command == NULL || command_size <= 0)
        return(NULL);

    // Allocate the console data structure
    Console *console = malloc(sizeof(Console));
    if(console == NULL)
        return(NULL);
    Component *base = (Component *)console;

    // Create the component
    if((compCreate(base, win, row, col, height, width, NULL)) == NULL)
    {
        free(console);
        return(NULL);
    }
    
    // Allocate the window
    Window *comp_window = winAllocate(row,col,height,width,NULL,false);
    if(win == NULL)
    {
        free(console); 
        return(NULL);
    }

    // Allocate the buffer for the console
    console->buffer = malloc((sizeof(wchar_t) * buffer_lines) + sizeof(wchar_t));
    if(console->buffer == NULL)
    {
        if(base->window->destroy)
            base->window->destroy(base->window);
        free(console);
        return(NULL);
    }
                    
    base->window = comp_window;
    console->prompt = DEFAULT_PROMPT;
    console->command = command;
    console->command_size = command_size;
    console->cursor_offset = 0;
    console->history_depth = DEFAULT_DEPTH;

    base->configure = consoleConfigureMethod;
    base->update = consoleUpdateMethod;
    base->input = consoleInputMethod;
    base->focus = consoleFocusMethod;
    base->notify_destroy = consoleDestroyMethod;

    // Configure the line array and other members according to the
    // contents of the buffer
    compConfigure(base);

    // Set the focus to this new component
    winSetFocus(win,base);

    return(base);
}