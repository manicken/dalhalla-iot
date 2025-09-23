/*
  Dalhalla IoT — JSON-configured HAL/DAL + Script Engine
  HAL = Hardware Abstraction Layer
  DAL = Device Abstraction Layer

  Provides IoT firmware building blocks for home automation and smart sensors.

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include "Logger.h"

void SliceStackReportError(const char* msg, const char* param);

        template<typename T, const char* TypeName>
        struct SliceStack {
            static constexpr const char* Name = TypeName;
            T* data;
            int minIndex;     // beginning of current slice
            int currIndex;    // next free slot
            int stackSize;
            int maxUsage;

            inline SliceStack() = delete;

            inline SliceStack(int _size) {
                data = new T[_size];
                stackSize = _size;
                minIndex = 0;
                currIndex = 0;
                maxUsage = 0;
            }
            inline ~SliceStack() {
                delete[] data;
            }
            // push
            inline void push(const T& v) {
                if (currIndex >= stackSize) {
                    SliceStackReportError("push - stack overflow:",Name);
                    return;
                }
                data[currIndex++] = v;
                if (currIndex > maxUsage)
                    maxUsage = currIndex;
            }

            // pop
            inline T top_n_pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top_n_pop - stack underflow:",Name);
                    return nullptr;
                }
                return data[--currIndex];
            }

            inline void pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                currIndex--;
            }
            inline void collapseTop() {
                if (currIndex - minIndex < 2) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                data[currIndex-2] = data[currIndex-1];
                currIndex--;
            }

            // top
            inline T& top() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top - stack underflow:",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[currIndex - 1];
            }

            // size of *this slice*
            inline int size() const {
                return currIndex - minIndex;
            }

            // check empty
            inline bool empty() const {
                return currIndex == minIndex;
            }

            // check not empty
            inline bool notEmpty() const {
                return currIndex != minIndex;
            }

            inline void ClearCurrSlice() {
                currIndex = minIndex;
            }

            // restore slice (like leaving it)
            inline void Restore(int prevMin, int prevCurr) {
                minIndex = prevMin;
                currIndex = prevCurr;
            }

            // start a nested slice (local context)
            inline void BeginSlice(int& _currMinIndex, int& _currIndex) {
                _currIndex = currIndex;
                _currMinIndex = minIndex;
                minIndex = currIndex;
            }

            inline T& operator[](int idx) const {
                if (idx < 0 || idx >= currIndex - minIndex) {
                    SliceStackReportError("stack out of bounds",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[minIndex + idx];
            }
        };