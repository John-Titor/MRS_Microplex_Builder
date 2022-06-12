/** @file
 *
 * EEPROM interface.
 *
 * @Note    EEPROM functions are NOT INTERRUPT SAFE. Do not call
 *          any of these functions from interrupt context.
 *
 * @Note    Writing to the EEPROM requires a sector erase, which
 *          takes no less than 20mS.
 */

#pragma ONCE

#include <stddef.h>
#include <stdint.h>

/* decoded CAN speed values */
#define MRS_CAN_1000KBPS    1
#define MRS_CAN_800KBPS     2
#define MRS_CAN_500KBPS     3
#define MRS_CAN_250KBPS     4
#define MRS_CAN_125KBPS     5

/**
 * MRS EEPROM contents.
 *
 * These are on page 0, which is always selected when the
 * EEPROM is not being actively read or written.
 *
 * @Note    Because the EEPROM page select may be flipped during
 *          a read or programming operation, this structure cannot
 *          be safely accessed from an interrupt handler.
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
} MRS_parameters_t;

extern const MRS_parameters_t   MRS_parameters @ 0x1402;

/** EEPROM offset for the named MRS EEPROM parameter */
#define MRS_PARAM_OFFSET(_p)    offsetof(MRS_parameters_t, _p)

/** Size of the named MRS EEPROM parameter */
#define MRS_PARAM_SIZE(_p)      sizeof(MRS_parameters. ## _p)

/** Copy named parameter to buffer - must be large enough */
#define MRS_PARAM_READ(_p, _b)              \
    HAL_eeprom_read(MRS_PARAM_OFFSET(_p),   \
                    MRS_PARAM_SIZE(_p),     \
                    (uint8_t *)_b)

extern void _HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data);
extern void _HAL_eeprom_write_str(uint16_t offset, uint8_t len, const char *data);

/** Set the SoftwareVersion parameter to the string (truncated) in _version */
#define MRS_set_software_version(_version)                      \
    _HAL_eeprom_write_str(MRS_PARAM_OFFSET(SoftwareVersion),    \
                          MRS_PARAM_SIZE(SoftwareVersion),      \
                          _version)


/** Set the ModuleName parameter to the string (truncated) in _name */
#define MRS_set_module_name(_name)                              \
    _HAL_eeprom_write_str(MRS_PARAM_OFFSET(ModuleName),         \
                          MRS_PARAM_SIZE(ModuleName),           \
                          _name)


/**
 * Write data to the EEPROM.
 *
 * Does not allow overwriting the MRS reserved area (offsets below 0x200).
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param len               The number of bytes to write.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write(uint16_t offset, uint8_t len, const uint8_t *data);

/**
 *  Write a byte to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write8(uint16_t offset, uint8_t data);

/**
 *  Write 2 bytes to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write16(uint16_t offset, uint16_t data);

/**
 *  Write 4 bytes to the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param data              The data to write.
 */
extern void HAL_eeprom_write32(uint16_t offset, uint32_t data);

/**
 * Read data from the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 * @param len               The number of bytes to read.
 * @param data              The buffer into which data should be read.
 */
extern void HAL_eeprom_read(uint16_t offset, uint8_t len, uint8_t *data);

/**
 * Read a byte from the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 */
extern uint8_t HAL_eeprom_read8(uint16_t offset);

/**
 * Read 2 bytes from the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 */
extern uint16_t HAL_eeprom_read16(uint16_t offset);

/**
 * Read 4 bytes from the EEPROM.
 *
 * @param offset            Offset from the base of the EEPROM.
 */
extern uint32_t HAL_eeprom_read32(uint16_t offset);
