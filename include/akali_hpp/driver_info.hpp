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
#ifndef AKALI_DRIVER_INFO_HPP_
#define AKALI_DRIVER_INFO_HPP_
#include "akali_hpp/arch.hpp"
#include <string>
#ifdef AKALI_WIN
#include <assert.h>
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#include <direct.h>
#endif

namespace akali_hpp {
#ifdef AKALI_WIN
class WinDriverInfo {
   public:
    enum { DRIVE_HOST = 7 };

    static float GetFreeMB(int nDrive) {
        std::wstring sRoot;
        unsigned long totalClusters, freeClusters, sectors, bytes;

        assert(nDrive > 0 && nDrive <= 26);

        if (IsDriveAvailable(nDrive)) {
            sRoot = GetRoot(nDrive);

            ::GetDiskFreeSpaceW(sRoot.c_str(), &sectors, &bytes, &freeClusters, &totalClusters);

            // do maths like this to avoid truncation
            // errors
            float fSpace = (float)sectors;
            fSpace *= bytes;
            fSpace *= freeClusters;
            fSpace /= 1048576;

            return fSpace;
        }

        return 0.0f;
    }

    static std::wstring GetVolume(int nDrive) {
        std::wstring sVolume, sRoot;

        assert(nDrive > 0 && nDrive <= 26);

        if (IsDriveAvailable(nDrive)) {
            sRoot = GetRoot(nDrive);

            wchar_t buf[20] = {0};
            GetVolumeInformationW(sRoot.c_str(), buf, 20, NULL, NULL, NULL, NULL, 0);

            std::wstring strVolume = buf;
            FormatName(sVolume);

            return sVolume;
        }

        return L"";
    }

    static std::wstring GetRoot(int nDrive) {
        std::wstring sRoot;

        sRoot = GetLetter(nDrive);
        sRoot += L":\\";

        return sRoot;
    }

    static std::wstring GetFullName(int nDrive) {
        std::wstring sFullName, sLetter, sVolume;

        assert(nDrive > 0 && nDrive <= 26);

        if (IsDriveAvailable(nDrive)) {
            sLetter = GetLetter(nDrive);
            sVolume = GetVolume(nDrive);

            sFullName = sVolume + L" (" + sLetter + L":)";

            return sFullName;
        }

        return L"";
    }

    static float GetTotalMB(int nDrive) {
        std::wstring sRoot;
        unsigned long totalClusters, freeClusters, sectors, bytes;

        assert(nDrive > 0 && nDrive <= 26);

        if (IsDriveAvailable(nDrive)) {
            sRoot = GetRoot(nDrive);

            ::GetDiskFreeSpaceW(sRoot.c_str(), &sectors, &bytes, &freeClusters, &totalClusters);

            // do maths like this to avoid truncation
            // errors
            float fSpace = (float)sectors;
            fSpace *= bytes;
            fSpace *= totalClusters;
            fSpace /= 1048576;

            return fSpace;
        }

        return 0.0f;
    }

    static wchar_t GetLetter(int nDrive) {
        assert(nDrive > 0 && nDrive <= 26);

        return (wchar_t)(nDrive + L'A' - 1);
    }

    static int GetType(int nDrive) {
        std::wstring sVolume;

        assert(nDrive > 0 && nDrive <= 26);

        // shortcut to avoid floppy access
        if (nDrive == 1 || nDrive == 2)
            return DRIVE_REMOVABLE;

        return ::GetDriveTypeW(GetRoot(nDrive).c_str());
    }

    static int GetPathType(const wchar_t* szPathName) {
        int nDrive = GetDrive(szPathName);

        if (nDrive >= 0)
            return GetType(nDrive);

        else if (IsRemotePath(szPathName) > 0)
            return DRIVE_REMOTE;

        else
            return DRIVE_UNKNOWN;
    }

    static void FormatName(std::wstring& sFilename) {
        // std::wstring sTemp, sChar;
        // int nLen, nChar;
        // wchar_t cChar, cLastChar = ' '; // space

        //// this function accepts pathnames and names with spaces
        // sFilename.MakeLower();
        // nLen = sFilename.GetLength();

        //// for each word make the first letter upper case
        // for (nChar = 0; nChar < nLen; nChar++)
        //{
        //    cChar = sFilename[nChar];

        //    if (cLastChar == ' ' || cLastChar == '\\')
        //    {
        //        sChar = std::wstring(cChar);
        //        sChar.MakeUpper();
        //        cChar = sChar[0];
        //    }

        //    sTemp += cChar;
        //    cLastChar = cChar;
        //}

        // sFilename = sTemp;
    }

    static bool IsDriveAvailable(int nDrive) {
        int nCurDrive;
        int nRes;

        // save cur drive and try to change to drive
        nCurDrive = _getdrive();
        nRes = _chdrive(nDrive);

        // if change successful change back before return
        if (nRes == 0)
            _chdrive(nCurDrive);

        return (nRes == 0) ? true : false;
    }

    static bool IsUNCPath(const wchar_t* szPathName) { return (wcsstr(szPathName, L"\\\\") == szPathName); }

    static int GetDrive(const wchar_t* szPathName) {
        int nDrive = 0;

        if (wcsstr(szPathName, L":") == szPathName + 1) {
            wchar_t cDrive = szPathName[0];
            cDrive = (wchar_t)toupper(cDrive);
            nDrive = cDrive - 64;
        }

        return nDrive ? nDrive : -1;
    }

    static bool IsMappedPath(const wchar_t* szPathName) {
        int nDrive;

        nDrive = GetDrive(szPathName);

        if (nDrive <= 0)
            return FALSE;

        return (GetType(nDrive) == DRIVE_REMOTE);
    }

    static int IsRemotePath(const wchar_t* szPathName, bool bAllowFileCheck = true) {
        if (bAllowFileCheck) {
            DWORD dwAttr = ::GetFileAttributesW(szPathName);

            if (dwAttr == 0xffffffff)
                return -1;
        }

        return (IsUNCPath(szPathName) || IsMappedPath(szPathName));
    }

    static bool IsFixedPath(const wchar_t* szPathName) {
        int nDrive = GetDrive(szPathName);

        if (nDrive == -1)  // unknown
            return FALSE;

        return (GetType(nDrive) == DRIVE_FIXED);
    }

    static bool IsRemovablePath(const wchar_t* szPathName) {
        int nDrive = GetDrive(szPathName);

        if (nDrive == -1)  // unknown
            return FALSE;

        return (GetType(nDrive) == DRIVE_REMOVABLE);
    }

    // -1 = no such path, else TRUE/FALSE
    static int IsReadonlyPath(const wchar_t* szPathName) {
        DWORD dwAttr = ::GetFileAttributesW(szPathName);

        if (dwAttr == 0xffffffff)
            return -1;

        // else
        return (dwAttr & FILE_ATTRIBUTE_READONLY);
    }

    static unsigned long GetSerialNumber(int nDrive) {
        if (GetType(nDrive) != DRIVE_FIXED)
            return 0;

        DWORD dwHDSerialNum = 0;

        if (!::GetVolumeInformationW(GetRoot(nDrive).c_str(), NULL, 0, &dwHDSerialNum, NULL, NULL, NULL,
                                     0))
            return (DWORD)-1;

        return dwHDSerialNum;
    }
};
}  // namespace akali_hpp
#endif
#endif