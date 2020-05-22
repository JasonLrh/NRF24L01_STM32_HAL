#ifndef NRF24L01P_H
#define NRF24L01P_H

#include "main.h"
#include "spi.h"

/* config defines */
// from 3 to 5 bytes width
#define NRF_RFCHANNEL_ADDR_LENS_IN_BYTE 5


// calculate defines
#ifndef NRF_RFCHANNEL_ADDR_LENS_IN_BYTE
#error "define NRF_RFCHANNEL_ADDR_LENS_IN_BYTE to limit address width"
#endif

#if NRF_RFCHANNEL_ADDR_LENS_IN_BYTE < 3 || NRF_RFCHANNEL_ADDR_LENS_IN_BYTE > 5
#error "NRF_RFCHANNEL_ADDR_LENS_IN_BYTE should be set (3,4,5)"
#endif

#define SETUP_AW_VAL NRF_RFCHANNEL_ADDR_LENS_IN_BYTE - 2

// public structs
typedef enum _nrf_err_t{
    NRF_OK = 0,
    NRF_ERR_arg = -1,
    NRF_ERR_permision = -2,
    NRF_ERR_IO = -3,
    NRF_ERR_undefine = -4,
}nrf_err_t;

typedef enum _nrf_mode_e{
    NRF_MODE_Power_Off = 0,
    NRF_MODE_Sleep_1 = 1,
    NRF_MODE_Sleep_2 = 2,   // can't use changeMode to get in this mode
    NRF_MODE_Tx = 3,        // changeMode may lead staus to Sleep_2 
    NRF_MODE_Rx = 4
}nrf_mode_e;

typedef enum _nrf_irq_type_e{
    NRF_IRQ_TX,
    NRF_IRQ_RX,
    NRF_IRQ_MAXRT
}nrf_irq_type_e;

//Define the register address for nRF24L01PP
#define nRF_CONFIG									0x00  // Configurate the status of transceiver, mode of CRC and the replay of transceiver status
#define nRF_EN_AA										0x01  // Enable the atuo-ack in all channels
#define nRF_EN_RXADDR								0x02  // Enable Rx Address
#define nRF_SETUP_AW								0x03  // Configurate the address width
#define nRF_SETUP_RETR							0x04  // Setup the retransmit
#define nRF_RF_CH										0x05  // Configurate the RF frequency
#define nRF_RF_SETUP								0x06  // Setup the rate of data, and transmit power
#define nRF_STATUS									0x07  //
#define nRF_OBSERVE_TX							0x08  //
#define nRF_CD											0x09  // Carrier detect
#define nRF_RX_ADDR_P0							0x0A  // Receive address of channel 0
#define nRF_RX_ADDR_P1							0x0B  // Receive address of channel 1
#define nRF_RX_ADDR_P2							0x0C  // Receive address of channel 2
#define nRF_RX_ADDR_P3							0x0D  // Receive address of channel 3
#define nRF_RX_ADDR_P4							0x0E  // Receive address of channel 4
#define nRF_RX_ADDR_P5							0x0F  // Receive address of channel 5
#define nRF_TX_ADDR									0x10  //       Transmit address
#define nRF_RX_PW_P0								0x11  // Size of receive data in channel 0
#define nRF_RX_PW_P1								0x12  // Size of receive data in channel 1
#define nRF_RX_PW_P2								0x13  // Size of receive data in channel 2
#define nRF_RX_PW_P3								0x14  // Size of receive data in channel 3
#define nRF_RX_PW_P4								0x15  // Size of receive data in channel 4
#define nRF_RX_PW_P5								0x16  // Size of receive data in channel 5
#define nRF_FIFO_STATUS							0x17  // FIFO Status
#define nRF_DYNPD										0x1C  // Dynamic Payload State for pipes
#define nRF_FEATURE									0x1D  // FEATURE Register


/* API function */
nrf_err_t nrf_load_tx(uint8_t * data,uint8_t size);
nrf_err_t nrf_clear_tx_fifo(void);
nrf_err_t nrf_clear_rx_fifo(void);
nrf_err_t nrf_config_tx_address(uint32_t addr);
// int nrf_change_device_mode(nrf_mode_e mod);
nrf_err_t nrf_config_rx_channel(uint8_t channel, uint32_t addr, uint8_t pack_size,uint8_t * buffer_point);
nrf_err_t nrf_config_rx_buffer(uint8_t channel,uint8_t * buffer_point);
nrf_err_t nrf_set_enable(nrf_mode_e mod);
nrf_mode_e nrf_get_mode(void);

/* special function :  be patient */
// put this funtion in IRQ pin's interrupt handle function;
int nrf_interrupt_handler(void);

#endif
