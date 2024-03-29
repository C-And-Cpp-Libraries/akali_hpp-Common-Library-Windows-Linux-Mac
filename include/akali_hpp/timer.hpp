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

#ifndef AKALI_TIMER_HPP_
#define AKALI_TIMER_HPP_
#pragma once

#include "akali_hpp/arch.hpp"

#ifdef AKALI_WIN
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#include <functional>

namespace akali_hpp {
class TimerBase {
   public:
    TimerBase() {
        m_hTimer = NULL;
        m_pTimer = NULL;
    }

    virtual ~TimerBase() {}
    static void CALLBACK TimerProc(void* param, BOOLEAN timerCalled) {
        UNREFERENCED_PARAMETER(timerCalled);
        TimerBase* timer = static_cast<TimerBase*>(param);

        timer->OnTimedEvent();
    }

    // About dwFlags, see:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms682485(v=vs.85).aspx
    //
    BOOL Start(DWORD ulInterval,  // ulInterval in ms
               BOOL bImmediately,
               BOOL bOnce,
               ULONG dwFlags = WT_EXECUTELONGFUNCTION) {
        BOOL bRet = FALSE;

        if (!m_hTimer) {
            bRet = CreateTimerQueueTimer(&m_hTimer, NULL, TimerProc, (PVOID)this,
                                         bImmediately ? 0 : ulInterval, bOnce ? 0 : ulInterval, dwFlags);
        }

        return bRet;
    }

    void Stop(bool bWait) {
        if (m_hTimer) {
            BOOL b = DeleteTimerQueueTimer(NULL, m_hTimer, bWait ? INVALID_HANDLE_VALUE : NULL);
            assert(b);
            m_hTimer = NULL;
        }
    }

    virtual void OnTimedEvent() {}

   private:
    HANDLE m_hTimer;
    PTP_TIMER m_pTimer;
};

template <class T>
class TTimer : public TimerBase {
   public:
    typedef private void (T::*POnTimer)(void);

    TTimer() {
        m_pClass = NULL;
        m_pfnOnTimer = NULL;
    }

    void SetTimedEvent(T* pClass, POnTimer pFunc) {
        m_pClass = pClass;
        m_pfnOnTimer = pFunc;
    }

   protected:
    void OnTimedEvent() override {
        if (m_pfnOnTimer && m_pClass) {
            (m_pClass->*m_pfnOnTimer)();
        }
    }

   private:
    T* m_pClass;
    POnTimer m_pfnOnTimer;
};

class Timer : public TimerBase {
   public:
    typedef std::function<void()> FN_CB;
    Timer() {}

    Timer(FN_CB cb) { SetTimedEvent(cb); }

    void SetTimedEvent(FN_CB cb) { m_cb = cb; }

   protected:
    void OnTimedEvent() override {
        if (m_cb) {
            m_cb();
        }
    }

   private:
    FN_CB m_cb;
};
}  // namespace akali_hpp
#endif
#endif  // !AKALI_WIN_TIMER_HPP_
