/*
 * csv.c
 *
 * CSV file parser for the Linux Console Application Framework (lcaf) library
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

// Function to remove leading/trailing whitespace from a string
char *csvTrim(char *str)
{
    char *end;

    // Trim leading space
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
        str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        end--;

    // Write new null terminator character
    *(end + 1) = '\0';

    return str;
}

// Function to read a CSV field, one character at a time
char *csvReadField(FILE *file)
{
    char *field = NULL;
    int field_len = 0;
    char c;
    bool in_quotes = false;

    while ((c = fgetc(file)) != EOF)
    {
        // If the character is a quote...
        if (c == '"')
        {
            // If the next character is not a quote...
            if((c = fgetc(file)) != '"')
            {
                // Toggle in quotes status, put the character back, and continue
                in_quotes = !in_quotes;
                ungetc(c, file);
                continue;
            }
        }

        // If delimiter, then field is complete
        if (c == ',' && !in_quotes)
            break;

        // Allocate another byte in field
        field = realloc(field, field_len + 1);
        if (field == NULL)
            return NULL;
        field[field_len++] = c;
    }

    // If reached the end of the file...
    if(c == EOF)
    {
        // Free field and return a NULL
        free(field);
        return NULL;
    }

    // Allocate another byte in field for the NULL terminator
    field = realloc(field, field_len + 1);
    if (field == NULL)
        return NULL;

    field[field_len] = '\0';
    
    return field;
}
