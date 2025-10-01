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

#pragma once

namespace HAL_JSON {
    namespace ScriptEngine {
        /**
         * @brief Parses a buffer into tokens while stripping comments and tracking line/column positions.
         *
         * This function performs a single pass over the input buffer:
         * - Skips both `// line` and C-style block comments.
         * - Skips whitespace while tracking line and column numbers.
         * - Splits the text into tokens separated by whitespace.
         * - Optionally fills an array of Token objects with start/end pointers and source position info.
         *
         * @param buffer    The modifiable input buffer (null-terminated). Comments are replaced with spaces.
         * @param tokens    Optional pointer to an array of Token objects. If `nullptr`, only token counting is performed.
         * @param maxCount  Maximum number of tokens that can be written to `tokens`.
         * @return          Number of tokens found, or -1 if `tokens` is not `nullptr` and more than `maxCount` tokens were detected.
         *
         * @note The function does not allocate memory; it works directly on the provided buffer.
         * @note If `tokens` is provided, the caller must ensure it has at least `maxCount` capacity.
         * @note If only the count is needed, call with `tokens = nullptr` and `maxCount = 0`.
         */
        template <typename T>
        int ParseAndTokenize(char* buffer, T* tokens=nullptr, int maxCount=-1);
    }
}