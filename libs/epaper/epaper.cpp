//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   epaper.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#include "epaper.h"
#include <unistd.h>
#include <cstring>

//#define DEBUG

namespace EPAPER {

// Función de delay en milisegundos
static void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

// Definición de registros
const uint8_t register_data_mid[] = { 0x00, 0x0e, 0x19, 0x02, 0x0f, 0x89 };
const uint8_t register_data_sm[] = { 0x00, 0x0e, 0x19, 0x02, 0xcf, 0x8d };

// Implementación de Spi_t
Spi_t::Spi_t() {
    // SPI ya está inicializado por bcm2835_init() en main
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256); // ~3.125MHz
}

Spi_t::~Spi_t() {
    // No cerrar SPI aquí porque bcm2835_close() lo hace al final
}

uint8_t Spi_t::Transfer1bytes(uint8_t data) {
    return bcm2835_spi_transfer(data);
}

// Implementación de Gpio_t
Gpio_t::Gpio_t(bool enable) : enabled(enable) {}

void Gpio_t::pinMode(uint16_t pin, uint8_t mode) {
    if (pin != NOT_CONNECTED) {
        bcm2835_gpio_fsel(pin, mode);
    }
}

void Gpio_t::digitalWrite(uint16_t pin, uint8_t value) {
    if (pin != NOT_CONNECTED) {
        bcm2835_gpio_write(pin, value);
    }
}

int Gpio_t::digitalRead(uint16_t pin) {
    if (pin != NOT_CONNECTED) {
        return bcm2835_gpio_lev(pin);
    }
    return 0;
}

// Implementación de EPD_Driver
EPD_Driver::EPD_Driver(uint32_t eScreen_EPD, const pins_t& board)
    : Gpio_t(true)
    , spi_ptr(std::make_unique<Spi_t>())
    , pin_cfg_epaper(board)
{
    // Tipo de pantalla
    pdi_cp = (uint16_t)eScreen_EPD;
    pdi_size = (uint16_t)(eScreen_EPD >> 8);

    uint16_t screenSizeV = 0;
    uint16_t screenSizeH = 0;
    [[maybe_unused]] uint16_t screenDiagonal = 0;
    [[maybe_unused]] uint16_t refreshTime = 0;
    
    #ifdef DEBUG
        std::cout << "debugger step 0" << std::endl;
    #endif

    switch (pdi_size) {
        case 0x15: // 1.54"
            screenSizeV = 152;
            screenSizeH = 152;
            screenDiagonal = 154;
            refreshTime = 16;
            break;

        case 0x21: // 2.13"
            screenSizeV = 212;
            screenSizeH = 104;
            screenDiagonal = 213;
            refreshTime = 15;
            break;

        case 0x26: // 2.66"
            screenSizeV = 296;
            screenSizeH = 152;
            screenDiagonal = 266;
            refreshTime = 15;
            break;

        case 0x27: // 2.71"
            screenSizeV = 264;
            screenSizeH = 176;
            screenDiagonal = 271;
            refreshTime = 19;
            break;

        case 0x28: // 2.87"
            screenSizeV = 296;
            screenSizeH = 128;
            screenDiagonal = 287;
            refreshTime = 14;
            break;

        case 0x37: // 3.70"
            screenSizeV = 416;
            screenSizeH = 240;
            screenDiagonal = 370;
            refreshTime = 15;
            break;

        case 0x41: // 4.17"
            screenSizeV = 300;
            screenSizeH = 400;
            screenDiagonal = 417;
            refreshTime = 19;
            break;

        case 0x43: // 4.37"
            screenSizeV = 480;
            screenSizeH = 176;
            screenDiagonal = 437;
            refreshTime = 21;
            break;

        default:
            break;
    }

    // Calcular tamaño de datos de imagen
    image_data_size = (uint32_t)screenSizeV * (uint32_t)(screenSizeH / 8);

    // Configurar registros según tamaño de pantalla
    memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}

int EPD_Driver::digitalRead(int gpio) {
    return Gpio_t::digitalRead(gpio);
}

uint8_t EPD_Driver::hV_HAL_SPI_transfer(uint8_t data) {
    return spi_ptr->Transfer1bytes(data);
}

void EPD_Driver::COG_initial() {
    // Configurar pines
    pinMode(pin_cfg_epaper.panelBusy, INPUT);
    pinMode(pin_cfg_epaper.panelDC, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);
    
    pinMode(pin_cfg_epaper.panelReset, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    
    pinMode(pin_cfg_epaper.panelCS, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    
    if (pin_cfg_epaper.panelON_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelON_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelON_EXT2, HIGH);
    }
    
    if (pin_cfg_epaper.panelSPI43_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelSPI43_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelSPI43_EXT2, LOW);
    }
    
    // Delay inicial
    delay_ms(5);
    
    // Secuencia de reset
    reset(5, 5, 10, 5, 5);
    
    // Soft reset
    softReset();
    
    // Configuración de temperatura
    sendIndexData(0xe5, &register_data[2], 1);  // Input Temperature: 25C
    sendIndexData(0xe0, &register_data[3], 1);  // Active Temperature
    sendIndexData(0x00, &register_data[4], 2);  // PSR
    
    // Configurar dimensiones
    v_screenSizeV = 296;  // vertical = wide size
    v_screenSizeH = 152;  // horizontal = small size
}

void EPD_Driver::sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    // Enviar comando
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    
    hV_HAL_SPI_transfer(index);
    
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    
    // Enviar datos
    for (uint32_t i = 0; i < len; i++) {
        hV_HAL_SPI_transfer(data[i]);
        #ifdef DBG_EPAPER
        std::cout << "Enviando dato: " << static_cast<int>(data[i]) << std::endl;
        #endif
    }
    
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
}

void EPD_Driver::softReset() {
    sendIndexData(0x00, &register_data[1], 1);  // Soft-reset
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
}

void EPD_Driver::reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5) {
    delay_ms(ms1);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    delay_ms(ms2);
    digitalWrite(pin_cfg_epaper.panelReset, LOW);
    delay_ms(ms3);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);
    delay_ms(ms4);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    delay_ms(ms5);
}

void EPD_Driver::DCDC_powerOn() {
    sendIndexData(0x04, &register_data[0], 1);  // Power on
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
}

void EPD_Driver::displayRefresh() {
    sendIndexData(0x12, &register_data[0], 1);  // Display Refresh
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
}

void EPD_Driver::globalUpdate(const uint8_t *data1s, const uint8_t *data2s) {
    // Enviar primer frame
    sendIndexData(0x10, data1s, image_data_size);
    
    #ifdef DBG_EPAPER
    std::cout << "cmd 0x10 size: " << image_data_size << std::endl;
    #endif
    
    // Enviar segundo frame
    sendIndexData(0x13, data2s, image_data_size);
    
    #ifdef DBG_EPAPER
    std::cout << "cmd 0x13 size: " << image_data_size << std::endl;
    #endif
    
    // Encender DC/DC y refrescar
    DCDC_powerOn();
    displayRefresh();
}

void EPD_Driver::COG_powerOff() {
    sendIndexData(0x02, &register_data[0], 0);  // Turn off DC/DC
    
    while (digitalRead(pin_cfg_epaper.panelBusy) != HIGH);
    
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    
    delay_ms(150);
    
    digitalWrite(pin_cfg_epaper.panelReset, LOW);
}

void EPD_Driver::printGpios() {
    std::cout << "========================================" << std::endl;
    std::cout << "Configuración de GPIOs para E-Paper:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Panel BUSY  : GPIO" << pin_cfg_epaper.panelBusy << std::endl;
    std::cout << "Panel DC    : GPIO" << pin_cfg_epaper.panelDC << std::endl;
    std::cout << "Panel RESET : GPIO" << pin_cfg_epaper.panelReset << std::endl;
    std::cout << "Panel CS    : GPIO" << pin_cfg_epaper.panelCS << std::endl;
    std::cout << "Panel ON_EXT2: ";
    if (pin_cfg_epaper.panelON_EXT2 == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.panelON_EXT2 << std::endl;
    }
    std::cout << "Panel SPI43 : ";
    if (pin_cfg_epaper.panelSPI43_EXT2 == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.panelSPI43_EXT2 << std::endl;
    }
    std::cout << "Flash CS    : ";
    if (pin_cfg_epaper.flashCS == NOT_CONNECTED) {
        std::cout << "NOT CONNECTED" << std::endl;
    } else {
        std::cout << "GPIO" << pin_cfg_epaper.flashCS << std::endl;
    }
    std::cout << "========================================" << std::endl;
    std::cout << "SPI Configuración:" << std::endl;
    std::cout << "  - Clock: SCLK (GPIO11)" << std::endl;
    std::cout << "  - MOSI: GPIO10" << std::endl;
    std::cout << "  - MISO: GPIO9 (no usado)" << std::endl;
    std::cout << "========================================" << std::endl;
}

} // namespace EPAPER
