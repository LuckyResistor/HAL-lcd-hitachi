#pragma once
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


#include "HConnection.hpp"

#include "hal-common/Timer.hpp"
#include "hal-mcp230xx/MCP23008.hpp"


namespace lr {
namespace lcd {


/// Connection to the chip using a MCP23008 chip and I2C, using 4bit data.
///
/// This is a template class which automatically generates fast code for
/// any used bit configuration.
///
/// The chip and connection have to be initialized before the `initialize()`
/// method is called. The `initialize()` method will just configure the
/// pins used for the display and leave the spare pin untouched. Do not
/// call `initialize()` on this instance manually, the display driver
/// will do this in it's own initialization.
///
/// Best is to operate the I2C bus to the IO chip at least at
/// 400kHz for an efficient communication with the display.
///
/// The last pin on the chip can not be used with this implementation.
/// For performance reasons, direct writes to OLAT are used.
///
/// @note Do not call the `initialize()` function. This function will
///    be called in the display driver.
///
/// @tparam tRsPin The pin for register select line.
/// @tparam tEnPin The pin for the enable line.
/// @tparam tLightPin The bit to enable the background light.
/// @tparam tDataBit The first bit for the 4bit data pins.
///    Please note, This is the bit number, which is equal to the pin number.
///    The number equals the pin number.
///    This first bit has to be connected to the DB4 line. The next
///    sequentially bits connected to DB5, DB6 and DB7.
///
template<MCP23008::Pin tRsPin, MCP23008::Pin tEnPin, MCP23008::Pin tLightPin, uint8_t tDataBit>
class HMCPConnection : public HConnection
{
public:
    /// Create a new connection.
    ///
    explicit constexpr HMCPConnection(MCP23008 *io) : _io(io) {}
    
private:
    /// Get the mask for the data pins.
    ///
    constexpr static MCP23008::PinMask dataMask() {
        return MCP23008::PinMask::fromMask(static_cast<uint8_t>(0b1111)<<tDataBit);
    }

    /// Shift the data bits to the right location and create a mask from it.
    ///
    constexpr static MCP23008::PinMask dataMaskFromValue(uint8_t value) {
        return MCP23008::PinMask::fromMask(static_cast<uint8_t>(value)<<tDataBit);
    }
    
    /// Get the mask for all used pins.
    ///
    constexpr static MCP23008::PinMask pinMask() {
        return dataMask()|tRsPin|tEnPin|tLightPin;
    }
    
    /// Send four bits.
    ///
    Status sendBits(uint8_t data) {
        _currentOutput.setFlag(tEnPin);
        _currentOutput.changeFlags(dataMaskFromValue(data), dataMask());
        if (hasError(_io->setAllOutputs(_currentOutput))) {
            return Status::Error;
        }
        Timer::delay(1_us);
        _currentOutput.clearFlag(tEnPin);
        if (hasError(_io->setAllOutputs(_currentOutput))) {
            return Status::Error;
        }
        Timer::delay(50_us);
        return Status::Success;
    }
    
public: // Implement HConnection
    Status initialize() override {
        // Setup the pin configuration.
        if (hasError(_io->setPullUps(pinMask(), MCP23008::PullUp::Disabled))) {
            return Status::Error;
        }
        if (hasError(_io->setDirections(pinMask(), MCP23008::Direction::Output))) {
            return Status::Error;
        }
        // Start with low states and make sure we wait long enough to the internal reset.
        if (hasError(_io->setAllOutputs(_currentOutput))) {
            return Status::Error;
        }
        Timer::delay(20_ms);
        // Make sure the display is initialized in 4bit mode.
        if (hasError(sendBits(0b0011))) {
            return Status::Error;
        }
        Timer::delay(4_ms);
        if (hasError(sendBits(0b0011))) {
            return Status::Error;
        }
        Timer::delay(100_us);
        if (hasError(sendBits(0b0011))) { // Now the display is in 8bit mode.
            return Status::Error;
        }
        if (hasError(sendBits(0b0010))) { // This will set it into the 4bit mode.
            return Status::Error;
        }
        // This is
        return Status::Success;
    }
    
    Status sendCommand(uint8_t command) override {
        _currentOutput.clearFlag(tRsPin);
        sendBits(command >> 4);
        sendBits(command & 0b00001111);
        return Status::Success;
    }
    
    Status sendData(uint8_t data) override {
        _currentOutput.setFlag(tRsPin);
        sendBits(data >> 4);
        sendBits(data & 0b00001111);
        return Status::Success;
    }
    
    Status setBacklightEnabled(bool enabled) override {
        if (enabled) {
            _currentOutput.setFlag(tLightPin);
        } else {
            _currentOutput.clearFlag(tLightPin);
        }
        if (hasError(_io->setAllOutputs(_currentOutput))) {
            return Status::Error;
        }
        return Status::Success;
    }

private:
    MCP23008 *_io; ///< The IO interface.
    MCP23008::PinMask _currentOutput; ///< The current output on the chip.
};


}
}

