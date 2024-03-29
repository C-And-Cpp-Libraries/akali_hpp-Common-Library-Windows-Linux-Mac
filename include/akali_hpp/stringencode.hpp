﻿/*******************************************************************************
 * Copyright (C) 2021 - 2026, winsoft666, <winsoft666@outlook.com>.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Expect bugs
 *
 * Please use and enjoy. Please let me know of any bugs/improvements
 * that you have found/implemented and I will fix/incorporate them into this
 * file.
 *******************************************************************************/

#ifndef AKALI_STRINGENCODE_HPP_
#define AKALI_STRINGENCODE_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include <string>
#include <vector>
#include "akali_hpp/arch.hpp"
#ifdef AKALI_WIN
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#endif

#pragma warning(disable : 4309)

#define STACK_ARRAY(TYPE, LEN) static_cast<TYPE*>(::alloca((LEN) * sizeof(TYPE)))

namespace akali_hpp {
class StringEncode {
   public:
    static std::string UrlEncode(const std::string& str) {
        char hex[] = "0123456789ABCDEF";
        std::string dst;

        for (size_t i = 0; i < str.size(); ++i) {
            unsigned char cc = str[i];

            if (cc >= 'A' && cc <= 'Z' || cc >= 'a' && cc <= 'z' || cc >= '0' && cc <= '9' || cc == '.' ||
                cc == '_' || cc == '-' || cc == '*' || cc == '~') {
                dst += cc;
            }
            else {
                unsigned char c = static_cast<unsigned char>(str[i]);
                dst += '%';
                dst += hex[c / 16];
                dst += hex[c % 16];
            }
        }

        return dst;
    }

    static size_t UrlDecode(char* buffer, size_t buflen, const char* source, size_t srclen) {
        if (nullptr == buffer)
            return srclen + 1;

        if (buflen <= 0)
            return 0;

        unsigned char h1, h2;
        size_t srcpos = 0, bufpos = 0;

        while ((srcpos < srclen) && (bufpos + 1 < buflen)) {
            unsigned char ch = source[srcpos++];

            if (ch == '+') {
                buffer[bufpos++] = ' ';
            }
            else if ((ch == '%') && (srcpos + 1 < srclen) && HexDecode(source[srcpos], &h1) &&
                     HexDecode(source[srcpos + 1], &h2)) {
                buffer[bufpos++] = (h1 << 4) | h2;
                srcpos += 2;
            }
            else {
                buffer[bufpos++] = ch;
            }
        }

        buffer[bufpos] = '\0';
        return bufpos;
    }

    static std::string UrlDecode(const std::string& source) { return s_transform(source, UrlDecode); }

    static char HexEncode(unsigned char val) {
        static const char HEX[] = "0123456789abcdef";
        assert(val < 16);
        return (val < 16) ? HEX[val] : '!';
    }

    static bool HexDecode(char ch, unsigned char* val) {
        if ((ch >= '0') && (ch <= '9')) {
            *val = ch - '0';
        }
        else if ((ch >= 'A') && (ch <= 'Z')) {
            *val = (ch - 'A') + 10;
        }
        else if ((ch >= 'a') && (ch <= 'z')) {
            *val = (ch - 'a') + 10;
        }
        else {
            return false;
        }

        return true;
    }

    static size_t HexEncodeWithDelimiter(char* buffer,
                                         size_t buflen,
                                         const char* csource,
                                         size_t srclen,
                                         char delimiter) {
        assert(buffer);
        if (buflen == 0)
            return 0;

        // Init and check bounds.
        const unsigned char* bsource = reinterpret_cast<const unsigned char*>(csource);
        size_t srcpos = 0, bufpos = 0;
        size_t needed = delimiter ? (srclen * 3) : (srclen * 2 + 1);

        if (buflen < needed)
            return 0;

        while (srcpos < srclen) {
            unsigned char ch = bsource[srcpos++];
            buffer[bufpos] = HexEncode((ch >> 4) & 0xF);
            buffer[bufpos + 1] = HexEncode((ch)&0xF);
            bufpos += 2;

            // Don't write a delimiter after the last byte.
            if (delimiter && (srcpos < srclen)) {
                buffer[bufpos] = delimiter;
                ++bufpos;
            }
        }

        // Null terminate.
        buffer[bufpos] = '\0';
        return bufpos;
    }

    static std::string HexEncode(const std::string& str) {
        return HexEncode(str.c_str(), str.size());
    }

    static std::string HexEncode(const char* source, size_t srclen) {
        return HexEncodeWithDelimiter(source, srclen, 0);
    }

    static std::string HexEncodeWithDelimiter(const char* source, size_t srclen, char delimiter) {
        const size_t kBufferSize = srclen * 3;
        char* buffer = STACK_ARRAY(char, kBufferSize);
        size_t length = HexEncodeWithDelimiter(buffer, kBufferSize, source, srclen, delimiter);
        assert(srclen == 0 || length > 0);
        return std::string(buffer, length);
    }

    static size_t HexDecodeWithDelimiter(char* cbuffer,
                                         size_t buflen,
                                         const char* source,
                                         size_t srclen,
                                         char delimiter) {
        assert(cbuffer);
        if (buflen == 0)
            return 0;

        // Init and bounds check.
        unsigned char* bbuffer = reinterpret_cast<unsigned char*>(cbuffer);
        size_t srcpos = 0, bufpos = 0;
        size_t needed = (delimiter) ? (srclen + 1) / 3 : srclen / 2;

        if (buflen < needed)
            return 0;

        while (srcpos < srclen) {
            if ((srclen - srcpos) < 2) {
                // This means we have an odd number of bytes.
                return 0;
            }

            unsigned char h1, h2;

            if (!HexDecode(source[srcpos], &h1) || !HexDecode(source[srcpos + 1], &h2))
                return 0;

            bbuffer[bufpos++] = (h1 << 4) | h2;
            srcpos += 2;

            // Remove the delimiter if needed.
            if (delimiter && (srclen - srcpos) > 1) {
                if (source[srcpos] != delimiter)
                    return 0;

                ++srcpos;
            }
        }

        return bufpos;
    }

    static size_t HexDecode(char* buffer, size_t buflen, const std::string& source) {
        return HexDecodeWithDelimiter(buffer, buflen, source, 0);
    }

    static size_t HexDecodeWithDelimiter(char* buffer,
                                         size_t buflen,
                                         const std::string& source,
                                         char delimiter) {
        return HexDecodeWithDelimiter(buffer, buflen, source.c_str(), source.length(), delimiter);
    }

#ifdef AKALI_WIN

    static std::string UnicodeToAnsi(const std::wstring& str, unsigned int code_page = 0) {
        std::string strRes;
        int iSize = ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char* szBuf = new (std::nothrow) char[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    static std::wstring AnsiToUnicode(const std::string& str, unsigned int code_page = 0) {
        std::wstring strRes;

        int iSize = ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, NULL, 0);

        if (iSize == 0)
            return strRes;

        wchar_t* szBuf = new (std::nothrow) wchar_t[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize * sizeof(wchar_t));

        ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    static std::string UnicodeToUtf8(const std::wstring& str) {
        std::string strRes;

        int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char* szBuf = new (std::nothrow) char[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize);

        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    static std::string UnicodeToUtf8BOM(const std::wstring& str) {
        std::string strRes;

        int iSize = ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

        if (iSize == 0)
            return strRes;

        char* szBuf = new (std::nothrow) char[iSize + 3];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize + 3);

        ::WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &szBuf[3], iSize, NULL, NULL);
        szBuf[0] = 0xef;
        szBuf[1] = 0xbb;
        szBuf[2] = 0xbf;

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    static std::wstring Utf8ToUnicode(const std::string& str) {
        std::wstring strRes;
        int iSize = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

        if (iSize == 0)
            return strRes;

        wchar_t* szBuf = new (std::nothrow) wchar_t[iSize];

        if (!szBuf)
            return strRes;

        memset(szBuf, 0, iSize * sizeof(wchar_t));
        ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, szBuf, iSize);

        strRes = szBuf;
        delete[] szBuf;

        return strRes;
    }

    static std::string AnsiToUtf8(const std::string& str, unsigned int code_page = 0) {
        return UnicodeToUtf8(AnsiToUnicode(str, code_page));
    }

    static std::string AnsiToUtf8BOM(const std::string& str, unsigned int code_page = 0) {
        return UnicodeToUtf8BOM(AnsiToUnicode(str, code_page));
    }

    static std::string Utf8ToAnsi(const std::string& str, unsigned int code_page = 0) {
        return UnicodeToAnsi(Utf8ToUnicode(str), code_page);
    }

#endif

   protected:
    // Apply any suitable string transform (including the ones above) to an STL
    // string. Stack-allocated temporary space is used for the transformation, so
    // value and source may refer to the same string.
    typedef size_t (*Transform)(char* buffer, size_t buflen, const char* source, size_t srclen);

    // Return the result of applying transform t to source.
    static std::string s_transform(const std::string& source, Transform t) {
        // Ask transformation function to approximate the destination size (returns upper bound)
        size_t maxlen = t(nullptr, 0, source.data(), source.length());
        char* buffer = STACK_ARRAY(char, maxlen);
        size_t len = t(buffer, maxlen, source.data(), source.length());
        std::string result(buffer, len);
        return result;
    }
};

#if (defined UNICODE || defined _UNICODE)
#define TCHARToAnsi(str) akali_hpp::StringEncode::UnicodeToAnsi((str), 0)
#define TCHARToUtf8(str) akali_hpp::StringEncode::UnicodeToUtf8((str))
#define AnsiToTCHAR(str) akali_hpp::StringEncode::AnsiToUnicode((str), 0)
#define Utf8ToTCHAR(str) akali_hpp::StringEncode::Utf8ToUnicode((str))
#define TCHARToUnicode(str) ((std::wstring)(str))
#define UnicodeToTCHAR(str) ((std::wstring)(str))
#else
#define TCHARToAnsi(str) ((std::string)(str))
#define TCHARToUtf8 akali_hpp::StringEncode::AnsiToUtf8((str), 0)
#define AnsiToTCHAR(str) ((std::string)(str))
#define Utf8ToTCHAR(str) akali_hpp::StringEncode::Utf8ToAnsi((str), 0)
#define TCHARToUnicode(str) akali_hpp::StringEncode::AnsiToUnicode((str), 0)
#define UnicodeToTCHAR(str) akali_hpp::StringEncode::UnicodeToAnsi((str), 0)
#endif
}  // namespace akali_hpp
#endif  // AKALI_STRINGENCODE_HPP_
