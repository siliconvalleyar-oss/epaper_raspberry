//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   tyme.h
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unistd.h>
#include <bcm2835.h>

namespace TYME {

// Prototipos de funciones
void delay_ms(uint32_t ms);
void delay_us(uint32_t us);
void delay_s(uint32_t s);
void delay(uint32_t ms);
void bcm2835_delay(int ms);
void delayMicroseconds(uint64_t us);
void wait_ms(uint32_t ms);
void wait_us(uint32_t us);

} // namespace TYME
