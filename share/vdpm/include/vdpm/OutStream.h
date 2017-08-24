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
#ifndef VDPM_OUTSTREAM_H
#define VDPM_OUTSTREAM_H

namespace vdpm
{
    class OutStream
    {
    public:
        virtual void writeChar(char& value) = 0;
        virtual void writeUInt(unsigned int& value) = 0;
        virtual void writeFloat(float& value) = 0;
    };
} // namespace vdpm

#endif // VDPM_OUTSTREAM_H
