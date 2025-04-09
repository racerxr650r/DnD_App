/*
 * hal.c
 *
 * Hardware abstraction layer used by scrn.c to get user input (keyboard and
 * mouse) and write to the display device. 
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
#include <ncursesw/curses.h>
#include <sys/ioctl.h>  // For ioctl()
#include <termios.h>  // For struct winsize

int halGetDisplaySize(int *rows, int *cols)
{
    struct winsize ws;

    // If stdin is not a terminal...
    if (isatty(STDIN_FILENO) == 0)
      return -1;
  
    // If ioctl call TIOCGWINSZ failed...
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
      return -1;
  
    *rows = ws.ws_row;
    *cols = ws.ws_col;
  
    return 0;
}


int halInitialize(void)
{
    // Initialize Ncurses
    initscr();

    // Configure ncurses
    raw();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr,TRUE);
    start_color();

    //init_color(16,200,200,200);
    init_pair(COLOR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_FIELD, COLOR_WHITE, 234);
    init_pair(COLOR_FOCUS, COLOR_BLACK, COLOR_WHITE);

    return(0);
}

void halShutDown(void)
{
    // Turn off ncurses
    endwin();
}

int halGetInput(void)
{
    return(getch());
}

void halShowCursor(bool enable)
{
    if(enable)
        curs_set(1);
    else
        curs_set(0);
}

int halSetCursorType(Cursor_Type type)
{
    int ret;

    // Set the cursor type
    switch (type)
    {
        case CURSOR_INSERT:
            halShowCursor(true); // Show the cursor
            // Set to beem cursor
            printf("\033[5 q"); // Set to beem cursor
            fflush(stdout);
            ret = 1;
            break;
        case CURSOR_OVERWRITE:
            halShowCursor(true); // Show the cursor
            // Set to block cursor
            printf("\033[1 q"); // Set to block cursor
            fflush(stdout);
            ret = 1;
            break;
        default:
            halShowCursor(false); // Hide the cursor
            ret = 0;
            break;
    }

    return(ret);
}

void halSetCursorPosition(int row, int col)
{
    move(row,col);
}

int halWriteDisplay(wchar_t *buffer, uint32_t *attrs, short *colors)
{
    int count;
    cchar_t wch;
    wchar_t character[] = {L'\0',L'\0'};
    
    if(buffer == NULL)
        return(0);

    move(0,0);

    // Write the root window to the screen
    //int count = mvaddwstr(0,0,buffer);
    for(count = 0; buffer[count] != L'\0'; count++)
    {
        //wch.chars[0] = buffer[count];
        //wch.chars[1] = L'\0';
        //wch.attr = attrs[count] | COLOR_PAIR(colors[count]);
        //wch.ext_color = colors[count];

        character[0] = buffer[count];
        setcchar(&wch,character,attrs[count],colors[count],NULL);
        // Apply the attribute to the window
        //wattrset(stdscr, attrs[count]);
        add_wch(&wch);
    }

    return(count);
}

void halRefreshDisplay(void)
{
    // Refresh the screen
    refresh();
}
