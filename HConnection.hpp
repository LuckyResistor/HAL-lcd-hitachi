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


#include "hal-common/StatusTools.hpp"


namespace lr {
namespace lcd {
        

/// The interface to a Hitachi HD44780 compatible display.
///
/// There are no read methods, because for most applications the
/// R/W connection is not used.
///
/// @note Do not call the `initialize()` function. This function will
///    be called in the display driver.
///
class HConnection
{
public:
    /// The status of the calls.
    ///
    using Status = CallStatus;
    
public:
    /// Initialize the connection.
    ///
    /// Setup the connection and the communication with the display.
    ///
    /// @return The status of the call.
    ///
    virtual Status initialize() = 0;
        
    /// Send a command to the display.
    ///
    /// @param comment The raw command data.
    /// @return The status of the call.
    ///
    virtual Status sendCommand(uint8_t command) = 0;
    
    /// Send data to the display.
    ///
    /// @param data The data byte to send.
    /// @return The status of the call.
    ///
    virtual Status sendData(uint8_t data) = 0;
    
    /// Enable the light.
    ///
    /// @param enabled `true` to enable the light.
    /// @return The status of the call.
    ///
    virtual Status setBacklightEnabled(bool enabled) = 0;
};
    

}
}

