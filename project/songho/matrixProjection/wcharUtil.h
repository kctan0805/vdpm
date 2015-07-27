///////////////////////////////////////////////////////////////////////////////
// wcharUtil.h
// ===========
// conversion utility between multi-byte char and wide char
// It also converts any number to char or wchar_t format. For example,
// toWchar(1) converts the number 1 to a wchar_t string, L"1".
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-07-14
// UPDATED: 2006-07-14
///////////////////////////////////////////////////////////////////////////////

#ifndef WCHAR_UTIL_H
#define WCHAR_UTIL_H

const wchar_t* toWchar(const char *str);        // convert char* to wchar_t*
const wchar_t* toWchar(double number);          // convert number to wchar_t*

const char* toChar(const wchar_t *str);         // convert wchar_t* to char*
const char* toChar(double number);              // convert number to char*

#endif
