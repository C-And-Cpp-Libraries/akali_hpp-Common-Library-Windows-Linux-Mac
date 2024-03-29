﻿/****************************** Module Header ******************************\
* Module Name:  ServiceInstaller.h
* Project:      CppWindowsService
* Copyright (c) Microsoft Corporation.
*
* The file declares functions that install and uninstall the service.
*
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/
#ifndef AKALI_WIN_SERVICE_INSTALLER_HPP_
#define AKALI_WIN_SERVICE_INSTALLER_HPP_
#pragma once
#include "akali_hpp/arch.hpp"
#ifdef AKALI_WIN
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#include <stdio.h>
#include <strsafe.h>

namespace akali_hpp {
class WinServiceInstaller {
   public:
    //
    //   FUNCTION: InstallService
    //
    //   PURPOSE: Install the current application as a service to the local
    //   service control manager database.
    //
    //   PARAMETERS:
    //   * pszServiceName - the name of the service to be installed
    //   * pszDisplayName - the display name of the service
    //   * dwStartType - the service start option. This parameter can be one of
    //     the following values: SERVICE_AUTO_START, SERVICE_BOOT_START,
    //     SERVICE_DEMAND_START, SERVICE_DISABLED, SERVICE_SYSTEM_START.
    //   * pszDependencies - a pointer to a double null-terminated array of null-
    //     separated names of services or load ordering groups that the system
    //     must start before this service.
    //   * pszAccount - the name of the account under which the service runs.
    //   * pszPassword - the password to the account name.
    //
    //   NOTE: If the function fails to install the service, it prints the error
    //   in the standard output stream for users to diagnose the problem.
    //
    static void InstallService(PWSTR pszServiceName,
                               PWSTR pszDisplayName,
                               PWSTR pszDecription,
                               DWORD dwStartType,
                               PWSTR pszDependencies,
                               PWSTR pszAccount,
                               PWSTR pszPassword) {
        wchar_t szPath[MAX_PATH];
        SC_HANDLE schSCManager = NULL;
        SC_HANDLE schService = NULL;

        if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) == 0) {
            wprintf(L"GetModuleFileName failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        // Open the local default service control manager database
        schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
        if (schSCManager == NULL) {
            wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        // Install the service into SCM by calling CreateService
        schService =
            CreateService(schSCManager,               // SCManager database
                          pszServiceName,             // Name of service
                          pszDisplayName,             // Name to display
                          SERVICE_ALL_ACCESS,         //SERVICE_QUERY_STATUS,           // Desired access
                          SERVICE_WIN32_OWN_PROCESS,  // Service type
                          dwStartType,                // Service start type
                          SERVICE_ERROR_NORMAL,       // Error control type
                          szPath,                     // Service's binary
                          NULL,                       // No load ordering group
                          NULL,                       // No tag identifier
                          NULL,                       //pszDependencies,                // Dependencies
                          NULL,                       //pszAccount,                     // Service running account
                          NULL                        //pszPassword                     // Password of the account
            );
        if (schService == NULL) {
            wprintf(L"CreateService failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }
        else {
            // Set service description
            if (pszDecription) {
                SERVICE_DESCRIPTION ServiceDesc;
                ServiceDesc.lpDescription = pszDecription;
                ::ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);
            }
        }

        wprintf(L"%s is installed.\n", pszServiceName);

    Cleanup:
        // Centralized cleanup for all allocated resources.
        if (schSCManager) {
            CloseServiceHandle(schSCManager);
            schSCManager = NULL;
        }
        if (schService) {
            CloseServiceHandle(schService);
            schService = NULL;
        }
    }

    //
    //   FUNCTION: UninstallService
    //
    //   PURPOSE: Stop and remove the service from the local service control
    //   manager database.
    //
    //   PARAMETERS:
    //   * pszServiceName - the name of the service to be removed.
    //
    //   NOTE: If the function fails to uninstall the service, it prints the
    //   error in the standard output stream for users to diagnose the problem.
    //
    static void UninstallService(PWSTR pszServiceName) {
        SC_HANDLE schSCManager = NULL;
        SC_HANDLE schService = NULL;
        SERVICE_STATUS ssSvcStatus = {};

        // Open the local default service control manager database
        schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (schSCManager == NULL) {
            wprintf(L"OpenSCManager failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        // Open the service with delete, stop, and query status permissions
        schService =
            OpenService(schSCManager, pszServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
        if (schService == NULL) {
            wprintf(L"OpenService failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        // Try to stop the service
        if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
            wprintf(L"Stopping %s.", pszServiceName);
            Sleep(1000);

            while (QueryServiceStatus(schService, &ssSvcStatus)) {
                if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                    wprintf(L".");
                    Sleep(1000);
                }
                else
                    break;
            }

            if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED) {
                wprintf(L"\n%s is stopped.\n", pszServiceName);
            }
            else {
                wprintf(L"\n%s failed to stop.\n", pszServiceName);
            }
        }

        // Now remove the service by calling DeleteService.
        if (!DeleteService(schService)) {
            wprintf(L"DeleteService failed w/err 0x%08lx\n", GetLastError());
            goto Cleanup;
        }

        wprintf(L"%s is removed.\n", pszServiceName);

    Cleanup:
        // Centralized cleanup for all allocated resources.
        if (schSCManager) {
            CloseServiceHandle(schSCManager);
            schSCManager = NULL;
        }
        if (schService) {
            CloseServiceHandle(schService);
            schService = NULL;
        }
    }

    static void StartService(PWSTR pszServiceName) {
        if (NULL == pszServiceName)
            return;

        SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, GENERIC_EXECUTE);
        if (schSCManager == NULL)
            return;

        SC_HANDLE schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
        if (schService == NULL)
            return;

        SERVICE_STATUS status;
        QueryServiceStatus(schService, &status);

        if (status.dwCurrentState == SERVICE_STOPPED)
            ::StartService(schService, 0, NULL);

        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
    }

    static void StopService(PWSTR pszServiceName) {
        if (NULL == pszServiceName)
            return;

        SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, GENERIC_EXECUTE);
        if (schSCManager == NULL)
            return;

        SC_HANDLE schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
        if (schService == NULL)
            return;

        SERVICE_STATUS status;
        QueryServiceStatus(schService, &status);

        if (status.dwCurrentState == SERVICE_RUNNING)
            ControlService(schService, SERVICE_CONTROL_STOP, &status);

        CloseServiceHandle(schSCManager);
        CloseServiceHandle(schService);
    }
};
}  // namespace akali_hpp
#endif
#endif