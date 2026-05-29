//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   tyme.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#include "tyme.h"
#include <unistd.h>
#include <bcm2835.h>

namespace TYME {

// Implementación de delay en milisegundos
void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void delay_us(uint32_t us) {
    usleep(us);
}

void delay_s(uint32_t s) {
    sleep(s);
}

void delay(uint32_t ms) {
    delay_ms(ms);
}

void bcm2835_delay(int ms) {
    delay_ms(ms);
}

void delayMicroseconds(uint64_t us) {
    bcm2835_delayMicroseconds(us);
}

void wait_ms(uint32_t ms) {
    delay_ms(ms);
}

void wait_us(uint32_t us) {
    delay_us(us);
}

} // namespace TYME
