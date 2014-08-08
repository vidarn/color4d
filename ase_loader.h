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

#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <assert.h>

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
    ASE_ERRORTYPE_UNEXPECTED_EOF
} ASE_ERRORTYPE;

typedef struct{
    wchar_t *name;
    ASE_COLORTYPE type;
    float col[4];
} ASE_COLOR;

typedef struct{
    wchar_t *name;
    uint16_t numColors;
    ASE_COLOR *colors;
} ASE_GROUP;

typedef struct{
    uint16_t numGroups;
    ASE_GROUP *groups;
} ASE_FILE;

static ASE_ERRORTYPE ase_readAseFile(ASE_FILE *ase, FILE *f);
static ASE_ERRORTYPE ase_openAndReadAseFile(ASE_FILE *ase, const char *filename);
static void ase_freeAseFile(ASE_FILE *ase);
static const wchar_t* ase_getErrorString(ASE_ERRORTYPE error);


/*  ---------------- Implementation ----------------------- */

static uint16_t ase_be16_to_cpu(const uint8_t *buf)
{
    return ((uint16_t)buf[1]) | (((uint16_t)buf[0]) << 8);
}

static uint32_t ase_be32_to_cpu(const uint8_t *buf)
{
    return  (((uint32_t)buf[0]) << 8*3) | (((uint32_t)buf[1]) << 8*2) | (((uint32_t)buf[2]) << 8) | ((uint32_t)buf[3]);
}

static ASE_ERRORTYPE ase_read_uint16(uint16_t *dest, uint16_t num, FILE *f)
{
    uint8_t tmp[2];
    uint16_t i;
    if(feof(f)) return ASE_ERRORTYPE_UNEXPECTED_EOF;
    for(i=0;i<num;i++){
        fread(tmp,sizeof(uint16_t),1,f);
        dest[i] = ase_be16_to_cpu(tmp);
    }
    return ASE_ERRORTYPE_SUCCESS;
}

static ASE_ERRORTYPE ase_read_uint32(uint32_t *dest, uint32_t num, FILE *f)
{
    uint8_t tmp[4];
    uint32_t i;
    if(feof(f)) return ASE_ERRORTYPE_UNEXPECTED_EOF;
    for(i=0;i<num;i++){
        fread(tmp,sizeof(uint32_t),1,f);
        dest[i] = ase_be32_to_cpu(tmp);
    }
    return ASE_ERRORTYPE_SUCCESS;
}

typedef enum{
    BLOCKTYPE_GROUP_START,
    BLOCKTYPE_GROUP_END,
    BLOCKTYPE_COLOR
} BLOCKTYPE;


static ASE_ERRORTYPE ase_readBlock(BLOCKTYPE *blockType, float color[4],ASE_COLORTYPE *colorType, wchar_t **name,FILE *f)
{
    char model[5];
    uint16_t i;
    uint16_t tmpBlockType;
    uint32_t blockLength = 0;
    uint16_t nameLength = 0;
    ASE_ERRORTYPE error = ase_read_uint16(&tmpBlockType,1,f);
    if(error) return error;
    switch(tmpBlockType){
        case 0xc001:
            *blockType = BLOCKTYPE_GROUP_START;
            break;
        case 0xc002:
            *blockType = BLOCKTYPE_GROUP_END;
            break;
        case 0x0001:
            *blockType = BLOCKTYPE_COLOR;
            break;
        default:
            return ASE_ERRORTYPE_INVALID_FILE;
    }
    error = ase_read_uint32(&blockLength, 1, f);
    if(error) return error;
    if(blockLength > 0){
        error = ase_read_uint16(&nameLength, 1, f);
        if(error) return error;
        if(nameLength > 0){
            *name = (wchar_t *)malloc(sizeof(wchar_t) * nameLength);
            for(i=0;i<nameLength;i++){
                uint16_t tmp;
                error = ase_read_uint16(&tmp,1,f);
                if(error){
                    free(*name);
                    return error;
                }
                (*name)[i] = (wchar_t)tmp;
            }
        }
        if(*blockType == BLOCKTYPE_COLOR){
            uint32_t tmp[4];
            uint16_t type, numVars;
            model[4] = 0;
            if(feof(f)){
                free(*name);
                return ASE_ERRORTYPE_UNEXPECTED_EOF;
            }
            fread(model,sizeof(char),4,f);
            if(strcmp(model,"RGB ") == 0){
                *colorType = ASE_COLORTYPE_RGB;
                numVars = 3;
            } else if(strcmp(model,"LAB ") == 0){
                *colorType = ASE_COLORTYPE_LAB;
                numVars = 3;
            } else if(strcmp(model,"CMYK") == 0){
                *colorType = ASE_COLORTYPE_CMYK;
                numVars = 4;
            } else if(strcmp(model,"GRAY") == 0){
                *colorType = ASE_COLORTYPE_GRAY;
                numVars = 1;
            }
            else {
                return ASE_ERRORTYPE_INVALID_FILE;
            }
            error = ase_read_uint32(tmp,numVars,f);
            if(error){
                free(*name);
                return error;
            }
            for(i = 0; i<4;i++){
                if(i < numVars){
                    color[i] = *(float *)&tmp[i];
                } else{
                    color[i] = -1.f;
                }
            }
            error = ase_read_uint16(&type,1,f);
            if(error){
                free(*name);
                return error;
            }
        }
    }
    return ASE_ERRORTYPE_SUCCESS;
}

static ASE_ERRORTYPE ase_openAndReadAseFile(ASE_FILE *ase, const char *filename)
{
    ASE_ERRORTYPE error;
    FILE *f = fopen(filename,"r");
    if(f){
        error = ase_readAseFile(ase,f);
        fclose(f);
    }
    else{
        error = ASE_ERRORTYPE_COULD_NOT_OPEN_FILE;
    }
    return error;
}

static ASE_ERRORTYPE ase_readAseFile(ASE_FILE *ase, FILE *f)
{
    float c[4];
    ASE_COLORTYPE colorType;
    ASE_ERRORTYPE error;
    ASE_COLOR *col;
    BLOCKTYPE blockType;
    wchar_t *name;
    char sig[5];
    uint16_t version[2];
    uint32_t numBlocks;
    uint16_t i, ii;
    ASE_GROUP *currentGroup = NULL;
    ase->numGroups = 0;
    ase->groups = NULL;
    if(feof(f)){
        return ASE_ERRORTYPE_UNEXPECTED_EOF;
    }
    fread(sig,sizeof(char),4,f);
    sig[4] = 0;
    if(strcmp(sig,"ASEF") != 0){
        return ASE_ERRORTYPE_INVALID_FILE;
    }
    error = ase_read_uint16(version,2,f);
    if(error) return error;
    error = ase_read_uint32(&numBlocks, 1, f);
    if(error) return error;
    for(i=0;i<numBlocks;i++){
        error = ase_readBlock(&blockType, c,&colorType,&name,f);
        if(error)
        {
            ase_freeAseFile(ase);
            return error;
        }
        switch(blockType){
            case BLOCKTYPE_GROUP_START:
                ase->numGroups++;
                ase->groups = (ASE_GROUP *)realloc(ase->groups,ase->numGroups*sizeof(ASE_GROUP));
                currentGroup = ase->groups + ase->numGroups - 1;
                currentGroup->name = name;
                currentGroup->numColors = 0;
                currentGroup->colors = NULL;
                break;
            case BLOCKTYPE_COLOR:
                currentGroup->numColors++;
                currentGroup->colors = (ASE_COLOR *)realloc(currentGroup->colors,currentGroup->numColors*sizeof(ASE_COLOR));
                col = currentGroup->colors + currentGroup->numColors - 1;
                col->name = name;
                col->type = colorType;
                for(ii=0;ii<4;ii++){
                    col->col[ii] = c[ii];
                }
                break;
            case BLOCKTYPE_GROUP_END:
                break;
        }
    }
    return ASE_ERRORTYPE_SUCCESS;
}

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

static const wchar_t* ase_getErrorString(ASE_ERRORTYPE error)
{
    switch(error){
        case ASE_ERRORTYPE_SUCCESS:
            return L"Success";
        case ASE_ERRORTYPE_COULD_NOT_OPEN_FILE:
            return L"Could not open file";
        case ASE_ERRORTYPE_INVALID_FILE:
            return L"Invalid file";
        case ASE_ERRORTYPE_UNEXPECTED_EOF:
            return L"Unexpected end-of-file";
    }
    return L"Unknown";
}
