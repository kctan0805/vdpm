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
#ifndef VDPM_STDINSTREAM_H
#define VDPM_STDINSTREAM_H

#include <fstream>
#include "vdpm/InStream.h"

namespace vdpm
{
    class StdInStream : public InStream
    {
    public:
        StdInStream(const char filePath[]);
        StdInStream(std::istream* is);
        ~StdInStream();

        void close();

        void readChar(char& value);
        void readUInt(unsigned int& value);
        void readFloat(float& value);

    private:
        std::istream* is;
        std::ifstream fin;
    };
} // namespace vdpm

#endif // VDPM_STDINSTREAM_H
