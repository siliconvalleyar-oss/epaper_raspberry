//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   boards.h
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <epaper/epaper.h>
#include <string>

namespace EPAPER {

// Configuración para Raspberry Pi 2W/3B/4B (BCM numbering)
const pins_t boardRaspberryPi = {
    .panelBusy        = 27,    // GPIO27 - Pin 13 - Estado ocupado
    .panelDC          = 18,    // GPIO18 - Pin 12 - Data/Command
    .panelReset       = 17,    // GPIO17 - Pin 11 - Reset
    .panelCS          = 8,     // GPIO8  - Pin 24 - Chip Select (CE0)
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Flash CS
};

// Configuración para Raspberry Pi Zero 2W
const pins_t boardRaspberryPiZero2W = {
    .panelBusy        = 25,    // GPIO25 - Pin 22 - Estado ocupado
    .panelDC          = 24,    // GPIO24 - Pin 18 - Data/Command
    .panelReset       = 23,    // GPIO23 - Pin 16 - Reset
    .panelCS          = 8,     // GPIO8  - Pin 24 - Chip Select (CE0)
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15 - Flash CS
};

// Configuración para Raspberry Pi 4/5
const pins_t boardRaspberryPi4 = {
    .panelBusy        = 27,    // GPIO27 - Pin 13
    .panelDC          = 18,    // GPIO18 - Pin 12
    .panelReset       = 17,    // GPIO17 - Pin 11
    .panelCS          = 8,     // GPIO8  - Pin 24
    .panelON_EXT2     = NOT_CONNECTED,
    .panelSPI43_EXT2  = NOT_CONNECTED,
    .flashCS          = 22     // GPIO22 - Pin 15
};

// Función auxiliar para obtener configuración por modelo
inline const pins_t& getBoardConfig(const std::string& model = "RaspberryPi") {
    if (model == "RaspberryPiZero2W") {
        return boardRaspberryPiZero2W;
    } else if (model == "RaspberryPi4" || model == "RaspberryPi5") {
        return boardRaspberryPi4;
    }
    return boardRaspberryPi;
}

} // namespace EPAPER
