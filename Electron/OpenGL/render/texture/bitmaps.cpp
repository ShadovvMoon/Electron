//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include <stdlib.h>
#include <stdio.h>
#include "bitmaps.h"

void *bitmaps;
bool load_bitmaps(const char *bitmap_file) {
    long length = 0;
    FILE * f = fopen (bitmap_file, "rb");
    
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = (long)ftell(f);
        fseek (f, 0, SEEK_SET);
        bitmaps = (char*)malloc (length);
        if (bitmaps)
        {
            fread (bitmaps, 1, length, f);
        }
        fclose (f);
        return true;
    }
    return false;
}

void *bitmap_data() {
    return bitmaps;
}
