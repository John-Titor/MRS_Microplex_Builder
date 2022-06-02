/** @file
 *
 * Interface to the MRS boot ROM and flash protocol.
 */

#pragma ONCE

#include <stdbool.h>
#include <stdint.h>

#include <HAL/_can.h>

#define MRS_CAN_1000KBPS            1
#define MRS_CAN_800KBPS             2
#define MRS_CAN_500KBPS             3
#define MRS_CAN_250KBPS             4
#define MRS_CAN_125KBPS             5

/**
 * Get the CAN bitrate from EEPROM.
 */
extern uint8_t          mrs_can_bitrate(void);

/**
 * MRS CAN flash protocol handler.
 */
extern bool             mrs_bootrom_rx(HAL_CAN_message_t *msg);

/**
 * MRS EEPROM contents.
 */
typedef struct {
    uint16_t    ParameterMagic;
    uint32_t    SerialNumber;
    char        PartNumber[12];
    char        DrawingNumber[12];
    char        Name[20];
    char        OrderNumber[8];
    char        TestDate[8];
    uint16_t    HardwareVersion;
    uint8_t     ResetCounter;
    uint16_t    LibraryVersion;
    uint8_t     ResetReasonLVD;
    uint8_t     ResetReasonLOC;
    uint8_t     ResetReasonILAD;
    uint8_t     ResetReasonILOP;
    uint8_t     ResetReasonCOP;
    uint8_t     MCUType;
    uint8_t     HardwareCANActive;
    uint8_t     _reserved1[3];
    uint16_t    BootloaderVersion;
    uint16_t    ProgramState;
    uint16_t    Portbyte1;
    uint16_t    Portbyte2;
    uint16_t    BaudrateBootloader1;
    uint16_t    BaudrateBootloader2;
    uint8_t     BootloaderIDExt1;
    uint32_t    BootloaderID1;
    uint8_t     BootloaderIDCRC1;
    uint8_t     BootloaderIDExt2;
    uint32_t    BootloaderID2;
    uint8_t     BootloaderIDCRC2;
    char        SoftwareVersion[20];
    char        ModuleName[30];
    uint8_t     BootloaderCANBus;
    uint16_t    COPWatchdogTimeout;
    uint8_t     _reserved2[7];
} mrs_parameters_t;

extern const mrs_parameters_t   mrs_parameters;
