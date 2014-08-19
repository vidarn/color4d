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
#include <string.h>
#include "ase_common.h"

/* ------------------ Public symbols -------------------- */

static ASE_ERRORTYPE ase_readAseFile(ASE_FILE *ase, FILE *f);
static ASE_ERRORTYPE ase_openAndReadAseFile(ASE_FILE *ase, const char *filename);

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

static ASE_ERRORTYPE ase_readBlock(ASE_BLOCKTYPE *blockType, float color[4],ASE_COLORTYPE *colorType, char **name,FILE *f)
{
    uint16_t tmpBlockType;
    uint32_t blockLength = 0;
    uint16_t nameLength = 0;
    ASE_ERRORTYPE error = ase_read_uint16(&tmpBlockType,1,f);
    if(error) return error;
    switch(tmpBlockType){
        case 0xc001:
            *blockType = ASE_BLOCKTYPE_GROUP_START;
            break;
        case 0xc002:
            *blockType = ASE_BLOCKTYPE_GROUP_END;
            break;
        case 0x0001:
            *blockType = ASE_BLOCKTYPE_COLOR;
            break;
        default:
            return ASE_ERRORTYPE_INVALID_FILE;
    }
    error = ase_read_uint32(&blockLength, 1, f);
    if(error) return error;
    if(blockLength > 0){
        uint16_t i;
        error = ase_read_uint16(&nameLength, 1, f);
        if(error) return error;
        if(nameLength > 0){
#ifndef ASE_NO_UTF8
            int32_t len;
            UChar *tmp = (UChar *)malloc(sizeof(uint16_t) * nameLength);
            UErrorCode errorCode;
            error = ase_read_uint16(tmp,nameLength,f);
            if(error){
                free(tmp);
                return error;
            }
            errorCode = U_ERROR_WARNING_START;
            u_strToUTF8WithSub(NULL,0,&len,tmp,-1,0xFFFD,NULL,&errorCode);
            len++;
            *name = (char *)malloc(sizeof(char)*len);
            errorCode = U_ERROR_WARNING_START;
            u_strToUTF8WithSub(*name,len,NULL,tmp,-1,0xFFFD,NULL,&errorCode);
            if(U_FAILURE(errorCode)){
                return ASE_ERRORTYPE_UNICODE;
            }
            free(tmp);
#else
            *name = (char *)malloc(sizeof(uint16_t) * nameLength);
            error = ase_read_uint16((uint16_t *)*name,nameLength,f);
            if(error){
                free(*name);
                return error;
            }
#endif
        }
        if(*blockType == ASE_BLOCKTYPE_COLOR){
            uint32_t tmp[4];
            uint16_t type, numVars;
            char model[5];
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
    ASE_BLOCKTYPE blockType;
    char *name;
    char sig[5];
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
    error = ase_read_uint16(ase->version,2,f);
    if(error) return error;
    error = ase_read_uint32(&numBlocks, 1, f);
    if(error) return error;
    for(i=0;i<numBlocks;i++){
        error = ase_readBlock(&blockType, c,&colorType,&name,f);
        if(error)
        {
            free(name);
            /*ase_freeAseFile(ase);*/
            return error;
        }
        switch(blockType){
            case ASE_BLOCKTYPE_GROUP_START:
                ase->numGroups++;
                ase->groups = (ASE_GROUP *)realloc(ase->groups,ase->numGroups*sizeof(ASE_GROUP));
                currentGroup = ase->groups + ase->numGroups - 1;
                currentGroup->name = name;
                currentGroup->numColors = 0;
                currentGroup->colors = NULL;
                break;
            case ASE_BLOCKTYPE_COLOR:
                currentGroup->numColors++;
                currentGroup->colors = (ASE_COLOR *)realloc(currentGroup->colors,currentGroup->numColors*sizeof(ASE_COLOR));
                col = currentGroup->colors + currentGroup->numColors - 1;
                col->name = name;
                col->type = colorType;
                for(ii=0;ii<4;ii++){
                    col->col[ii] = c[ii];
                }
                break;
            case ASE_BLOCKTYPE_GROUP_END:
                break;
        }
    }
    return ASE_ERRORTYPE_SUCCESS;
}

