/*
 * LunarGravity - gravityclockwin32.hpp
 *
 * Copyright (C) 2017 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <time.h>

#include "gravityclock.hpp"

class GravityClockWin32 : public GravityClock {
public:
    GravityClockWin32() : GravityClock() { }

    virtual void Start() {
        QueryPerformanceFrequency(&m_frequency);
        m_last_comp_time = GrabCurrentTime();
    }

    virtual void StartGameTime() {
        m_last_game_time = GrabCurrentTime();
        m_paused = false;
    }

    virtual void GetTimeDiffMS(float &comp_diff, float &game_diff) {
        LARGE_INTEGER current;
        LARGE_INTEGER elapsed;
        while (true) {
            current = GrabCurrentTime();
            elapsed.QuadPart = current.QuadPart - m_last_comp_time.QuadPart;
            elapsed.QuadPart *= 1000000;
            elapsed.QuadPart /= m_frequency.QuadPart;
            m_last_comp_time = current;
            comp_diff = (float)elapsed.QuadPart;

            if (!m_paused) {
                elapsed.QuadPart = current.QuadPart - m_last_game_time.QuadPart;
                elapsed.QuadPart *= 1000000;
                elapsed.QuadPart /= m_frequency.QuadPart;
                m_last_game_time = current;
                game_diff = (float)elapsed.QuadPart;
            }
        }
    }
    
private:

    LARGE_INTEGER m_last_comp_time;
    LARGE_INTEGER m_last_game_time;
    LARGE_INTEGER m_frequency;

    LARGE_INTEGER GrabCurrentTime(void) {
        LARGE_INTEGER current_time;
        QueryPerformanceCounter(&current_time);
        return current_time;
    }
};
