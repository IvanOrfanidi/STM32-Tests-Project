/**
 ******************************************************************************
 * @file    nrf24l01.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    04/01/2018
 * @brief
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2018 </center></h2>
 ******************************************************************************
 */


/* Includes ------------------------------------------------------------------*/
#include "nrf24l01.hpp"
#include "stm32f10x_rcc.h"
#include "board.hpp"

     
/// Addresses of the RX_PW_P# registers
const uint8_t RX_PW_PIPE[] = 
{
    Nrf::REG_RX_PW_P0,
    Nrf::REG_RX_PW_P1,
    Nrf::REG_RX_PW_P2,
    Nrf::REG_RX_PW_P3,
    Nrf::REG_RX_PW_P4,
    Nrf::REG_RX_PW_P5
};


/**
 * @brief Static instances of a class
 */
Nrf* Nrf::Nrf24l01[NRF24_MAX_COUNT];


/**
 * @brief Сonstructor
 */
Nrf::Nrf(VirtualPort* port, GPIO_TypeDef* ce_port, uint16_t ce_pin, GPIO_TypeDef* cns_port, uint16_t cns_pin)
{
    InterfaceSettings_t newSettings;
    newSettings.CE_Port = ce_port;
    newSettings.CNS_Port = cns_port;
    newSettings.CE_Pin.GPIO_Pin = ce_pin;
    newSettings.CNS_Pin.GPIO_Pin = cns_pin;
    VPort = port;
    
    // Def settings Pin
    newSettings.CE_Pin.GPIO_Mode = GPIO_Mode_Out_PP;
    newSettings.CE_Pin.GPIO_Speed = GPIO_Speed_10MHz;
    newSettings.CNS_Pin.GPIO_Mode = GPIO_Mode_Out_PP;
    newSettings.CNS_Pin.GPIO_Speed = GPIO_Speed_10MHz;
    
    InterfaceSettings = nullptr;
    size_t freeClass = NRF24_MAX_COUNT;
    for(size_t i = 0; i < NRF24_MAX_COUNT; i++) {
        if(Nrf24l01[i]) {
            InterfaceSettings_t nrfSettings;
            Nrf24l01[i]->GetInterfaceSettings(&nrfSettings);
            if(((memcmp(&nrfSettings.CE_Pin, &newSettings.CE_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
               (nrfSettings.CE_Port == newSettings.CE_Port)) ||
               ((memcmp(&nrfSettings.CNS_Pin, &newSettings.CNS_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
               (nrfSettings.CNS_Port == newSettings.CNS_Port))) {
                    /// Error, class was create
                    return;
            }
        }
        else {
            freeClass = i;
        }
    }
    
    if(NRF24_MAX_COUNT == freeClass) {
        return;
    }
    
    SetInterfaceSettings(newSettings);
    
    // Config GPIO(CE, CNS)
    InitGpio(newSettings);
        
    Nrf24l01[freeClass] = this;
}


/**
 * @brief Destructor
 */
Nrf::~Nrf()
{
    if(InterfaceSettings) {
        DeInitGpio(InterfaceSettings);
        DeInitSpi(InterfaceSettings);
        for(size_t i = 0; i < NRF24_MAX_COUNT; i++) {
            if(Nrf24l01[i]) {
                InterfaceSettings_t nrfSettings;
                Nrf24l01[i]->GetInterfaceSettings(&nrfSettings);
                if(((memcmp(&nrfSettings.CE_Pin, &InterfaceSettings->CE_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
                   (nrfSettings.CE_Port == InterfaceSettings->CE_Port)) ||
                   ((memcmp(&nrfSettings.CNS_Pin, &InterfaceSettings->CNS_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
                   (nrfSettings.CNS_Port == InterfaceSettings->CNS_Port))) {
                        delete [] InterfaceSettings;
                        Nrf24l01[i]  = nullptr;
                        break;
                }
            }
        }
    }
}


void Nrf::Init(Settings_t& settings)
{
    //  Set transceiver to it's initial state
    // Регистр устанавливает динамическую длину полезной нагрузки
    WriteReg(REG_DYNPD, 0);
    // Регистр устанавливает параметры для полезной нагрузки
    WriteReg(REG_FEATURE, 0);
    
    // Set RF channel, freq + 2400MHz
    SetRFChannel(settings.Channel);
    
    // Set data rate
    SetDataRate(settings.DataRate);
    
    // Set CRC scheme
    SetCrcScheme(settings.CrcScheme);
    
    // Set address width, its common for all pipes (RX and TX)
    SetAddrWidth(settings.AddrWidth);
    
    // Configure TX/RX PIPE
    if(settings.OperationalMode == MODE_TX) {
        // Set program TX address
        SetAddr(Nrf::PIPETX, settings.Addr);
        
        // Set program address for pipe, must be same as TX (for Auto-ACK)
        SetAddr(settings.Pipe, settings.Addr);

        // Configure Auto-ACK for pipe (for ACK packets)
        if(settings.StateAutoAck == AA_ON) {
            // Set configure auto retransmit
            SetAutoRetr(settings.AutoRetransmitDelay, settings.CountAutoRetransmits);
            EnableAA(settings.Pipe);
        }
        else {
            DisableAA(settings.Pipe);
        }
    }
    else {
        // Set program RX address
        SetAddr(settings.Pipe, settings.Addr);
        
        // Configure Auto-ACK: enabled and payload length
        if(settings.StateAutoAck == AA_ON) {
            SetRxPipe(settings.Pipe, settings.StateAutoAck, settings.PayloadLength);
        }
    }
    
    // Set TX power
    SetTxPower(settings.RfPower);
    
    // Set operational mode (PTX == transmitter)
    SetOperationalMode(settings.OperationalMode);
    
    // Clear the FIFO's
    FlushRX();
    FlushTX();
    
    // Clear any pending interrupt flags
    ClearIRQFlags();
    
    // Deassert CSN pin (chip release)
    CsnHigh();
}


/**
 * @brief Clear any pending IRQ flags
 */
void Nrf::ClearIRQFlags() const
{
    // Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
    uint8_t state = ReadReg(REG_STATUS);
    state |= MASK_STATUS_IRQ;
    WriteReg(REG_STATUS, state);
}


/**
 * @brief Get alls settings
 * @param [out] settings - alls interface settings, GPIO
 */
void Nrf::GetInterfaceSettings(InterfaceSettings_t* const settings) const
{
    memcpy(settings, InterfaceSettings, sizeof(InterfaceSettings_t));
}


/**
 * @brief Set alls settings
 * @param [in] settings - alls interface settings, GPIO
 */
void Nrf::SetInterfaceSettings(InterfaceSettings_t& settings)
{
    /* Copy Settings */
    InterfaceSettings = new InterfaceSettings_t;
    memcpy(&InterfaceSettings->CE_Pin, &settings.CE_Pin, sizeof(GPIO_InitTypeDef));
    memcpy(&InterfaceSettings->CNS_Pin, &settings.CNS_Pin, sizeof(GPIO_InitTypeDef));
    InterfaceSettings->CE_Port = settings.CE_Port;
    InterfaceSettings->CNS_Port = settings.CNS_Port;
}


/**
 * @brief Deinitialisation GPIO
 * @param [in] settings - settings GPIO
 */
void Nrf::DeInitGpio(InterfaceSettings_t* settings) const
{
    GPIO_InitTypeDef pin;
    pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    
    pin.GPIO_Pin = settings->CE_Pin.GPIO_Pin;
    GPIO_Init(InterfaceSettings->CE_Port, &pin);
    
    pin.GPIO_Pin = settings->CNS_Pin.GPIO_Pin;
    GPIO_Init(InterfaceSettings->CE_Port, &pin);
}


/**
 * @brief Initialisation GPIO
 * @param [in] settings - settings GPIO
 */
void Nrf::InitGpio(InterfaceSettings_t& settings) const
{
    // Enable Clock Port Output
    Board::GpioClock(InterfaceSettings->CE_Port, ENABLE);
    Board::GpioClock(InterfaceSettings->CNS_Port, ENABLE);
    
    /* Config GPIO CE & CNS for nRF24 */
    GPIO_Init(InterfaceSettings->CE_Port, &InterfaceSettings->CE_Pin);
    GPIO_Init(InterfaceSettings->CNS_Port, &InterfaceSettings->CNS_Pin);
    
    RxOff();
}


/**
 * @brief Returns the status of class creation.
 * @retval true -  Success;
 *         false - Fail.
 */
bool Nrf::CreateClass() const
{
    return (InterfaceSettings != nullptr);
}


/**
 * @brief Put the transceiver to the RX mode
 */
void Nrf::RxOn() const
{
    GPIO_SetBits(InterfaceSettings->CE_Port, InterfaceSettings->CE_Pin.GPIO_Pin);
}


/**
 * @brief Put the transceiver to the TX mode
 */
void Nrf::RxOff() const
{
    GPIO_ResetBits(InterfaceSettings->CE_Port, InterfaceSettings->CE_Pin.GPIO_Pin);
}


/**
 * @brief Check if the nRF24L01 present
 * @retval true -  Success;
 *         false - Fail.
 */
bool Nrf::Check() const
{
    /// Fake address to test transceiver presence (5 bytes long)
    enum { SIZE_TEST_BUF = 5 };
    const uint8_t request[SIZE_TEST_BUF] = { 'n', 'R', 'F', '2', '4'};
    WriteBuffer(CMD_W_REGISTER | REG_TX_ADDR, request, SIZE_TEST_BUF);
    
    uint8_t answer[SIZE_TEST_BUF];
    ReadBuffer(CMD_R_REGISTER | REG_TX_ADDR, answer, SIZE_TEST_BUF);
    
    if(memcmp(request, answer, SIZE_TEST_BUF)) {
        return false;
    }
    
    return true;
}


/**
 * @brief Function to transmit data packet
 * @param [in] buf - pointer to the buffer with data to transmit
 * @param [in] length - length of the data buffer in bytes
 * @retval: one of TXResult_t values:
 *              TX_SUCCESS
 *              TX_ERROR
 *              TX_TIMEOUT
 *              TX_MAXRT
*/
Nrf::TXResult_t Nrf::TransmitPacket(const uint8_t* buf, uint8_t length) const
{
    uint32_t wait = WAIT_TIMEOUT;

    // Deassert the CE pin (in case if it still high)
    RxOff();

    // Transfer a data from the specified buffer to the TX FIFO
    WritePayload(buf, length);

    // Start a transmission by asserting CE pin (must be held at least 10us)
    RxOn();

    // Poll the transceiver status register until one of the following flags will be set:
    //   TX_DS  - means the packet has been transmitted
    //   MAX_RT - means the maximum number of TX retransmits happened
    // note: this solution is far from perfect, better to use IRQ instead of polling the status
    uint8_t status;
    do {
        status = GetStatus();
        if(status & (FLAG_TX_DS | FLAG_MAX_RT)) {
            break;
        }
    } while (wait--);
    
    // Deassert the CE pin (Standby-II --> Standby-I)
    RxOff();

    if(!wait) {
        // Timeout
        return TX_TIMEOUT;
    }
    
    // Clear pending IRQ flags
    ClearIRQFlags();
    
    if(status & FLAG_MAX_RT) {
        // Auto retransmit counter exceeds the programmed maximum limit (FIFO is not removed)
        return TX_MAXRT;
    }

    if(status & FLAG_TX_DS) {
        // Successful transmission
        return TX_SUCCESS;
    }

    // Some banana happens, a payload remains in the TX FIFO, flush it
    FlushTX();

    return TX_ERROR;
}


void Nrf::CsnLow() const
{
    GPIO_ResetBits(InterfaceSettings->CNS_Port, InterfaceSettings->CNS_Pin.GPIO_Pin);
}


void Nrf::CsnHigh() const
{
    GPIO_SetBits(InterfaceSettings->CNS_Port, InterfaceSettings->CNS_Pin.GPIO_Pin);
}

 
/**
 * @brief Write a multi-byte register
 * @param [in] reg - number of register to write
 * @param [in] buf - pointer to the buffer with data to write
 * @param [in] count - number of bytes to write
 */
void Nrf::WriteBuffer(uint8_t reg, const uint8_t* buf, uint8_t count) const
{
    CsnLow();
    
    SpiSendReceiveData(reg);
    
    while(count--) {
        SpiSendReceiveData(*buf++);
    }
    
    CsnHigh();
}


/**
 * @brief Read a multi-byte register
 * @param [in] reg - number of register to read
 * @param [out] buf - pointer to the buffer with data to read
 * @param [in] count - number of bytes to read
 */
void Nrf::ReadBuffer(uint8_t reg, uint8_t* buf, uint8_t count) const
{
    CsnLow();
    
    SpiSendReceiveData(reg);
    
    while(count--) {
        *buf++ = SpiSendReceiveData(Nrf24Registers_t::CMD_NOP);
    }
    
    CsnHigh();
}


/**
 * @brief Read a byte register
 * @param [in] address - address of register to read
 * @retval data byte
 */
uint8_t Nrf::ReadReg(uint8_t address) const
{
    CsnLow();

    uint8_t byte = address & MASK_REG_MAP;
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    
    byte = CMD_NOP;
    while(VPort->GetLen() == 0);
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
    
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    
    while(VPort->GetLen() == 0);
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    CsnHigh();
    
    return byte;
}


/**
 * @brief Write a byte register
 * @param [in] address - address of register to write
 * @param [in] data - data byte
 */
void Nrf::WriteReg(uint8_t address, uint8_t data) const
{
    CsnLow();
    
    if(address < CMD_W_REGISTER) {
        // This is a register access
        SpiSendReceiveData(CMD_W_REGISTER | (address & MASK_REG_MAP));
    
        SpiSendReceiveData(data);
    }
    else {
        // This is a single byte command or future command/register
        SpiSendReceiveData(address);
        
        if ((address != CMD_FLUSH_TX) && (address != CMD_FLUSH_RX) && 
            (address != CMD_REUSE_TX_PL) && (address != CMD_NOP)) {
                SpiSendReceiveData(data);
            }
    }

    CsnHigh();
}


/**
 * @brief SPI transmit/receive function (hardware depended)
 * @param [in]  data - value to transmit via SPI
 * @retval value received from SPI
 */
uint8_t Nrf::SpiSendReceiveData(uint8_t byte) const
{
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    
    byte = CMD_NOP;
    while(VPort->GetLen() == 0);
    
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
    return byte;
}


/**
 * @brief Control transceiver power mode
 * @param [in] mode - new state of power mode, one of PWR_xx values:
 *              PWR_UP    - Power up
                PWR_DOWN  - Power down
 */
void Nrf::SetPowerMode(PowerControl_t mode) const
{
    uint8_t state = ReadReg(REG_CONFIG);
    if(mode == PWR_UP) {
        // Set the PWR_UP bit of CONFIG register to wake the transceiver
        // It goes into Stanby-I mode with consumption about 26uA
        state |= CONFIG_PWR_UP;
    }
    else {
        // Clear the PWR_UP bit of CONFIG register to put the transceiver
        // into power down mode with consumption about 900nA
        state &= ~CONFIG_PWR_UP;
    }
    WriteReg(REG_CONFIG, state);
}


/**
 * @brief Enable transceiver power
 */
void Nrf::Enable() const
{
    // Clear any pending IRQ flags
    ClearIRQFlags();
    SetPowerMode(Nrf::PWR_UP);
}


/**
 * @brief Disable transceiver power
 */
void Nrf::Disable() const
{
    SetPowerMode(PWR_DOWN);
}


/**
 * @brief Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
 */
void Nrf::ResetPLOS() const
{
    // The PLOS counter is reset after write to RF_CH register
    const uint8_t value = ReadReg(REG_RF_CH);
    WriteReg(REG_RF_CH, value);
}


/**
 * @brief Set frequency channel
 * @param [in] channel - radio frequency channel, value from 0 to 127
 * @note: frequency will be (2400 + channel)MHz
 * @note: PLOS_CNT[7:4] bits of the OBSERVER_TX register will be reset
 */
void Nrf::SetRFChannel(uint8_t channel) const
{
    WriteReg(REG_RF_CH, channel);
}


/**
 * @brief Flush the TX FIFO
 */
void Nrf::FlushTX() const
{
    WriteReg(CMD_FLUSH_TX, CMD_NOP);
}


/**
 * @brief Flush the RX FIFO
 */
void Nrf::FlushRX() const
{
    WriteReg(CMD_FLUSH_RX, CMD_NOP);
}


Nrf::RXResult_t Nrf::ReadPayload(uint8_t* buf, uint8_t* length)
{    
    // Extract a payload pipe number from the STATUS register
    const uint8_t pipe = ((ReadReg(REG_STATUS) & MASK_RX_P_NO) >> 1);
    
    // RX FIFO empty?
    if(pipe < RX_PIPE_ALL) {
        // Get payload length
        *length = ReadReg(RX_PW_PIPE[pipe]);

        // Read a payload from the RX FIFO
        if(*length) {
            ReadBuffer(CMD_R_RX_PAYLOAD, buf, *length);
        }

        return (RXResult_t)pipe;
    }

    // The RX FIFO is empty
    *length = 0;

    return RX_EMPTY;
}
   
/**
 * @brief Write TX payload
 * @param [in] buf - pointer to the buffer with payload data
 * @param [in] length - payload length in bytes
 */
void Nrf::WritePayload(const uint8_t* buf, uint8_t length) const
{
    WriteBuffer(CMD_W_TX_PAYLOAD, buf, length);
}

   
/**
 * @brief Set transceiver operational mode
 * @param [in] mode - operational mode, one of MODE_xx values
 */
void Nrf::SetOperationalMode(TransceiverMode_t mode) const
{
    // Configure PRIM_RX bit of the CONFIG register
    uint8_t value  = ReadReg(REG_CONFIG);
    value &= ~CONFIG_PRIM_RX;
    value |= ((uint8_t)mode & CONFIG_PRIM_RX);
    WriteReg(REG_CONFIG, value);
}


/**
 * @brief Configure transceiver CRC scheme
 * @param [in] scheme - CRC scheme, one of CRC_xx values
 * @note: transceiver will forcibly turn on the CRC in case if auto acknowledgment
 *       enabled for at least one RX pipe
 */
void Nrf::SetCrcScheme(CrcEncodingScheme_t scheme) const
{
    // Configure EN_CRC[3] and CRCO[2] bits of the CONFIG register
    uint8_t value  = ReadReg(REG_CONFIG);
    value &= ~MASK_CRC;
    value |= ((uint8_t)scheme & MASK_CRC);
    WriteReg(REG_CONFIG, value);
}


/**
 * @brief Set automatic retransmission parameters
 * @param [in] ard - auto retransmit delay, one of ARD_xx values
 * @param [in] arc - count of auto retransmits, value form 0 to 15
 * @note: zero arc value means that the automatic retransmission disabled
*/
void Nrf::SetAutoRetr(RetransmitDelay_t ard, uint8_t arc) const
{
    // Set auto retransmit settings (SETUP_RETR register)
    const uint8_t value = ((ard << 4) | (arc & MASK_RETR_ARC));
    WriteReg(REG_SETUP_RETR, value);
}


/**
 * @brief Set of address widths
 * @param [in] addr_width - RX/TX address field width, value from 3 to 5
 * @note: this setting is common for all pipes
*/
void Nrf::SetAddrWidth(uint8_t addr_width) const
{
    WriteReg(REG_SETUP_AW, addr_width - 2);
}


/**
 * @brief Set static RX address for a specified pipe
 * @param [in] pipe - pipe to configure address, one of PIPEx values
 * @param [in] addr - pointer to the buffer with address
 * @note: pipe can be a number from 0 to 5 (RX pipes) and 6 (TX pipe)
 * @note: buffer length must be equal to current address width of transceiver
 * @note: for pipes[2..5] only first byte of address will be written because
 *       other bytes of address equals to pipe1
 * @note: for pipes[2..5] only first byte of address will be written because
 *       pipes 1-5 share the four most significant address bytes
*/
void Nrf::SetAddr(uint8_t pipe, const uint8_t* addr) const
{
    /// Addresses of the address registers
    const uint8_t ADDR_REGS[] = 
    {
        Nrf::REG_RX_ADDR_P0,
        Nrf::REG_RX_ADDR_P1,
        Nrf::REG_RX_ADDR_P2,
        Nrf::REG_RX_ADDR_P3,
        Nrf::REG_RX_ADDR_P4,
        Nrf::REG_RX_ADDR_P5,
        Nrf::REG_TX_ADDR
    };

    // RX_ADDR_Px register
    uint8_t addr_width;
    switch(pipe)
    {
        case PIPETX:
        case PIPE0:
        case PIPE1:
            // Get address width
            addr_width = ReadReg(REG_SETUP_AW) + 1;
            // Write address in reverse order (LSByte first)
            addr += addr_width;
            CsnLow();
            
            SpiSendReceiveData(CMD_W_REGISTER | ADDR_REGS[pipe]);
            do {
                SpiSendReceiveData(*addr--);
            } 
            while(addr_width--);
            
            CsnHigh();
            break;
        case PIPE2:
        case PIPE3:
        case PIPE4:
        case PIPE5:
            // Write address LSBbyte (only first byte from the addr buffer)
            WriteReg(ADDR_REGS[pipe], *addr);
            break;
        default:
            // Incorrect pipe number -> do nothing
            break;
    }
}


/**
 * @brief Configure RF output power in TX mode
 * @param [in] tx_pwr - RF output power, one of TXPWR_xx values
*/
void Nrf::SetTxPower(RfOutputPower_t tx_pwr) const
{
    // Configure RF_PWR[2:1] bits of the RF_SETUP register
    uint8_t value  = ReadReg(REG_RF_SETUP);
    value &= ~MASK_RF_PWR;
    value |= tx_pwr;
    WriteReg(REG_RF_SETUP, value);
}


/**
 * @brief Configure transceiver data rate
 * @param [in] data_rate - data rate, one of DR_xx values
*/
void Nrf::SetDataRate(DataRate_t data_rate) const
{
    // Configure RF_DR_LOW[5] and RF_DR_HIGH[3] bits of the RF_SETUP register
    uint8_t value  = ReadReg(REG_RF_SETUP);
    value &= ~MASK_DATARATE;
    value |= data_rate;
    WriteReg(REG_RF_SETUP, value);
}


/**
 * @brief Configure a specified RX pipe
 * @param [in] pipe - number of the RX pipe, value from 0 to 5
 * @param [in] aa_state - state of auto acknowledgment, one of AA_xx values
 * @param [in] payload_len - payload length in bytes
*/
void Nrf::SetRxPipe(EnumerationRxPipe_t pipe, StateAutoAcknowledgment_t aa_state, uint8_t payload_len) const
{
    // Enable the specified pipe (EN_RXADDR register)
    uint8_t value = ((ReadReg(REG_EN_RXADDR) | (1 << pipe)) & MASK_EN_RX);
    WriteReg(REG_EN_RXADDR, value);

    // Set RX payload length (RX_PW_Px register)
    WriteReg(RX_PW_PIPE[pipe], payload_len & MASK_RX_PW);

    // Set auto acknowledgment for a specified pipe (EN_AA register)
    value = ReadReg(REG_EN_AA);
    if (aa_state == AA_ON) {
        value |=  (1 << pipe);
    }
    else {
        value &= ~(1 << pipe);
    }
    WriteReg(REG_EN_AA, value);
}


/**
 * @brief Disable specified RX pipe
 * @param [in] pipe - number of RX pipe, value from 0 to 5
*/
void Nrf::ClosePipe(uint8_t pipe) const
{
    uint8_t value  = ReadReg(REG_EN_RXADDR);
    value &= ~(1 << pipe);
    value &= MASK_EN_RX;
    WriteReg(REG_EN_RXADDR, value);
}

/**
 * @brief Enable the auto retransmit (a.k.a. enhanced ShockBurst) for the specified RX pipe
 * @param [in] pipe - number of the RX pipe, value from 0 to 5
*/
void Nrf::EnableAA(EnumerationRxPipe_t pipe) const
{
    // Set bit in EN_AA register
    uint8_t value  = ReadReg(REG_EN_AA);
    value |= (1 << (uint8_t)pipe);
    WriteReg(REG_EN_AA, value);
}


/**
 * @brief Disable the auto retransmit (a.k.a. enhanced ShockBurst) for one or all RX pipes
 * @param [in] pipe - number of the RX pipe, value from 0 to 5, 
 *             any other value will disable AA for all RX pipes
*/
void Nrf::DisableAA(EnumerationRxPipe_t pipe) const
{
    if (pipe > 5) {
        // Disable Auto-ACK for ALL pipes
        WriteReg(REG_EN_AA, 0x00);
    }
    else {
        // Clear bit in the EN_AA register
        uint8_t value  = ReadReg(REG_EN_AA);
        value &= ~(1 << (uint8_t)pipe);
        WriteReg(REG_EN_AA, value);
    }
}


/**
 * @brief Get value of the STATUS register
 * @retval: value of STATUS register
*/
uint8_t Nrf::GetStatus() const
{
    return ReadReg(REG_STATUS);
}


/**
 * @brief Get pending IRQ flags
 * @retval: current status of RX_DR, TX_DS and MAX_RT bits of the STATUS register
*/
uint8_t Nrf::GetIRQFlags() const
{
    return (ReadReg(REG_STATUS) & MASK_STATUS_IRQ);
}


/**
 * @brief Get status of the RX FIFO
 * @retval: one of the STATUS_RXFIFO_xx values
*/
uint8_t Nrf::GetStatus_RXFIFO() const
{
    return (ReadReg(REG_FIFO_STATUS) & MASK_RXFIFO);
}


/**
 * @brief Get status of the TX FIFO
 * @retval: one of the STATUS_TXFIFO_xx values
 * @note: the TX_REUSE bit ignored
*/
uint8_t Nrf::GetStatus_TXFIFO() const
{
    return ((ReadReg(REG_FIFO_STATUS) & MASK_TXFIFO) >> 4);
}


/**
 * @brief Get pipe number for the payload available for reading from RX FIFO
 * @retval: pipe number or 0x07 if the RX FIFO is empty
*/
uint8_t Nrf::GetRXSource() const
{
    return ((ReadReg(REG_STATUS) & MASK_RX_P_NO) >> 1);
}


/**
 * @brief Get auto retransmit statistic
 * @retval: value of OBSERVE_TX register which contains two counters encoded in nibbles:
 *  high - lost packets count (max value 15, can be reseted by write to RF_CH register)
 *  low  - retransmitted packets count (max value 15, reseted when new transmission starts)
*/
uint8_t Nrf::GetRetransmitCounters() const
{
    return (ReadReg(REG_OBSERVE_TX));
}
