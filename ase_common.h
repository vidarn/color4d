/*
   The MIT License (MIT)

   Copyright (c) 2014 Vidar Nelson

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#pragma once
#ifndef ASE_NO_UTF8
#include <unicode/ustring.h>
#include <stdio.h>
#endif

/* ------------------ Public symbols -------------------- */

typedef enum{
    ASE_COLORTYPE_RGB,
    ASE_COLORTYPE_CMYK,
    ASE_COLORTYPE_LAB,
    ASE_COLORTYPE_GRAY
} ASE_COLORTYPE;

typedef enum{
    ASE_ERRORTYPE_SUCCESS,
    ASE_ERRORTYPE_COULD_NOT_OPEN_FILE,
    ASE_ERRORTYPE_INVALID_FILE,
    ASE_ERRORTYPE_INVALID_ASE,
    ASE_ERRORTYPE_UNEXPECTED_EOF,
    ASE_ERRORTYPE_UNICODE
} ASE_ERRORTYPE;

typedef struct{
    char *name;
    ASE_COLORTYPE type;
    float col[4];
} ASE_COLOR;

typedef struct{
    char *name;
    uint16_t numColors;
    ASE_COLOR *colors;
} ASE_GROUP;

typedef struct{
    uint16_t numGroups;
    uint16_t version[2];
    ASE_GROUP *groups;
} ASE_FILE;

static void ase_freeAseFile(ASE_FILE *ase);
static const char* ase_getErrorString(ASE_ERRORTYPE error);

/*  ---------------- Implementation ----------------------- */

typedef enum{
    ASE_BLOCKTYPE_GROUP_START,
    ASE_BLOCKTYPE_GROUP_END,
    ASE_BLOCKTYPE_COLOR
} ASE_BLOCKTYPE;


static void ase_freeAseFile(ASE_FILE *ase)
{
    uint16_t i, ii;
    ASE_GROUP *group;
    ASE_COLOR *color;
    for(i=0;i<ase->numGroups;i++){
        group = ase->groups + i;
        for(ii=0;ii<group->numColors;ii++){
            color = group->colors + ii;
            free(color->name);
        }
        free(group->colors);
        free(group->name);
    }
    free(ase->groups);
    ase->numGroups = 0;
}

static const char* ase_getErrorString(ASE_ERRORTYPE error)
{
    switch(error){
        case ASE_ERRORTYPE_SUCCESS:
            return "Success";
        case ASE_ERRORTYPE_COULD_NOT_OPEN_FILE:
            return "Could not open file";
        case ASE_ERRORTYPE_INVALID_FILE:
            return "Invalid file";
        case ASE_ERRORTYPE_INVALID_ASE:
            return "Invalid ase";
        case ASE_ERRORTYPE_UNEXPECTED_EOF:
            return "Unexpected end-of-file";
        case ASE_ERRORTYPE_UNICODE:
            return "Unicode error";
    }
    return "Unknown";
}

static uint16_t ase_uint16StrLen(const uint16_t *str)
{
    uint16_t len = 0;
    while(str[len] != 0){
        len++;
    }
    len++;
    return len;
}

