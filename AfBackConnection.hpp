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


#include "HMCPConnection.hpp"


namespace lr {
namespace lcd {
        

/// A conection using the I2C/SPI character LCD backpack from Adafruit.
///
/// The backpack has to be used in I2C mode. You can select the address
/// and communication channel for the chip in the constructor
/// of the MCP23008 class.
///
/// Link: https://www.adafruit.com/product/292
///
/// The weak 47k pull-up resistors on the board do not allow higher
/// bus speeds then 100kHz. Use a level shifter and stronger pull-ups
/// for 400kHz communication.
///
/// @note Do not call the `initialize()` function. This function will
///    be called in the display driver.
///
class AfBackConnection : public HMCPConnection<MCP23008::Pin::GPA1, MCP23008::Pin::GPA2, MCP23008::Pin::GPA7, 3>
{
public:
    /// Create a new instance for the connection.
    ///
    inline AfBackConnection(MCP23008 *io) : HMCPConnection(io) {}
};

        
}
}

