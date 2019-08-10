//
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
#include "HDisplay.hpp"


#include "HConnection.hpp"

#include "hal-common/Timer.hpp"


namespace lr {
namespace lcd {


HDisplay::HDisplay(
    HConnection* const connection,
    const uint8_t layoutRows,
    const uint8_t layoutColumns)
:
    _connection(connection),
    _layoutColumns(layoutColumns),
    _layoutRows(layoutRows)
{
}

    
HDisplay::Status HDisplay::initialize()
{
    // First initialize the connection.
    if (hasError(_connection->initialize())) return Status::Error;

    // Send commands to initialize the display.
    CommandMask cmd = Command::Function;
    if (isTwoLineMode()) {
        cmd |= Command::FunctionTwoLines;
    }
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    cmd = Command::EntryMode;
    cmd |= Command::EntryModeIncrement;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    // Disable the display.
    cmd = Command::Enable;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    // Clear the dispplay
    cmd = Command::Clear;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    Timer::delay(3_ms);
    // Set the cursor to the home position.
    cmd = Command::Home;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    Timer::delay(3_ms);
    // Enable the display.
    cmd = Command::Enable;
    cmd |= Command::EnableDisplay;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    // Initialize the state.
    _state.increment = true;
    _state.autoShift = false;
    _state.displayEnabled = true;
    _state.cursorVisible = false;
    _state.cursorBlinks = false;
    return Status::Success;
}


bool HDisplay::isTwoLineMode() const
{
    return _layoutRows > 1;
}

    
uint8_t HDisplay::getAddressForPosition(uint8_t x, uint8_t y)
{
    // This implementation should work for most common displays, but it
    // is virtual, so it can be changed for displays.
    
    // Check and limit the values into a valid range.
    if (_layoutRows == 1 && x >= 80) {
        x = 79;
    } else if (_layoutRows <= 2 && x >= 40) {
        x = 39;
    } else if (_layoutRows > 2 && x >= _layoutColumns) {
        x = _layoutColumns-1;
    }
    if (y >= _layoutRows) {
        y = _layoutRows-1;
    }
    
    // In case of a one line display, it is simple.
    if (_layoutRows == 1) {
        return x;
    } else if (_layoutRows == 2) {
        return x + (y == 0 ? 0 : 40);
    } else {
        // For 4 line displays, lines 3+4 are an extension of the first two lines.
        return x + ((y&1)==0 ? 0 : 40) + ((y&2)==0 ? 0 : _layoutColumns);
    }
}

    
HDisplay::Status HDisplay::reset()
{
    // Just call the other methods.
    if (hasError(clear())) return Status::Error;
    if (hasError(cursorReset())) return Status::Error;
    if (hasError(setEnabled(true))) return Status::Error;
    if (hasError(setCursorMode(CursorMode::Off))) return Status::Error;
    if (hasError(setWritingDirection(WritingDirection::LeftToRight))) return Status::Error;
    if (hasError(setAutoScrollEnabled(false))) return Status::Error;
    return Status::Success;
}

    
HDisplay::Status HDisplay::clear()
{
    CommandMask cmd = Command::Clear;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    Timer::delay(3_ms);
    return Status::Success;
}

    
HDisplay::Status HDisplay::cursorReset()
{
    CommandMask cmd = Command::Home;
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    Timer::delay(3_ms);
    return Status::Success;
}

    
HDisplay::Status HDisplay::setCursor(uint8_t x, uint8_t y)
{
    CommandMask cmd = Command::DDAddress;
    cmd |= CommandMask::fromMask(getAddressForPosition(x, y));
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    return Status::Success;
}

    
HDisplay::Status HDisplay::writeChar(char c)
{
    _connection->sendData(c);
    return Status::Success;
}

    
HDisplay::Status HDisplay::writeText(const String &text)
{
    for (String::Size i = 0; i < text.getLength(); ++i) {
        writeChar(text.getCharAt(i));
    }
    return Status::Success;
}

    
HDisplay::Status HDisplay::sendEnabledCommand()
{
    CommandMask cmd = Command::Enable;
    if (_state.displayEnabled) {
        cmd |= Command::EnableDisplay;
    }
    if (_state.cursorVisible) {
        cmd |= Command::EnableCursor;
    }
    if (_state.cursorBlinks) {
        cmd |= Command::EnableBlink;
    }
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    return Status::Success;
}

    
HDisplay::Status HDisplay::setEnabled(bool enabled)
{
    _state.displayEnabled = enabled;
    return sendEnabledCommand();
}

    
HDisplay::Status HDisplay::setCursorMode(CursorMode mode)
{
    _state.cursorVisible = (mode != CursorMode::Off);
    _state.cursorBlinks = (mode == CursorMode::Block);
    return sendEnabledCommand();
}

    
HDisplay::Status HDisplay::setBacklightEnabled(bool enabled)
{
    if (hasError(_connection->setBacklightEnabled(enabled))) return Status::Error;
    return Status::Success;
}

    
HDisplay::Status HDisplay::sendEntryModeCommand()
{
    CommandMask cmd = Command::EntryMode;
    if (_state.increment) {
        cmd |= Command::EntryModeIncrement;
    }
    if (_state.autoShift) {
        cmd |= Command::EntryModeShift;
    }
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    return Status::Success;
}

    
HDisplay::Status HDisplay::setWritingDirection(WritingDirection writingDirection)
{
    _state.increment = (writingDirection == WritingDirection::LeftToRight);
    return sendEntryModeCommand();
}

    
HDisplay::Status HDisplay::setAutoScrollEnabled(bool enabled)
{
    _state.autoShift = enabled;
    return sendEntryModeCommand();
}

    
HDisplay::Status HDisplay::scroll(ScrollDirection scrollDirection)
{
    CommandMask cmd = Command::Shift|Command::ShiftDisplay;
    if (scrollDirection == ScrollDirection::Right) {
        cmd |= Command::ShiftRight;
    }
    if (hasError(_connection->sendCommand(cmd))) return Status::Error;
    return Status::Success;
}


}
}
