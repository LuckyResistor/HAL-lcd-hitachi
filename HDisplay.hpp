#pragma once
//
// The HAL to communicate with a Hitachi HD44780 compatible display.
// ---------------------------------------------------------------------------
// (c)2019 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include "hal-common/Flags.hpp"
#include "hal-lcd-character/CharacterDisplay.hpp"


namespace lr {
namespace lcd {

    
class HConnection;


/// The HAL to communicate with a Hitachi HD44780 compatible display.
///
/// This implementation assumes one controller chip for a display.
/// It supports all regular displays with up to 4 lines and 40 columns
/// using a single driver chip for the display.
///
/// It assumes a character memory of 80 characters which is mapped
/// in one of the common memory configurations.
///
/// Writing to the display will automatically use the full memory
/// available, including not visible areas - so you can shift the
/// display to reveal the hidden text.
///
/// Displays with four lines do not support the hidden areas, because
/// the memory layout is not suitable for shifting.
///
class HDisplay : public CharacterDisplay
{
public:
    /// Create a new instance
    ///
    /// The parameter `layoutRows` and `layoutColumns` are used to calculate the
    /// correct cursor positions in the display. You are not free in defining
    /// them, they have to match the display layout to get the correct RAM address
    /// for writing to the display.
    ///
    /// @param connection The connection to the display.
    /// @param layoutRows The number of rows of the display (1,2 or 4).
    /// @param layoutColumns The number of columns of the display (8-40).
    ///
    HDisplay(
        HConnection* const connection,
        const uint8_t layoutRows,
        const uint8_t layoutColumns);

public:
    /// Initialize the display.
    ///
    /// This will initialize the connection to the display and
    /// set the display into the default state (as `reset()` does).
    ///
    /// @return The status of the call.
    ///
    Status initialize();

public: // Implement CharacterDisplay.
    Status reset() override;
    Status clear() override;
    Status cursorReset() override;
    Status setCursor(uint8_t x, uint8_t y) override;
    Status writeChar(char c) override;
    Status writeText(const String &text) override;
    Status setEnabled(bool enabled) override;
    Status setCursorMode(CursorMode mode) override;
    Status setBacklightEnabled(bool enabled) override;
    Status setWritingDirection(WritingDirection writingDirection) override;
    Status setAutoScrollEnabled(bool enabled) override;
    Status scroll(ScrollDirection scrollDirection) override;

public: // Low-Level interface.
    /// The commands and flags for the display.
    ///
    enum class Command : uint8_t {
        Clear = (1<<0),
        Home = (1<<1),
        EntryMode = (1<<2),
        EntryModeIncrement = (1<<1),
        EntryModeShift = (1<<0),
        Enable = (1<<3),
        EnableDisplay = (1<<2),
        EnableCursor = (1<<1),
        EnableBlink = (1<<0),
        Shift = (1<<4),
        ShiftDisplay = (1<<3),
        ShiftRight = (1<<2),
        Function = (1<<5),
        FunctionTwoLines = (1<<3),
        Function11Dots = (1<<2),
        CGAddress = (1<<6),
        DDAddress = (1<<7)
    };
    LR_DECLARE_FLAGS(Command, CommandMask);

protected:
    /// Send the enabled command with the current state.
    ///
    Status sendEnabledCommand();
    
    /// Send the entry mode command with the current state.
    ///
    Status sendEntryModeCommand();
    
    /// Check if the two line mode shall be acticated.
    ///
    virtual bool isTwoLineMode() const;
    
    /// Get the address for a cursor location.
    ///
    virtual uint8_t getAddressForPosition(uint8_t x, uint8_t y);
    
protected:
    HConnection* const _connection; ///< The connection to the display.
    const uint8_t _layoutColumns; ///< The number of columns of the display.
    const uint8_t _layoutRows; ///< The number of rows of the display.
    struct {
        bool increment : 1; ///< If increment is enabled.
        bool autoShift : 1; ///< If auto shift is enabled.
        bool displayEnabled : 1; ///< If the display is enabled.
        bool cursorVisible : 1; ///< If the cursor is visible.
        bool cursorBlinks : 1; ///< If the cursor blinks/is a block.
    } _state; ///< The state of the display.
};

    
LR_DECLARE_OPERATORS_FOR_FLAGS(HDisplay::CommandMask);
    

}
}

