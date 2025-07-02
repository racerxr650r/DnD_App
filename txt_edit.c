/*
 * txt_edit.c
 *
 * Text editor component of the text windowing UI section of the Linux Console
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

// Local Macros ---------------------------------------------------------------
#define PREVIOUS_LINE_PTR(editor)       (wchar_t *)(editor->line[(editor->cursor_row-1>=0)?(editor->cursor_row-1):0])
#define CURRENT_LINE_PTR(editor)        (wchar_t *)(editor->line[editor->cursor_row])
#define NEXT_LINE_PTR(editor)           (wchar_t *)(editor->line[(editor->cursor_row+1<editor->rows)?editor->cursor_row+1:editor->rows])
#define CURRENT_BUFFER_PTR(editor)      (wchar_t *)(CURRENT_LINE_PTR(editor)+editor->cursor_col)
#define CURRENT_BUFFER_OFFSET(editor)   (int)(CURRENT_BUFFER_PTR(editor) - editor->buffer)
#define START_BUFFER_PTR(editor)        (wchar_t *)&editor->buffer[editor->select_start]
#define END_BUFFER_PTR(editor)          (wchar_t *)&editor->buffer[editor->select_end]
#define PREVIOUS_CHARACTER(editor)      (wchar_t)((editor->cursor_col == 0 && editor->cursor_row == 0)?L'\0':CURRENT_BUFFER_PTR(editor)[-1])
#define CURRENT_CHARACTER(editor)       (wchar_t)(CURRENT_BUFFER_PTR(editor)[0])
#define NEXT_CHARACTER(editor)          (wchar_t)((editor->cursor_row == editor->rows-1 && editor->cursor_col == line_length(editor, editor->cursor_row)-1)?L'\0':CURRENT_BUFFER_PTR(editor)[1])

// Local Functions ------------------------------------------------------------
/*local int texteditLineNumber(Text_Editor *editor)
{
    for(int i = 1; i < editor->rows; i++)
        if(editor->line[i] > CURRENT_BUFFER_PTR(editor))
            return(i-1);
    return(editor->rows - 1);
}*/

local int texteditLineLen(Text_Editor *editor, int line)
{
    if(line < 0 || line >= editor->rows)
        return(0);
    
    for(int i = 0; i < editor->cols; i++)
        if(editor->line[line][i] == L'\n' || editor->line[line][i] == L'\0')
            return(i);

    return(editor->cols);
}

local int txteditSetCursor(Text_Editor *editor, int row, int col)
{   
    Component *base = (Component *)editor;

    // If out of bounds...
    if(editor == NULL || row < 0 || row > editor->rows || col < 0 || col > editor->cols)
        return(-1);

    // If the new column is beyond the new line...
    if(col>texteditLineLen(editor,row))
        col = texteditLineLen(editor,row);
    
    // If the same as the current location...
    if(editor->cursor_row == row && editor->cursor_col == col)
        return(0);

    editor->cursor_row = row;
    editor->cursor_col = col;

    // If the cursor is left or right of the visible window...
    if(editor->cursor_col < editor->left_visible)
        editor->left_visible = editor->cursor_col;
    else if(editor->cursor_col - editor->left_visible >= base->window->width)
        editor->left_visible = editor->cursor_col - base->window->width + 1;

    // If the cursor is above or below the visible window...
    if(editor->cursor_row < editor->top_visible)
        editor->top_visible = editor->cursor_row;
    else if(editor->cursor_row - editor->top_visible >= base->window->height)
        editor->top_visible = editor->cursor_row - base->window->height + 1;

    return(1);
}

local int txteditSetCursorPtr(Text_Editor *editor, wchar_t *ptr)
{
    if(editor == NULL || ptr == NULL || ptr < editor->buffer || ptr > editor->buffer + editor->buffer_size)
        return(-1);

    int row = 0;
    int col =0;

    if(ptr > editor->buffer)
    {
        int i; 
        for(i = 0; i < editor->rows && editor->line[i] <= ptr; ++i);
        row = --i;
        wchar_t *curr_line_ptr = editor->line[i];
        col = ptr - curr_line_ptr;
    }

    return(txteditSetCursor(editor,row,col));
}

local void txteditInsertText(Text_Editor *editor, wchar_t *text)
{
    if(editor == NULL || text == NULL)
        return;

    // Determine the length of the text to be inserted
    int length = wcslen(text);
    if(length == 0)
        return;

    // If the length will exceed the buffer size...
    if(wcslen(editor->buffer) + length > editor->buffer_size -1)
        length = editor->buffer_size - wcslen(editor->buffer) - 1;

    // If there is room for some or all of the new text...
    if(length > 0)
    {
        wchar_t *start = CURRENT_BUFFER_PTR(editor);
        wchar_t *end = CURRENT_BUFFER_PTR(editor) + length;
        
        // Make room and insert the text
        memmove(end,start,(wcslen(start)+1)*sizeof(wchar_t));
        memcpy(start,text,length*sizeof(wchar_t));

        // Rebuild the line array because no telling what we just did
        txteditConfigure(editor);
        txteditSetCursorPtr(editor,end);
    }
}

local void txteditReplaceText(Text_Editor *editor, wchar_t *text)
{
    if(editor == NULL || text == NULL)
        return;

    // Determine the length of the text to be replaced
    int length = wcslen(text);
    if(length == 0)
        return;

    // If the length will exceed the buffer size...
    if(CURRENT_BUFFER_PTR(editor) + length > editor->buffer + editor->buffer_size -1)
        length = length - ((CURRENT_BUFFER_PTR(editor) + length) - &editor->buffer[editor->buffer_size-1]);
    
    wchar_t *start = START_BUFFER_PTR(editor);
    wchar_t *end = END_BUFFER_PTR(editor);
    // If there is room for some or all of the new text...
    if(length > 0)
    {
        memcpy(start,text,length*sizeof(wchar_t));

        // Rebuild the lines array because no telling what we just did
        txteditConfigure(editor);
        txteditSetCursorPtr(editor,end);
    }

    // Restore the selected area to none
    editor->select_start = -1;
    editor->select_end = -1;
}

local void txteditDeleteText(Text_Editor *editor)
{
    if(editor == NULL || editor->select_start == -1 || editor->select_end == -1)
        return;

    wchar_t *start;
    wchar_t *end;

    // If end is less than start...
    if(END_BUFFER_PTR(editor) < START_BUFFER_PTR(editor))
    {
        start = END_BUFFER_PTR(editor);
        end = START_BUFFER_PTR(editor)+1;
    }
    else
    {
        start = START_BUFFER_PTR(editor);
        end = END_BUFFER_PTR(editor)+1;
    }
    
    int length = wcslen(end)+1;
    memmove(start,end,length*sizeof(wchar_t));

    // Rebuild the lines array because no telling what we just did
    txteditConfigure(editor);
    txteditSetCursorPtr(editor,start);

    // Restore the selected area to none
    editor->select_start = -1;
    editor->select_end = -1;

    Component *base = (Component *)editor;
    winSetStale(base->parent);
}

//select_text(Text_Editor *editor, int row, int col, bool new);
//deselect_text(Text_Editor *editor);

// Text Edit Component Methods ------------------------------------------------
local void txteditUpdateMethod(Component *base)
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
        // Update the position of the cursor in the parent window
        base->parent->cur_row = base->window->cur_row + base->window->row;
        base->parent->cur_col = base->window->cur_col + base->window->col;
    }

    // Wrte the component window to the parent window of the component
    winWrite(base->parent,base->window);

    winPrint(base->parent,base->parent->height-1, 2, "Row-%d/%d Col-%d/%d Line-%d Buffer-%d",editor->cursor_row,editor->rows,editor->cursor_col,editor->cols,texteditLineLen(editor,editor->cursor_row),wcslen(editor->buffer));
}

local int txteditInputMethod(Component *base, int ch)
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

local void txteditFocusMethod(Component *base)
{
    Text_Editor *editor = (Text_Editor *)base;
    // Set the cursor for the component window
    base->window->cur_col = editor->cursor_col - editor->left_visible;
    base->window->cur_row = editor->cursor_row - editor->top_visible;
    // Set the cursor for the parent window
    base->parent->cur_row = base->window->cur_row + base->window->row;
    base->parent->cur_col = base->window->cur_col + base->window->col;
    winSetCursor(base->parent,true);
}

local void texteditConfigureMethod(Component *base)
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

local void texteditDestroyMethod(Component *base)
{
    Text_Editor *editor = (Text_Editor *)base;
    if(base->window->destroy)
        base->window->destroy(base->window);
    if(editor->line != NULL)
        free(editor->line);
}

// Text Edit Component User Functions -----------------------------------------
Component *txteditCreate(Window *win, int row, int col, int height, int width, wchar_t *buffer, int buffer_size)
{
    // Check input parameters
    if(row < 0 || col < 0 || height <= 0 || width <= 0 || buffer == NULL || buffer_size <= 0)
        return(NULL);

    // Allocate the integer
    Text_Editor *editor = malloc(sizeof(Text_Editor));
    if(editor == NULL)
        return(NULL);
    Component *base = (Component *)editor;

    // Create the component
    if((compCreate(base, win, row, col, height, width, NULL)) == NULL)
        return(NULL);
    
    Window *comp_window = winAllocate(row,col,height,width,NULL,false);
    if(win == NULL)
        return(NULL);
                    
    base->window = comp_window;
    editor->buffer = buffer;
    editor->buffer_size = buffer_size;
    editor->cursor_row = 0;
    editor->cursor_col = 0;
    editor->height = height;
    editor->width = width;
    editor->select_start = -1;
    editor->select_end = -1;
    editor->top_visible = 0;
    editor->left_visible = 0;
    editor->line = NULL;

    editor->wrap = false;
    editor->tab_width = 4;

    base->configure = texteditConfigureMethod;
    base->update = txteditUpdateMethod;
    base->input = txteditInputMethod;
    base->focus = txteditFocusMethod;
    base->notify_destroy = texteditDestroyMethod;

    // Configure the line array and other members according to the
    // contents of the buffer
    compConfigure(base);

    // Set the focus to this new component
    winSetFocus(win,base);

    return(base);
}
