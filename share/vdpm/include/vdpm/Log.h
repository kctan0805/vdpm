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
#ifndef VDPM_LOG_H
#define VDPM_LOG_H

namespace vdpm
{
    class Log
    {
    public:
        ~Log();

        static Log& getInstance();
        static void(*println)(const char *format, ...);

    private:
        Log();

        static void printlnEmpty(const char *format, ...);
    };
} // namespace vdpm

#endif // VDPM_LOG_H
