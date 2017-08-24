/* vdpm - View-dependent progressive meshes library
* Copyright 2015 Jim Tan
* https://github.com/kctan0805/vdpm
*
* vdpm is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "vdpm/Log.h"
#include "vdpm/StdInStream.h"

using namespace std;
using namespace vdpm;

StdInStream::StdInStream(const char filePath[])
{
    fin.open(filePath, ifstream::in | ifstream::binary);

    if (!fin.is_open())
    {
        Log::println("failed to open %s", filePath);
        return;
    }
    is = &fin;
}

StdInStream::StdInStream(std::istream* is)
{
    this->is = is;
}

StdInStream::~StdInStream()
{
    close();
}

void StdInStream::close()
{
    if (is == &fin)
    {
        fin.close();
        is = NULL;
    }
}

void StdInStream::readChar(char& value)
{
    is->read((char*)&value, sizeof(char));
}

void StdInStream::readUInt(unsigned int& value)
{
    is->read((char*)&value, sizeof(unsigned int));
}

void StdInStream::readFloat(float& value)
{
    is->read((char*)&value, sizeof(float));
}
