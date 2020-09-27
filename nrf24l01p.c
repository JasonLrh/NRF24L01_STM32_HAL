#include "nrf24l01p.h"

// imagratable chapter
#define CSN(x) HAL_GPIO_WritePin(CSN_GPIO_Port,CSN_Pin,(x==0)?GPIO_PIN_RESET:GPIO_PIN_SET)
#define CE(x) HAL_GPIO_WritePin(CEN_GPIO_Port,CEN_Pin,(x==0)?GPIO_PIN_RESET:GPIO_PIN_SET)

#define delay(x) HAL_Delay(x)

#define write(data,size) (int)HAL_SPI_Transmit(&hspi1,data,size,HAL_MAX_DELAY)
#define read(data,size) (int)HAL_SPI_Transmit(&hspi1,data,size,HAL_MAX_DELAY)

//do not suggest to edit below (unless you know what you are doing)

/* private define */
typedef enum _nrf_cmd_e{
    NRF_CMD_R_RX = 0x61,
    NRF_CMD_W_TX = 0xa0,
    NRF_CMD_CLEAR_TX = 0xe1,
    NRF_CMD_CLEAR_RX = 0xe2,
    NRF_CMD_REUSE_TX = 0xe3,
    NRF_CMD_NOP = 0xff
}nrf_cmd_e;

/* private vars */
static volatile nrf_mode_e nrf_mode = NRF_MODE_Power_Off;
static uint8_t pip_width[5]={0,0,0,0,0};
static uint8_t * rx_buffer_point[5] = {NULL,NULL,NULL,NULL,NULL};

/* private functions */
static nrf_err_t read_reg(uint8_t reg,uint8_t * data,uint8_t size){
    nrf_err_t ret = NRF_OK;
    reg &= 0x1f;
    CSN(0);
    ret |= write(&reg,1);
    if(ret){
        CSN(1);
        return NRF_ERR_IO;
    }
    ret |= read(data,size);
    CSN(1);
    if (ret){
        return NRF_ERR_IO;
    }else{
        return NRF_OK;
    }
}

static nrf_err_t write_reg(uint8_t reg,uint8_t * data,uint8_t size){
    if(nrf_mode > 2){
        return NRF_ERR_permision;
    }
    nrf_err_t ret = NRF_OK;
    reg &= 0x1f;
    reg |= 0x20;
    CSN(0);
    ret |= write(&reg,1);
    if(ret){
        CSN(1);
        return NRF_ERR_IO;
    }
    ret |= write(data,size);
    CSN(1);
    if (ret){
        return NRF_ERR_IO;
    }else{
        return NRF_OK;
    }
}

/* API function */
// simple function to load tx fifo
nrf_err_t nrf_load_tx(uint8_t * data,uint8_t size){
    if(size == 0 || size > 32){
        return NRF_ERR_arg;
    }
    nrf_err_t ret = NRF_OK;
    uint8_t cmd = (uint8_t)NRF_CMD_W_TX;
    CSN(0);
    ret = write(&cmd,1);
    if(ret){
        CSN(1);
        return NRF_ERR_IO;
    }
    nrf_mode = NRF_MODE_Tx;
    ret = write(data,size);
    CSN(1);
    if(ret){
        return NRF_ERR_IO;
    }else{
        return NRF_OK;
    }
}

nrf_err_t nrf_clear_tx_fifo(void){
    uint8_t cmd = (uint8_t)NRF_CMD_CLEAR_TX;
    CSN(0);
    nrf_err_t ret = write(&cmd,1);
    CSN(1);
    if( ret ){
        return NRF_ERR_IO;
    }else{
        return NRF_OK;
    }
}

nrf_err_t nrf_clear_rx_fifo(void){
    uint8_t cmd = (uint8_t)NRF_CMD_CLEAR_RX;
    CSN(0);
    nrf_err_t ret = write(&cmd,1);
    CSN(1);
    return ret;
}

// change Tx address
nrf_err_t nrf_config_tx_address(uint32_t addr){
    uint8_t addr_buf[3];
    int i = 0;
    for (i = 0;i<3;i++,addr >>= 8){
        addr_buf[i] = (uint8_t)addr;
    }
    return write_reg(nRF_TX_ADDR,addr_buf,3);
}


// status machine manager
static nrf_err_t nrf_change_device_mode(nrf_mode_e mod){
    nrf_err_t ret = NRF_OK;
    if(mod == nrf_mode){//mo need to change
        return NRF_OK;
    }else if(mod == NRF_MODE_Sleep_2){//this mode can't be set directly
        return NRF_ERR_arg;
    }else if(mod == NRF_MODE_Power_Off){//the way every mode to this mode is same
        uint8_t CMD = 0x00;
        CE(0);
        ret = write_reg(nRF_CONFIG,&CMD,1);
        nrf_mode = NRF_MODE_Power_Off;
        return ret;
    }
    if(nrf_mode > 2){
        if (mod > 2){
            CE(0);
            delay(1);
            uint8_t CMD = 0x0a | (mod == NRF_MODE_Rx)?0x01:0x00;
            ret = write_reg(nRF_CONFIG,&CMD,1);
            CE(1);
            nrf_mode = (mod==NRF_MODE_Tx)?NRF_MODE_Sleep_2:NRF_MODE_Rx;
            return ret;
        }else{
            CE(0);
            nrf_mode = NRF_MODE_Sleep_1;
            return NRF_OK;
        }
    }else if(nrf_mode == NRF_MODE_Power_Off){
        uint8_t CMD = 0x0a | (mod == NRF_MODE_Rx)?0x01:0x00;
        ret = write_reg(nRF_CONFIG,&CMD,1);
        if(mod > 2){
            CE(1);
            nrf_mode = (mod == NRF_MODE_Tx)?NRF_MODE_Sleep_2:mod;
        }else{
            nrf_mode = NRF_MODE_Sleep_1;
        }
        return ret;
    }
    return 0;
}

/*
channel:rx chn number range 0~5
address:address:
    channel 0,1 use addr[24 bit LSB]
    channel 2~5 use addr[8 bit LSB]
pack_size:rx payload
*/
nrf_err_t nrf_config_rx_channel(uint8_t channel, uint32_t addr, uint8_t pack_size,uint8_t * buff_point){
    if (channel > 5){
        return -6;
    }
    if(buff_point != NULL){
        rx_buffer_point[channel] = buff_point;
    }else{
        return -10;
    }
    if(pack_size == 0 || pack_size > 32){
        return -1;
    }
    pip_width[channel] = pack_size;
    nrf_err_t ret;
    uint8_t addr_buf[3],temp;
    int i = 0;
    for (i = 0;i<3;i++,addr >>= 8){
        addr_buf[i] = (uint8_t)addr;
    }
    ret = read_reg(nRF_EN_RXADDR,&temp,1);
    if(ret){
        return -5;
    }
    temp |= 0x01 << channel;
    ret = write_reg(nRF_EN_RXADDR,&temp,1);
    if(ret){
        return -4;
    }
    if(channel < 2){
        ret = write_reg(nRF_RX_PW_P0,&pack_size,1);
        if(ret){
            return -3;
        }
        return write_reg(nRF_RX_ADDR_P0,addr_buf,3);
    }else{
        ret = write_reg(nRF_RX_PW_P0,&pack_size,1);
        if(ret){
            return -2;
        }
        return write_reg(nRF_RX_ADDR_P0,addr_buf,1);
    }
}

nrf_err_t nrf_config_rx_buffer(uint8_t channel,uint8_t * buffer_point){
    if(channel > 5){
        return NRF_ERR_arg;
    }else if(buffer_point == NULL){
        return NRF_ERR_arg;
    }
    rx_buffer_point[channel] = buffer_point;
    return NRF_OK;
}

/*
mod:only NRF_MODE_Tx & NRF_MODE_Rx can be selected;
buffer: rx data load into this position
*/
nrf_err_t nrf_set_enable(nrf_mode_e mod){
    if(mod < 0 || mod > 4){
        return NRF_ERR_arg;
    }
    uint8_t dat;
    if(nrf_change_device_mode(NRF_MODE_Power_Off)){
        return NRF_ERR_undefine;
    }
    dat=NRF_RFCHANNEL_ADDR_LENS_IN_BYTE;
    write_reg(nRF_SETUP_AW,&dat,1);// make address width = NRF_RFCHANNEL_ADDR_LENS_IN_BYTE
    dat = 0x3f;
    write_reg(nRF_EN_AA,&dat,1);// enable auto ack on every channel
    dat = 0x0e;
    write_reg(nRF_RF_SETUP,&dat,1);// set up RF
    return nrf_change_device_mode(mod);
}

nrf_mode_e nrf_get_mode(void){
    return nrf_mode;
}

/* this function need to be put in interrupt function */
/* return :
    0~5:rx recived.return channel number
    6  :tx ok
    7  :MAXRT (retry over times)
    <0 :io error accurd.the value is treat for debug
 */
int nrf_interrupt_handler(void){
    int ret = -1;
    uint8_t status;
    uint8_t chn = 255;
    if(read_reg(nRF_STATUS,&status,1)){
        return -1;
    }
    if(status & 0x40){//rx recieved
        uint8_t cmd = (uint8_t)NRF_CMD_R_RX;
        CSN(0);
        if(write(&cmd,1)){
            CSN(1);
            return -2;
        }
        chn = ( status & 0x0f ) >> 1;
        if(read(rx_buffer_point[chn],pip_width[chn])){
            CSN(1);
            return -3;
        }
        ret = chn;
        CSN(1);
    }else if(status & 0x20){//tx ok
        if(status & 0x01){
            if(nrf_clear_tx_fifo()){//tx buffer full;need to clear;
                return -4;
            };
        }else{
            nrf_mode = NRF_MODE_Sleep_2;
        }
        ret =  6;//tx ok
    }else{//max retry
        ret = 7;//max retry
    }
    //clear status bit
    status = 0x70;
    if(write_reg(nRF_STATUS,&status,1)){
        return -5;
    }
    return ret;
}