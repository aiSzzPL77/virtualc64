/*
 * (C) 2010 - 2018 Dirk W. Hoffmann. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "AnyC64File.h"

AnyC64File::AnyC64File()
{
    const char *defaultName = "HELLO VIRTUALC64";
    memcpy(name, defaultName, strlen(defaultName) + 1);
}

AnyC64File::~AnyC64File()
{
    dealloc();

    if (path)
		free(path);
}

void
AnyC64File::dealloc()
{
    if (data == NULL) {
        assert(size == 0);
        return;
    }
    
    delete[] data;
    data = NULL;
    size = 0;
}

bool
AnyC64File::checkBufferHeader(const uint8_t *buffer, size_t length, const uint8_t *header)
{
    assert(buffer != NULL);
    assert(header != NULL);
    
    unsigned i;
    
    for (i = 0; i < length && header[i] != 0; i++) {
        if (header[i] != buffer[i])
            return false;
    }
 
    return header[i] == 0;
}

void
AnyC64File::setPath(const char *str)
{
    assert(str != NULL);
    
    // Set path
    if (path) free(path);
    path = strdup(str);
    
    // Set default name
    memset(name, 0, sizeof(name));
    char *filename = ExtractFilenameWithoutSuffix(path);
    strncpy(name, filename, sizeof(name) - 1);
    free(filename);
    ascii2petStr(name);
}

const char *
AnyC64File::getName()
{
    return name;
}

const unsigned short *
AnyC64File::getUnicodeName()
{
    translateToUnicode(name, unicode, 0xE000, sizeof(unicode) / 2);
    return unicode;
}

void
AnyC64File::seek(long offset)
{
    fp = (offset < size) ? size : -1;
}

int
AnyC64File::getByte()
{
    int result;
    
    assert(eof <= size);
    
    if (fp < 0)
        return -1;
    
    // get byte
    result = data[fp];
    
    // check for end of file
    if (fp == eof) {
        fp = -1;
    } else {
        fp++;
    }
    
    return result;
}


bool
AnyC64File::readFromBuffer(const uint8_t *buffer, size_t length)
{
    assert (buffer != NULL);
    
    dealloc();
    if ((data = new uint8_t[length]) == NULL)
        return false;
    
    memcpy(data, buffer, length);
    size = length;
    eof = length;
    seek(0);
    return true;
}

bool
AnyC64File::readFromFile(const char *filename)
{
    assert (filename != NULL);
    
    bool success = false;
	uint8_t *buffer = NULL;
	FILE *file = NULL;
	struct stat fileProperties;
	
	// Check file type
    if (!hasSameType(filename)) {
		goto exit;
	}
	
	// Get file properties
    if (stat(filename, &fileProperties) != 0) {
		goto exit;
	}
		
	// Open file
	if (!(file = fopen(filename, "r"))) {
		goto exit;
	}

	// Allocate memory
	if (!(buffer = new uint8_t[fileProperties.st_size])) {
		goto exit;
	}
	
	// Read from file
	int c;
	for (unsigned i = 0; i < fileProperties.st_size; i++) {
		c = fgetc(file);
		if (c == EOF)
			break;
		buffer[i] = (uint8_t)c;
	}
	
	// Read from buffer (subclass specific behaviour)
	dealloc();
	if (!readFromBuffer(buffer, (unsigned)fileProperties.st_size)) {
		goto exit;
	}

    setPath(filename);
    success = true;
    
    debug(1, "File %s read successfully\n", path);
	
exit:
	
    if (file)
		fclose(file);
	if (buffer)
		delete[] buffer;

	return success;
}

size_t
AnyC64File::writeToBuffer(uint8_t *buffer)
{
	return 0;
}

bool 
AnyC64File::writeToFile(const char *filename)
{
	bool success = false;
	uint8_t *data = NULL;
	FILE *file;
	size_t filesize;
   
    // Determine file size
    filesize = writeToBuffer(NULL);
    if (filesize == 0)
        return false;
    
	// Open file
    assert (filename != NULL);
	if (!(file = fopen(filename, "w"))) {
		goto exit;
	}
		
	// Allocate memory
    if (!(data = new uint8_t[filesize])) {
		goto exit;
	}
	
	// Write to buffer 
	if (!writeToBuffer(data)) {
		goto exit;
	}

	// Write to file
	for (unsigned i = 0; i < filesize; i++) {
		fputc(data[i], file);
	}	
	
	success = true;

exit:
		
	if (file)
        fclose(file);
	if (data)
        delete[] data;
		
	return success;
}
