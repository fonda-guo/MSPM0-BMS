#include "BQ769x2_protocol.h"

//******************************************************************************
// BQ Parameters ***************************************************************
//******************************************************************************
// Global Variables for cell voltages, temperatures, Stack voltage, PACK Pin voltage, LD Pin voltage, CC2 current
uint16_t CellVoltage[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
float Temperature[7]     = {0, 0, 0, 0, 0, 0, 0};
uint16_t Stack_Voltage   = 0x00;
uint16_t Pack_Voltage    = 0x00;
uint16_t LD_Voltage      = 0x00;
int16_t Pack_Current     = 0x00;
uint16_t AlarmBits       = 0x00;

uint8_t value_SafetyStatusA;  // Safety Status Register A
uint8_t value_SafetyStatusB;  // Safety Status Register B
uint8_t value_SafetyStatusC;  // Safety Status Register C
uint8_t value_PFStatusA;      // Permanent Fail Status Register A
uint8_t value_PFStatusB;      // Permanent Fail Status Register B
uint8_t value_PFStatusC;      // Permanent Fail Status Register C
uint8_t FET_Status;  // FET Status register contents  - Shows states of FETs
uint16_t CB_ActiveCells  = 0x00;  // Cell Balancing Active Cells

uint8_t UV_Fault             = 0;  // under-voltage fault state
uint8_t OV_Fault             = 0;  // over-voltage fault state
uint8_t SCD_Fault            = 0;  // short-circuit fault state
uint8_t OCD_Fault            = 0;  // over-current fault state
uint8_t ProtectionsTriggered = 0;  // Set to 1 if any protection triggers

uint8_t LD_ON = 0;  // Load Detect status bit
uint8_t DSG   = 0;  // discharge FET state
uint8_t CHG   = 0;  // charge FET state
uint8_t PCHG  = 0;  // pre-charge FET state
uint8_t PDSG  = 0;  // pre-discharge FET state

uint32_t AccumulatedCharge_Int;   // in AFE_READPASSQ func
uint32_t AccumulatedCharge_Frac;  // in AFE_READPASSQ func
uint32_t AccumulatedCharge_Time;  // in AFE_READPASSQ func

uint8_t RX_32Byte[32] = {0x00};
volatile uint8_t RX_data[2]    = {0x00};

//**********************************BQ Parameters *********************************

//**********************************Function prototypes **********************************
uint8_t Checksum(uint8_t *ptr, uint8_t len)
// Calculates the checksum when writing to a RAM register. The checksum is the inverse of the sum of the bytes.
{
    uint8_t i;
    uint8_t checksum = 0;

    for (i = 0; i < len; i++) checksum += ptr[i];

    checksum = 0xff & ~checksum;

    return (checksum);
}

uint8_t CRC8(uint8_t *ptr, uint8_t len)
//Calculates CRC8 for passed bytes. Used in i2c read and write functions
{
    uint8_t i;
    uint8_t crc = 0;
    while (len-- != 0) {
        for (i = 0x80; i != 0; i /= 2) {
            if ((crc & 0x80) != 0) {
                crc *= 2;
                crc ^= 0x107;
            } else
                crc *= 2;

            if ((*ptr & i) != 0) crc ^= 0x107;
        }
        ptr++;
    }
    return (crc);
}

void DirectCommands(uint8_t command, uint16_t data, uint8_t type)
// See the TRM or the BQ76952 header file for a full list of Direct Commands.
// For read type, user can read data from command address and stored in the Rx state of global variable to be read.
// For write type, user can write the data to the command address.
{  //type: R = read, W = write
    uint8_t TX_data[2] = {0x00, 0x00};

    //little endian format
    TX_data[0] = data & 0xff;
    TX_data[1] = (data >> 8) & 0xff;

    if (type == R) {                       //Read
        I2C_ReadReg(command, RX_data, 2);  //RX_data is a global variable
        delayMS(2);
        delayMS(2);  //success in 100k. iImportant to delay 4ms, or register read data may go wrong
    }
    if (type == W) {  //write
        //Control_status, alarm_status, alarm_enable all 2 bytes long
        I2C_WriteReg(command, TX_data, 2);
        delayMS(2);
        delayMS(2);
    }
}

void CommandSubcommands(uint16_t command)  //For Command only Subcommands
// See the TRM or the BQ76952 header file for a full list of Command-only subcommands
// All that this function do is formatting the transfer array then writing the array to hex 3E,
// the monitor will then operate based on the command.
{  //For DEEPSLEEP/SHUTDOWN subcommand you will need to call this function twice consecutively

    uint8_t TX_Reg[2] = {0x00, 0x00};

    //TX_Reg in little endian format
    TX_Reg[0] = command & 0xff;
    TX_Reg[1] = (command >> 8) & 0xff;

    I2C_WriteReg(0x3E, TX_Reg, 2);
    delayMS(2);
}

void Subcommands(uint16_t command, uint16_t data, uint8_t type)
// See the TRM or the BQ76952 header file for a full list of Subcommands
// The input type can either be the defined macros R for read, W for write, or W2 for write two bytes.
{
    //security keys and Manu_data writes dont work with this function (reading these commands works)
    //max readback size is 32 bytes i.e. DASTATUS, CUV/COV snapshot
    uint8_t TX_Reg[4]    = {0x00, 0x00, 0x00, 0x00};
    uint8_t TX_Buffer[2] = {0x00, 0x00};

    //TX_Reg in little endian format
    TX_Reg[0] = command & 0xff;
    TX_Reg[1] = (command >> 8) & 0xff;

    if (type == R) {  //read
        I2C_WriteReg(0x3E, TX_Reg, 2);
        delayMS(2);
        I2C_ReadReg(0x40, RX_32Byte, 32);  //RX_32Byte is a global variable
    } else if (type == W) {
        //FET_Control, REG12_Control
        TX_Reg[2] = data & 0xff;
        I2C_WriteReg(0x3E, TX_Reg, 3);
        delayMS(1);
        TX_Buffer[0] = Checksum(TX_Reg, 3);
        TX_Buffer[1] = 0x05;  //combined length of registers address and data
        I2C_WriteReg(0x60, TX_Buffer, 2);
        delayMS(1);
    } else if (type == W2) {  //write data with 2 bytes
        //CB_Active_Cells, CB_SET_LVL
        TX_Reg[2] = data & 0xff;
        TX_Reg[3] = (data >> 8) & 0xff;
        I2C_WriteReg(0x3E, TX_Reg, 4);
        delayMS(1);
        TX_Buffer[0] = Checksum(TX_Reg, 4);
        TX_Buffer[1] = 0x06;  //combined length of registers address and data
        I2C_WriteReg(0x60, TX_Buffer, 2);
        delayMS(1);
    }
}

void BQ769x2_SetRegister(uint16_t reg_addr, uint32_t reg_data, uint8_t datalen)
// This function will write hex 3E for the initial write for subcommands in direct memory
// and write to register hex 60 for the checksum to enter the data transmitted was correct.
// and there are different cases for the three varying data lengths.
{
    uint8_t TX_Buffer[2]  = {0x00, 0x00};
    uint8_t TX_RegData[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    //TX_RegData in little endian format
    TX_RegData[0] = reg_addr & 0xff;
    TX_RegData[1] = (reg_addr >> 8) & 0xff;
    TX_RegData[2] = reg_data & 0xff;  //1st byte of data

    switch (datalen) {
        case 1:  //1 byte datalength
            I2C_WriteReg(0x3E, TX_RegData, 3);
            delayMS(2);
            TX_Buffer[0] = Checksum(TX_RegData, 3);
            TX_Buffer[1] =
                0x05;  //combined length of register address and data
            I2C_WriteReg(0x60, TX_Buffer, 2);  // Write the checksum and length
            delayMS(2);
            break;
        case 2:  //2 byte datalength
            TX_RegData[3] = (reg_data >> 8) & 0xff;
            I2C_WriteReg(0x3E, TX_RegData, 4);
            delayMS(2);
            TX_Buffer[0] = Checksum(TX_RegData, 4);
            TX_Buffer[1] =
                0x06;  //combined length of register address and data
            I2C_WriteReg(0x60, TX_Buffer, 2);  // Write the checksum and length
            delayMS(2);
            break;
        case 4:  //4 byte datalength, Only used for CCGain and Capacity Gain
            TX_RegData[3] = (reg_data >> 8) & 0xff;
            TX_RegData[4] = (reg_data >> 16) & 0xff;
            TX_RegData[5] = (reg_data >> 24) & 0xff;
            I2C_WriteReg(0x3E, TX_RegData, 6);
            delayMS(2);
            TX_Buffer[0] = Checksum(TX_RegData, 6);
            TX_Buffer[1] =
                0x08;  //combined length of register address and data
            I2C_WriteReg(0x60, TX_Buffer, 2);  // Write the checksum and length
            delayMS(2);
            break;
    }
}
//************************************BQ769X2 Functions*********************************
void BQ769x2_Init(BatPackBasicInfo* packInfo)
{
    // Configures all parameters in device RAM
    // Enter CONFIGUPDATE mode (Subcommand 0x0090) - It is required to be in CONFIG_UPDATE mode to program the device RAM settings
    // See TRM for full description of CONFIG_UPDATE mode
    CommandSubcommands(BQ769x2_RESET);
    delayMS(60);
    CommandSubcommands(SET_CFGUPDATE);
    delayMS(8);

    // After entering CONFIG_UPDATE mode, RAM registers can be programmed. When programming RAM, checksum and length must also be
    // programmed for the change to take effect. All of the RAM registers are described in detail in the BQ769x2 TRM.
    // An easier way to find the descriptions is in the BQStudio Data Memory screen. When you move the mouse over the register name,
    // a full description of the register and the bits will pop up on the screen.

    // 'Power Config' - 0x9234 = 0x2D80
    // Setting the DSLP_LDO bit allows the LDOs to remain active when the device goes into Deep Sleep mode
    // Set wake speed bits to 00 for best performance
	BQ769x2_SetRegister(PowerConfig, 0x2D90, 2);

    // 'REG0 Config' - set REG0_EN bit to enable pre-regulator
    BQ769x2_SetRegister(REG0Config, 0x01, 1);

    // 'REG12 Config' - Enable REG1 with 3.3V output (0x0D for 3.3V, 0x0F for 5V)
    BQ769x2_SetRegister(REG12Config, 0x0D, 1);

    //Set 0x00 to those no used pins
    NotUsedPin_Initialize(packInfo->notUsedPinIndex, NOT_USED_PIN_NUM);
    // Set 4 Pins to measure Cell Temperature
    Temperature_Pin_Initialize(packInfo->thermistorPinIndex, THERMISTOR_NUM);		

    // 'VCell Mode' - Enable 16 cells - 0x9304 = 0x0000; Writing 0x0000 sets the default of 16 cells
		VcellMode_Initialize(packInfo->cellIndex, CELL_NUM);
		
    // Enable protections in 'Enabled Protections A' 0x9261 = 0xBC
    // Enables SCD (short-circuit), OCD1 (over-current in discharge), OCC (over-current in charge),
    // COV (over-voltage), CUV (under-voltage)
    BQ769x2_SetRegister(EnabledProtectionsA, 0xBC, 1);

    // Enable all protections in 'Enabled Protections B' 0x9262 = 0xF7
    // Enables OTF (over-temperature FET), OTINT (internal over-temperature), OTD (over-temperature in discharge),
    // OTC (over-temperature in charge), UTINT (internal under-temperature), UTD (under-temperature in discharge), UTC (under-temperature in charge)
    BQ769x2_SetRegister(EnabledProtectionsB, 0xF7, 1);

    // 'Default Alarm Mask' - 0x..82 Enables the FullScan and ADScan bits, default value = 0xF800
    BQ769x2_SetRegister(DefaultAlarmMask, 0xF882, 2);

    //Cell Balance Init
    CellBalanceInit(packInfo);

    // Set up CUV (under-voltage) Threshold - 0x9275
    BQ769x2_SetRegister(CUVThreshold, packInfo->CUVvol, 1);
    // Set up COV (over-voltage) Threshold - 0x9278
		BQ769x2_SetRegister(COVThreshold, packInfo->COVvol, 1);
		
    //Set up current unit
    //BQ769x2_SetRegister(DAConfiguration,0x06,1);
		BQ769x2_SetRegister(DAConfiguration,0x07,1);
		//Set CCGain to calibration current
    BQ769x2_SetRegister(CCGain,0x40F21AD2,4);//7.4768*1.0119
    // Set up OCC (over-current in charge) Threshold - 0x9280 = 0x05 (10 mV = 10A across 1mOhm sense resistor) Units in 2mV
    BQ769x2_SetRegister(OCCThreshold, 0x05, 1);
    // Set up OCD1 (over-current in discharge) Threshold - 0x9282 = 0x0A (20 mV = 20A across 1mOhm sense resistor) units of 2mV
    BQ769x2_SetRegister(OCD1Threshold, 0x0A, 1);

    // Set up SCD (short discharge current) Threshold - 0x9286 = 0x05 (100 mV = 100A across 1mOhm sense resistor)  0x05=100mV
    BQ769x2_SetRegister(SCDThreshold, 0x05, 1);

    // Set up SCD Delay - 0x9287 = 0x03 (30 us) Enabled with a delay of (value - 1) * 15 �s; min value of 1
    BQ769x2_SetRegister(SCDDelay, 0x03, 1);

    // Set up SCDL Latch Limit to 1 to set SCD recovery only with load removal 0x9295 = 0x01
    // If this is not set, then SCD will recover based on time (SCD Recovery Time parameter).
    BQ769x2_SetRegister(SCDLLatchLimit, 0x01, 1);

    delayMS(8);
    // Exit CONFIGUPDATE mode  - Subcommand 0x0092
    CommandSubcommands(EXIT_CFGUPDATE);
    delayMS(8);
    //Control All FETs on
    CommandSubcommands(FET_ENABLE);
    delayMS(8);
    CommandSubcommands(ALL_FETS_ON);
    delayMS(8);
    CommandSubcommands(SLEEP_DISABLE);
    delayMS(8);
}

// ********************************* BQ769x2 Status and Fault Commands   *****************************************

void BQ769x2_ReadAlarmStatus()
{
    // Read this register to find out why the ALERT pin was asserted
    DirectCommands(AlarmStatus, 0x00, R);
    AlarmBits = (uint16_t) RX_data[1] * 256 + (uint16_t) RX_data[0];
}

uint16_t BQ76952_ReadBatteryStatus()
{
	  DirectCommands(BatteryStatus, 0x00, R);
	  return (uint16_t) RX_data[1] * 256 + (uint16_t) RX_data[0];
}

void BQ769x2_ReadSafetyStatus()
{
    // Read Safety Status A/B/C and find which bits are set
    // This shows which primary protections have been triggered
    DirectCommands(SafetyStatusA, 0x00, R);
    value_SafetyStatusA = (RX_data[1] * 256 + RX_data[0]);
    //Example Fault Flags
    UV_Fault  = ((0x4 & RX_data[0]) >> 2);
    OV_Fault  = ((0x8 & RX_data[0]) >> 3);
    SCD_Fault = ((0x8 & RX_data[1]) >> 3);
    OCD_Fault = ((0x2 & RX_data[1]) >> 1);
    DirectCommands(SafetyStatusB, 0x00, R);
    value_SafetyStatusB = (RX_data[1] * 256 + RX_data[0]);
    DirectCommands(SafetyStatusC, 0x00, R);
    value_SafetyStatusC = (RX_data[1] * 256 + RX_data[0]);
    if ((value_SafetyStatusA + value_SafetyStatusB + value_SafetyStatusC) >
        1) {
        ProtectionsTriggered = 1;
    } else {
        ProtectionsTriggered = 0;
    }
}
//PF is not used for now
void BQ769x2_ReadPFStatus()
{
    // Read Permanent Fail Status A/B/C and find which bits are set
    // This shows which permanent failures have been triggered
    DirectCommands(PFStatusA, 0x00, R);
    value_PFStatusA = (RX_data[1] * 256 + RX_data[0]);
    DirectCommands(PFStatusB, 0x00, R);
    value_PFStatusB = (RX_data[1] * 256 + RX_data[0]);
    DirectCommands(PFStatusC, 0x00, R);
    value_PFStatusC = (RX_data[1] * 256 + RX_data[0]);
}

void BQ769x2_ReadFETStatus()
{
    // Read FET Status to see which FETs are enabled
    DirectCommands(FETStatus, 0x00, R);
    FET_Status = (RX_data[1] * 256 + RX_data[0]);
}
// ********************************* End of BQ769x2 Status and Fault Commands   *****************************************

// ********************************* BQ769x2 Measurement Commands   *****************************************

uint16_t BQ769x2_ReadVoltage(uint8_t command)
// This function can be used to read a specific cell voltage or stack / pack / LD voltage
{
    //RX_data is global var
    DirectCommands(command, 0x00, R);
    if (command >= Cell1Voltage &&
        command <= Cell16Voltage) {  //Cells 1 through 16 (0x14 to 0x32)
        return (RX_data[1] * 256 + RX_data[0]);  //voltage is reported in mV
    } else {                                     //stack, Pack, LD
        return 10 * (RX_data[1] * 256 +
                        RX_data[0]);  //voltage is reported in 0.01V units
    }
}

void BQ769x2_ReadAllVoltages()
// Reads all cell voltages, Stack voltage, PACK pin voltage, and LD pin voltage
{
    uint8_t x;
    int cellvoltageholder = Cell1Voltage;  //Cell1Voltage is 0x14
    for (x = 0; x < 16; x++) {             //Reads all cell voltages
        CellVoltage[x]    = BQ769x2_ReadVoltage(cellvoltageholder);
        cellvoltageholder = cellvoltageholder + 2;
    }
    Stack_Voltage = BQ769x2_ReadVoltage(StackVoltage);
    Pack_Voltage  = BQ769x2_ReadVoltage(PACKPinVoltage);
    LD_Voltage    = BQ769x2_ReadVoltage(LDPinVoltage);
}

void BQ769x2_ReadCurrent()
// Reads PACK current
{
    DirectCommands(CC2Current, 0x00, R);
    Pack_Current =
        (int16_t)((uint16_t) RX_data[1] * 256 +
                  (uint16_t) RX_data[0]);  // current is reported in mA
}

float BQ769x2_ReadTemperature(uint8_t command)
{
    DirectCommands(command, 0x00, R);
    //RX_data is a global var
    return (0.1 * (float) (RX_data[1] * 256 + RX_data[0])) -
           273.15;  // converts from 0.1K to Celcius
}

void BQ769x2_ReadAllTemperatures()
{
    Temperature[0] = BQ769x2_ReadTemperature(DDSGTemperature);
    Temperature[1] = BQ769x2_ReadTemperature(DCHGTemperature);
    Temperature[2] = BQ769x2_ReadTemperature(DFETOFFTemperature);
	  Temperature[3] = BQ769x2_ReadTemperature(ALERTTemperature);
	  Temperature[4] = BQ769x2_ReadTemperature(TS3Temperature);
	  Temperature[5] = BQ769x2_ReadTemperature(TS1Temperature);
    Temperature[6] = BQ769x2_ReadTemperature(CFETOFFTemperature);	
}

void BQ769x2_ReadPassQ()
{  // Read Accumulated Charge and Time from DASTATUS6
    Subcommands(DASTATUS6, 0x00, R);
    AccumulatedCharge_Int  = ((RX_32Byte[3] << 24) + (RX_32Byte[2] << 16) +
                             (RX_32Byte[1] << 8) + RX_32Byte[0]);  //Bytes 0-3
    AccumulatedCharge_Frac = ((RX_32Byte[7] << 24) + (RX_32Byte[6] << 16) +
                              (RX_32Byte[5] << 8) + RX_32Byte[4]);  //Bytes 4-7
    AccumulatedCharge_Time =
        ((RX_32Byte[11] << 24) + (RX_32Byte[10] << 16) + (RX_32Byte[9] << 8) +
            RX_32Byte[8]);  //Bytes 8-11
}
//************************************End of BQ769x2 Measurement Commands******************************************
//************************************Customer function************************************************************
void Temperature_Pin_Initialize(const uint16_t *temperaturePinIndex, uint8_t temperatureNum){
    
	
	  for(int i = 0; i < temperatureNum; i++){
		    BQ769x2_SetRegister(temperaturePinIndex[i], 0x07, 1);
		}
}

void VcellMode_Initialize(const uint8_t* cellIndex, uint8_t cellNum){
    uint16_t volmode = 0;
	  for(int i = 0; i < cellNum; i++){
		    volmode += (0x1 << ((cellIndex[i] - Cell1Voltage) >> 1));
		}
		BQ769x2_SetRegister(VCellMode, volmode, 2);
}

void NotUsedPin_Initialize(const uint16_t* notUsedPinIndex, uint8_t pinNum){
	  for(int i = 0; i < pinNum; i++){
		    BQ769x2_SetRegister(notUsedPinIndex[i], 0x00, 1);
		}
}

//************************************Cell Balance************************************************************
void CellBalanceInit(BatPackBasicInfo* packInfo){
    //Automated balancing while in Relax or Charge modes, but no host-controlled mode
    BQ769x2_SetRegister(BalancingConfiguration, packInfo->CB_config, 1);
	  
	  BQ769x2_SetRegister(MinCellTemp, packInfo->CB_MinTemp, 1);
	  BQ769x2_SetRegister(MaxCellTemp, packInfo->CB_MaxTemp, 1);
	  BQ769x2_SetRegister(MaxInternalTemp, packInfo->CB_MaxIntTemp, 1);
	  BQ769x2_SetRegister(CellBalanceInterval, packInfo->CB_Interval, 1);
	  BQ769x2_SetRegister(CellBalanceMaxCells, packInfo->CB_MaxCells, 1);
	  BQ769x2_SetRegister(CellBalanceMinCellVCharge, packInfo->CB_MinCellVol_Chg, 2);
	  BQ769x2_SetRegister(CellBalanceMinDeltaCharge, packInfo->CB_MinDelta_Chg, 1);
	  BQ769x2_SetRegister(CellBalanceStopDeltaCharge, packInfo->CB_StopDelta_Chg, 1);
	  BQ769x2_SetRegister(CellBalanceMinCellVRelax, packInfo->CB_MinCellVol_Rlx, 2);
	  BQ769x2_SetRegister(CellBalanceMinDeltaRelax, packInfo->CB_MinDelta_Rlx, 1);
	  BQ769x2_SetRegister(CellBalanceStopDeltaRelax, packInfo->CB_StopDelta_Rlx, 1);
}
//Read from CB_ACTIVE_CELLS
uint16_t CellBalanceStatusGet(){
    Subcommands(CB_ACTIVE_CELLS, 0x00, R);
	  CB_ActiveCells = ((RX_32Byte[1] << 8) + RX_32Byte[0]);  //Bytes 0-1;
	  return CB_ActiveCells;
}

//Close cell balance
void CloseCellBalance(){
   BQ769x2_SetRegister(BalancingConfiguration, 0x00, 1);
}
//Open cell balance
void OpenCellBalance(BatPackBasicInfo* packInfo){
   BQ769x2_SetRegister(BalancingConfiguration, packInfo->CB_config, 1);
}