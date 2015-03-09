//
//  ProtonCacheFile.cpp
//  Proton
//
//  Created by Paul Whitcomb on 3/4/15.
//  Copyright (c) 2015 Paul Whitcomb. All rights reserved.
//

#include "ProtonCacheFile.h"

ProtonCacheFile::ProtonCacheFile() {}

ProtonCacheFile::ProtonCacheFile(const char *data, size_t length) {
    this->data_ptr = std::unique_ptr<char []>(new char[length]);
    this->data_length = length;
    memcpy(this->data_ptr.get(), data, length);
}

ProtonCacheFile& ProtonCacheFile::operator=(const ProtonCacheFile &file) {
    this->data_ptr = std::unique_ptr<char []>(new char[file.data_length]);
    memcpy(this->data_ptr.get(),file.data_ptr.get(),file.data_length);
    this->data_length = file.data_length;
    return *this;
}

const void *ProtonCacheFile::Data() const {
    return this->data_ptr.get();
}

size_t ProtonCacheFile::Length() const {
    return this->data_length;
}

ProtonCacheFile::ProtonCacheFile(const ProtonCacheFile& file) {
    *this = file;
}