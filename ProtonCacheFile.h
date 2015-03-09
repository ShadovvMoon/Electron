//
//  ProtonCacheFile.h
//  Proton
//
//  Created by Paul Whitcomb on 3/4/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#ifndef __Proton__ProtonCacheFile__
#define __Proton__ProtonCacheFile__

#include <stdio.h>
#include <vector>

struct ProtonCacheFile {
public:
    const void *Data() const;
    size_t Length() const;
    
    ProtonCacheFile();
    ProtonCacheFile(const char *data, size_t length);
    ProtonCacheFile(const ProtonCacheFile& map);
    ProtonCacheFile& operator=(const ProtonCacheFile& map);
    
protected:
    std::unique_ptr<char []> data_ptr;
    size_t data_length = 0;
};

#endif /* defined(__Proton__ProtonCacheFile__) */
