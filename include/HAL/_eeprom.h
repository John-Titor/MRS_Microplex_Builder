/** @file
 *
 * EEPROM interface.
 *
 * @note    EEPROM functions are NOT INTERRUPT SAFE. Do not call
 *          any of these functions from interrupt context.
 *
 * @note    Writing to the EEPROM requires a sector erase, which
 *          takes no less than 20mS.
 */

#pragma ONCE

#include <stddef.h>
#include <stdint.h>

/** CAN speeds as encoded in the EEPROM */
enum {
    MRS_CAN_1000KBPS    = 1,    /**< 1Mbps */
    MRS_CAN_800KBPS     = 2,    /**< 800kbps */
    MRS_CAN_500KBPS     = 3,    /**< 500kbps */
    MRS_CAN_250KBPS     = 4,    /**< 250kbps */
    MRS_CAN_125KBPS     = 5,    /**< 125kbps */
};

/**
 * MRS EEPROM contents.
 *
 * These are on page 0, which is always selected when the
 * EEPROM is not being actively read or written.
 *
 * @note    Because the EEPROM page select may be flipped during
 *          a read or programming operation, this structure cannot
 *          be safely accessed from an interrupt handler.
 */
typedef struct {
    uint16_t    _ParameterMagic;
    uint32_t    SerialNumber;           /**< serial number, ID is low 16b */
    char        PartNumber[12];         /**< MRS part number */
    char        DrawingNumber[12];      /**< unknown */
    char        Name[20];               /**< MRS product name */
    char        OrderNumber[8];         /**< unknown */
    char        TestDate[8];            /**< module manufacturing test date */
    uint16_t    HardwareVersion;        /**< unknown */
    uint8_t     ResetCounter;           /**< unknown */
    uint16_t    LibraryVersion;         /**< unknown */
    uint8_t     ResetReasonLVD;         /**< LVD reset counter */
    uint8_t     ResetReasonLOC;         /**< clock loss reset counter */
    uint8_t     ResetReasonILAD;        /**< illegal address reset counter */
    uint8_t     ResetReasonILOP;        /**< illegal instruction reset counter */
    uint8_t     ResetReasonCOP;         /**< watchdog reset counter */
    uint8_t     MCUType;                /**< always 1, hcs08 */
    uint8_t     HardwareCANActive;      /**< unknown */
    uint8_t     _reserved1[3];
    uint16_t    BootloaderVersion;      /**< unknown */
    uint16_t    ProgramState;           /**< unknown  */
    uint16_t    Portbyte1;              /**< unknown  */
    uint16_t    Portbyte2;              /**< unknown  */
    uint16_t    BaudrateBootloader1;    /**< encoded CAN speed */
    uint16_t    BaudrateBootloader2;    /**< encoded CAN speed */
    uint8_t     BootloaderIDExt1;       /**< bootloader CAN ID extended? */
    uint32_t    BootloaderID1;          /**< bootloader CAN ID */
    uint8_t     BootloaderIDCRC1;       /**< bootloader CAN ID CRC */
    uint8_t     BootloaderIDExt2;       /**< bootloader CAN ID extended? */
    uint32_t    BootloaderID2;          /**< bootloader CAN ID */
    uint8_t     BootloaderIDCRC2;       /**< bootloader CAN ID CRC */
    char        SoftwareVersion[20];    /**< application software version */
    char        ModuleName[30];         /**< application software name */
    uint8_t     BootloaderCANBus;       /**< unknown */
    uint16_t    COPWatchdogTimeout;     /**< unknown */
    uint8_t     _reserved2[7];
} MRS_parameters_t;

/** Direct-mapped version of the MRS parameter structure in EEPROM. */
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
