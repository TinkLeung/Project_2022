
/**
 * File: Nvme.h
 */ 

#ifndef NVME_H
#define NVME_H

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <endian.h>
#include "linux/plugin.h"
#include "util/json.h"

#define unlikely(x) x

#include "nvme_base.h"

struct nvme_effects_log_page {
    __le32 acs[256];
    __le32 iocs[256];
    __u8   resv[2048];
};

struct nvme_error_log_page {
    __le64	error_count;
    __le16	sqid;
    __le16	cmdid;
    __le16	status_field;
    __le16	parm_error_location;
    __le64	lba;
    __le32	nsid;
    __u8	vs;
    __u8	trtype;
    __u8	resv[2];
    __le64	cs;
    __le16	trtype_spec_info;
    __u8	resv2[22];
};

struct nvme_firmware_log_page {
    __u8	afi;
    __u8	resv[7];
    __u64	frs[7];
    __u8	resv2[448];
};

/* idle and active power scales occupy the last 2 bits of the field */
#define POWER_SCALE(s) ((s) >> 6)

struct nvme_host_mem_buffer {
    __u32			hsize;
    __u32			hmdlal;
    __u32			hmdlau;
    __u32			hmdlec;
    __u8			rsvd16[4080];
};

struct nvme_auto_pst {
    __u32	data;
    __u32	rsvd32;
};

struct nvme_timestamp {
    __u8 timestamp[6];
    __u8 attr;
    __u8 rsvd;
};

struct nvme_controller_list {
    __le16 num;
    __le16 identifier[];
};

struct nvme_secondary_controller_entry {
    __le16 scid;	/* Secondary Controller Identifier */
    __le16 pcid;	/* Primary Controller Identifier */
    __u8   scs;	/* Secondary Controller State */
    __u8   rsvd5[3];
    __le16 vfn;	/* Virtual Function Number */
    __le16 nvq;	/* Number of VQ Flexible Resources Assigned */
    __le16 nvi;	/* Number of VI Flexible Resources Assigned */
    __u8   rsvd14[18];
};

struct nvme_secondary_controllers_list {
    __u8   num;
    __u8   rsvd[31];
    struct nvme_secondary_controller_entry sc_entry[127];
};

struct nvme_bar_cap {
    __u16	mqes;
    __u8	ams_cqr;
    __u8	to;
    __u16	bps_css_nssrs_dstrd;
    __u8	mpsmax_mpsmin;
    __u8	rsvd_pmrs;
};

#ifdef __CHECKER__
#define __force       __attribute__((force))
#else
#define __force
#endif

static inline __le16 cpu_to_le16(uint16_t x)
{
    return (__force __le16)htole16(x);
}
static inline __le32 cpu_to_le32(uint32_t x)
{
    return (__force __le32)htole32(x);
}
static inline __le64 cpu_to_le64(uint64_t x)
{
    return (__force __le64)htole64(x);
}

static inline uint16_t le16_to_cpu(__le16 x)
{
    return le16toh((__force __u16)x);
}
static inline uint32_t le32_to_cpu(__le32 x)
{
    return le32toh((__force __u32)x);
}
static inline uint64_t le64_to_cpu(__le64 x)
{
    return le64toh((__force __u64)x);
}

struct nvme_subsystem;
struct nvme_ctrl;

struct nvme_namespace {
    char *name;
    struct nvme_ctrl *ctrl;

    unsigned nsid;
    struct nvme_id_ns ns;
};

struct nvme_path {
    char *name;
};

struct nvme_ctrl {
    char *name;
    struct nvme_subsystem *subsys;

    char *address;
    char *transport;
    char *state;

    struct nvme_id_ctrl id;

    int    nr_namespaces;
    struct nvme_namespace *namespaces;

    int    nr_paths;
    struct nvme_path *paths;
};

struct nvme_subsystem {
    char *name;
    char *subsysnqn;

    int    nr_ctrls;
    struct nvme_ctrl *ctrls;

    int    nr_namespaces;
    struct nvme_namespace *namespaces;
};

struct nvme_topology {
    int    nr_subsystems;
    struct nvme_subsystem *subsystems;
};

struct ctrl_list_item {
    char *name;
    char *address;
    char *transport;
    char *state;
    char *ana_state;
    char *subsysnqn;
    char *traddr;
    char *trsvcid;
    char *host_traddr;
};

struct subsys_list_item {
    char *name;
    char *subsysnqn;
    int nctrls;
    struct ctrl_list_item *ctrls;
};

enum {
    NORMAL,
    JSON,
    BINARY,
};

struct connect_args {
    char *subsysnqn;
    char *transport;
    char *traddr;
    char *trsvcid;
    char *host_traddr;
};

#define SYS_NVME		"/sys/class/nvme"

bool ctrl_matches_connectargs(char *name, struct connect_args *args);
char *find_ctrl_with_connectargs(struct connect_args *args);
char *__parse_connect_arg(char *conargs, const char delim, const char *fieldnm);

extern const char *conarg_nqn;
extern const char *conarg_transport;
extern const char *conarg_traddr;
extern const char *conarg_trsvcid;
extern const char *conarg_host_traddr;
extern const char *dev;
extern const char *subsys_dir;

void register_extension(struct plugin *plugin);

#include "util/argconfig.h"
int parse_and_open(int argc, char **argv, const char *desc,
    const struct argconfig_commandline_options *clo, void *cfg, size_t size);

extern const char *devicename;

int __id_ctrl(int argc, char **argv, struct command *cmd, struct plugin *plugin, void (*vs)(__u8 *vs, struct json_object *root));
int	validate_output_format(char *format);
int open_dev(char *dev);
void close_dev(int fd);
int fw_download_ui(char* nvmedev, char* FileName, __u32 transize);
int fw_commit_ui(char* nvmedev, int slotID, unsigned long selectAction);


int get_nvme_ctrl_info(char *name, char *path, struct ctrl_list_item *ctrl,
            __u32 nsid);
struct subsys_list_item *get_subsys_list(int *subcnt, char *subsysnqn, __u32 nsid);
void free_subsys_list(struct subsys_list_item *slist, int n);
char *nvme_char_from_block(char *block);
int get_nsid(int fd);
void free_ctrl_list_item(struct ctrl_list_item *ctrls);
void *mmap_registers(const char *dev);

extern int current_index;
int scan_namespace_filter(const struct dirent *d);
int scan_ctrl_paths_filter(const struct dirent *d);
int scan_ctrls_filter(const struct dirent *d);
int scan_subsys_filter(const struct dirent *d);
int scan_dev_filter(const struct dirent *d);

int scan_subsystems(struct nvme_topology *t);
void free_topology(struct nvme_topology *t);
char *get_nvme_subsnqn(char *path);

/*
 * is_64bit_reg - It checks whether given offset of the controller register is
 *                64bit or not.
 * @offset: offset of controller register field in bytes
 *
 * It gives true if given offset is 64bit register, otherwise it returns false.
 *
 * Notes:  This function does not care about transport so that the offset is
 * not going to be checked inside of this function for the unsupported fields
 * in a specific transport.  For example, BPMBL(Boot Partition Memory Buffer
 * Location) register is not supported by fabrics, but it can be chcked here.
 */
static inline bool is_64bit_reg(__u32 offset)
{
    if (offset == NVME_REG_CAP ||
            offset == NVME_REG_ASQ ||
            offset == NVME_REG_ACQ ||
            offset == NVME_REG_BPMBL)
        return true;

    return false;
}


typedef struct {
    unsigned short  MP;                 // bit 0:15.    Maximum  Power (MP)
    
    unsigned char   Reserved0;          // bit 16:23

    unsigned char   MPS         : 1;    // bit 24: Max Power Scale (MPS)
    unsigned char   NOPS        : 1;    // bit 25: Non-Operational State (NOPS)
    unsigned char   Reserved1   : 6;    // bit 26:31

    unsigned long   ENLAT;              // bit 32:63.   Entry Latency (ENLAT)
    unsigned long    EXLAT;              // bit 64:95.   Exit Latency (EXLAT)

    unsigned char   RRT         : 5;    // bit 96:100.  Relative Read Throughput (RRT)
    unsigned char   Reserved2   : 3;    // bit 101:103

    unsigned char   RRL         : 5;    // bit 104:108  Relative Read Latency (RRL)
    unsigned char   Reserved3   : 3;    // bit 109:111

    unsigned char   RWT         : 5;    // bit 112:116  Relative Write Throughput (RWT)
    unsigned char   Reserved4   : 3;    // bit 117:119

    unsigned char   RWL         : 5;    // bit 120:124  Relative Write Latency (RWL)
    unsigned char   Reserved5   : 3;    // bit 125:127

    unsigned short  IDLP;               // bit 128:143  Idle Power (IDLP)

    unsigned char   Reserved6   : 6;    // bit 144:149
    unsigned char   IPS         : 2;    // bit 150:151  Idle Power Scale (IPS)
    
    unsigned char   Reserved7;          // bit 152:159

    unsigned short  ACTP;               // bit 160:175  Active Power (ACTP)

    unsigned char   APW         : 3;    // bit 176:178  Active Power Workload (APW)
    unsigned char   Reserved8   : 3;    // bit 179:181
    unsigned char   APS         : 2;    // bit 182:183  Active Power Scale (APS)


    unsigned char   Reserved9[9];       // bit 184:255.

} NVME_POWER_STATE_DESC, *PNVME_POWER_STATE_DESC;

typedef struct {
    //
    // byte 0 : 255, Controller Capabilities and Features
    //
    unsigned short  VID;                // byte 0:1.    M - PCI Vendor ID (VID)
    unsigned short  SSVID;              // byte 2:3.    M - PCI Subsystem Vendor ID (SSVID)
    unsigned char   SN[20];             // byte 4: 23.  M - Serial Number (SN)
    unsigned char   MN[40];             // byte 24:63.  M - Model Number (MN)
    unsigned char   FR[8];              // byte 64:71.  M - Firmware Revision (FR)
    unsigned char   RAB;                // byte 72.     M - Recommended Arbitration Burst (RAB)
    unsigned char   IEEE[3];            // byte 73:75.  M - IEEE OUI Identifier (IEEE). Controller Vendor code.

    struct {
        unsigned char   MultiPCIePorts      : 1;
        unsigned char   MultiControllers    : 1;
        unsigned char   SRIOV               : 1;
        unsigned char   Reserved            : 5;
    } CMIC;                     // byte 76.     O - Controller Multi-Path I/O and Namespace Sharing Capabilities (CMIC)

    unsigned char   MDTS;               // byte 77.     M - Maximum Data Transfer Size (MDTS)
    unsigned short  CNTLID;             // byte 78:79.   M - Controller ID (CNTLID)
    unsigned long   VER;                // byte 80:83.   M - Version (VER)
    unsigned long   RTD3R;              // byte 84:87.   M - RTD3 Resume Latency (RTD3R)
    unsigned long   RTD3E;              // byte 88:91.   M - RTD3 Entry Latency (RTD3E)

    struct {
        unsigned long   Reserved0                   : 8;
        unsigned long   NamespaceAttributeChanged   : 1;
        unsigned long   Reserved1                   : 23;
    } OAES;                     // byte 92:95.   M - Optional Asynchronous Events Supported (OAES)

    unsigned char   Reserved0[144];     // byte 96:239.
    unsigned char   ReservedForManagement[16];     // byte 240:255.  Refer to the NVMe Management Interface Specification for definition.

    //
    // byte 256 : 511, Admin Command Set Attributes
    //
    struct {
        unsigned short  SecurityCommands    : 1;
        unsigned short  FormatNVM           : 1;
        unsigned short  FirmwareCommands    : 1;
        unsigned short  NamespaceCommands   : 1;
        unsigned short  Reserved            : 12;
    } OACS;                     // byte 256:257. M - Optional Admin Command Support (OACS)

    unsigned char   ACL;                // byte 258.    M - Abort Command Limit (ACL)
    unsigned char   AERL;               // byte 259.    M - Asynchronous Event Request Limit (AERL)

    struct {
        unsigned char   Slot1ReadOnly   : 1;
        unsigned char   SlotCount       : 3;
        unsigned char   ActivationWithoutReset  : 1;
        unsigned char   Reserved        : 3;
    } FRMW;                     // byte 260.    M - Firmware Updates (FRMW)

    struct {
        unsigned char   SmartPagePerNamespace   : 1;
        unsigned char   CommandEffectsLog       : 1;
        unsigned char   Reserved                : 6;
    } LPA;                      // byte 261.    M - Log Page Attributes (LPA)

    unsigned char   ELPE;               // byte 262.    M - Error Log Page Entries (ELPE)
    unsigned char   NPSS;               // byte 263.    M - Number of Power States Support (NPSS)

    struct {
        unsigned char   CommandFormatInSpec : 1;
        unsigned char   Reserved            : 7;
    } AVSCC;                    // byte 264.    M - Admin Vendor Specific Command Configuration (AVSCC)

    struct {
        unsigned char   Supported       : 1;
        unsigned char   Reserved        : 7;
    } APSTA;                    // byte 265.     O - Autonomous Power State Transition Attributes (APSTA)

    unsigned short  WCTEMP;             // byte 266:267. M - Warning Composite Temperature Threshold (WCTEMP)
    unsigned short  CCTEMP;             // byte 268:269. M - Critical Composite Temperature Threshold (CCTEMP)
    unsigned short  MTFA;               // byte 270:271. O - Maximum Time for Firmware Activation (MTFA)
    unsigned long   HMPRE;              // byte 272:275. O - Host Memory Buffer Preferred Size (HMPRE)
    unsigned long   HMMIN;              // byte 276:279. O - Host Memory Buffer Minimum Size (HMMIN)
    unsigned char   TNVMCAP[16];        // byte 280:295. O - Total NVM Capacity (TNVMCAP)
    unsigned char   UNVMCAP[16];        // byte 296:311. O - Unallocated NVM Capacity (UNVMCAP)

    struct {
        unsigned long   RPMBUnitCount           : 3;    // Number of RPMB Units
        unsigned long   AuthenticationMethod    : 3;    // Authentication Method
        unsigned long   Reserved0               : 10;
        unsigned long   TotalSize               : 8;    // Total Size: in 128KB units.
        unsigned long   AccessSize              : 8;    // Access Size: in 512B units.
    } RPMBS;                    // byte 312:315. O - Replay Protected Memory Block Support (RPMBS)

    unsigned char   Reserved1[196];     // byte 316:511.

    //
    // byte 512 : 703, NVM Command Set Attributes
    //
    struct {
        unsigned char   RequiredEntrySize   : 4;    // The value is in bytes and is reported as a power of two (2^n).
        unsigned char   MaxEntrySize        : 4;    // This value is larger than or equal to the required SQ entry size.  The value is in bytes and is reported as a power of two (2^n).
    } SQES;                     // byte 512.    M - Submission Queue Entry Size (SQES)

    struct {
        unsigned char   RequiredEntrySize   : 4;    // The value is in bytes and is reported as a power of two (2^n).
        unsigned char   MaxEntrySize        : 4;    // This value is larger than or equal to the required CQ entry size. The value is in bytes and is reported as a power of two (2^n).
    } CQES;                     // byte 513.    M - Completion Queue Entry Size (CQES)

    unsigned char   Reserved2[2];       // byte 514:515.

    unsigned long   NN;                 // byte 516:519. M - Number of Namespaces (NN)

    struct {
        unsigned short  Compare             : 1;
        unsigned short  WriteUncorrectable  : 1;
        unsigned short  DatasetManagement   : 1;
        unsigned short  WriteZeroes         : 1;
        unsigned short  FeatureField        : 1;
        unsigned short  Reservations        : 1;

        unsigned short  Reserved            : 10;
    } ONCS;                     // byte 520:521. M - Optional NVM Command Support (ONCS)

    struct {
        unsigned short  CompareAndWrite             : 1;
        unsigned short  Reserved                    : 15;
    } FUSES;                    // byte 522:523. M - Fused Operation Support (FUSES)

    struct {
        unsigned char   FormatApplyToAll                : 1;
        unsigned char   SecureEraseApplyToAll           : 1;
        unsigned char   CryptographicEraseSupported     : 1;
        unsigned char   Reserved                        : 5;
    } FNA;                      // byte 524.     M - Format NVM Attributes (FNA)

    struct {
        unsigned char   Present     : 1;
        unsigned char   Reserved    : 7;
    } VWC;                      // byte 525.     M - Volatile Write Cache (VWC)

    unsigned short  AWUN;               // byte 526:527. M - Atomic Write Unit Normal (AWUN)
    unsigned short  AWUPF;              // byte 528:529. M - Atomic Write Unit Power Fail (AWUPF)

    struct {
        unsigned char   CommandFormatInSpec : 1;
        unsigned char   Reserved            : 7;
    } NVSCC;                    // byte 530.     M - NVM Vendor Specific Command Configuration (NVSCC)

    unsigned char   Reserved3;          // byte 531.

    unsigned short  ACWU;               // byte 532:533  O - Atomic Compare & Write Unit (ACWU)

    unsigned char   Reserved4[2];       // byte 534:535.

    struct {
        unsigned long   SGLSupported            : 1;
        unsigned long   Reserved0               : 15;
        unsigned long   BitBucketDescrSupported : 1;
        unsigned long   ByteAlignedContiguousPhysicalBuffer : 1;
        unsigned long   SGLLengthLargerThanDataLength       : 1;
        unsigned long   Reserved1               : 13;
    } SGLS;                     // byte 536:539. O - SGL Support (SGLS)

    unsigned char   Reserved5[164];     // byte 540:703.

    //
    // byte 704 : 2047, I/O Command Set Attributes
    //
    unsigned char   Reserved6[1344];    // byte 704:2047.

    //
    // byte 2048 : 3071, Power State Descriptors
    //
    NVME_POWER_STATE_DESC   PDS[32];    // byte 2048:2079. M - Power State 0 Descriptor (PSD0):  This field indicates the characteristics of power state 0
                                        // byte 2080:2111. O - Power State 1 Descriptor (PSD1):  This field indicates the characteristics of power state 1
                                        // byte 2112:2143. O - Power State 2 Descriptor (PSD1):  This field indicates the characteristics of power state 2
                                        // byte 2144:2175. O - Power State 3 Descriptor (PSD1):  This field indicates the characteristics of power state 3
                                        // byte 2176:2207. O - Power State 4 Descriptor (PSD1):  This field indicates the characteristics of power state 4
                                        // byte 2208:2239. O - Power State 5 Descriptor (PSD1):  This field indicates the characteristics of power state 5
                                        // byte 2240:2271. O - Power State 6 Descriptor (PSD1):  This field indicates the characteristics of power state 6
                                        // byte 2272:2303. O - Power State 7 Descriptor (PSD1):  This field indicates the characteristics of power state 7
                                        // byte 2304:2335. O - Power State 8 Descriptor (PSD1):  This field indicates the characteristics of power state 8
                                        // byte 2336:2367. O - Power State 9 Descriptor (PSD1):  This field indicates the characteristics of power state 9
                                        // byte 2368:2399. O - Power State 10 Descriptor (PSD1):  This field indicates the characteristics of power state 10
                                        // byte 2400:2431. O - Power State 11 Descriptor (PSD1):  This field indicates the characteristics of power state 11
                                        // byte 2432:2463. O - Power State 12 Descriptor (PSD1):  This field indicates the characteristics of power state 12
                                        // byte 2464:2495. O - Power State 13 Descriptor (PSD1):  This field indicates the characteristics of power state 13
                                        // byte 2496:2527. O - Power State 14 Descriptor (PSD1):  This field indicates the characteristics of power state 14
                                        // byte 2528:2559. O - Power State 15 Descriptor (PSD1):  This field indicates the characteristics of power state 15
                                        // byte 2560:2591. O - Power State 16 Descriptor (PSD1):  This field indicates the characteristics of power state 16
                                        // byte 2592:2623. O - Power State 17 Descriptor (PSD1):  This field indicates the characteristics of power state 17
                                        // byte 2624:2655. O - Power State 18 Descriptor (PSD1):  This field indicates the characteristics of power state 18
                                        // byte 2656:2687. O - Power State 19 Descriptor (PSD1):  This field indicates the characteristics of power state 19
                                        // byte 2688:2719. O - Power State 20 Descriptor (PSD1):  This field indicates the characteristics of power state 20
                                        // byte 2720:2751. O - Power State 21 Descriptor (PSD1):  This field indicates the characteristics of power state 21
                                        // byte 2752:2783. O - Power State 22 Descriptor (PSD1):  This field indicates the characteristics of power state 22
                                        // byte 2784:2815. O - Power State 23 Descriptor (PSD1):  This field indicates the characteristics of power state 23
                                        // byte 2816:2847. O - Power State 24 Descriptor (PSD1):  This field indicates the characteristics of power state 24
                                        // byte 2848:2879. O - Power State 25 Descriptor (PSD1):  This field indicates the characteristics of power state 25
                                        // byte 2880:2911. O - Power State 26 Descriptor (PSD1):  This field indicates the characteristics of power state 26
                                        // byte 2912:2943. O - Power State 27 Descriptor (PSD1):  This field indicates the characteristics of power state 27
                                        // byte 2944:2975. O - Power State 28 Descriptor (PSD1):  This field indicates the characteristics of power state 28
                                        // byte 2976:3007. O - Power State 29 Descriptor (PSD1):  This field indicates the characteristics of power state 29
                                        // byte 3008:3039. O - Power State 30 Descriptor (PSD1):  This field indicates the characteristics of power state 30
                                        // byte 3040:3071. O - Power State 31 Descriptor (PSD1):  This field indicates the characteristics of power state 31

    //
    // byte 3072 : 4095, Vendor Specific
    //
    unsigned char   VS[1024];           // byte 3072 : 4095.

} NVME_IDENTIFY_CONTROLLER_DATA, *PNVME_IDENTIFY_CONTROLLER_DATA;


//
// Information of log: NVME_LOG_PAGE_HEALTH_INFO. Size: 512 bytes
//
typedef struct
{
    union
    {
        struct
        {
            unsigned char   AvailableSpaceLow   : 1;                    // If set to 1, then the available spare space has fallen below the threshold.
            unsigned char   TemperatureThreshold : 1;                   // If set to 1, then a temperature is above an over temperature threshold or below an under temperature threshold.
            unsigned char   ReliabilityDegraded : 1;                    // If set to 1, then the device reliability has been degraded due to significant media related  errors or any internal error that degrades device reliability.
            unsigned char   ReadOnly            : 1;                    // If set to 1, then the media has been placed in read only mode
            unsigned char   VolatileMemoryBackupDeviceFailed    : 1;    // If set to 1, then the volatile memory backup device has failed. This field is only valid if the controller has a volatile memory backup solution.
            unsigned char   Reserved                            : 3;
        } DUMMYSTRUCTNAME;
        unsigned char AsUchar;
    } CriticalWarning;    // This field indicates critical warnings for the state of the  controller. Each bit corresponds to a critical warning type; multiple bits may be set.

    unsigned char   Temperature[2];                 // Temperature: Contains the temperature of the overall device (controller and NVM included) in units of Kelvin. If the temperature exceeds the temperature threshold, refer to section 5.12.1.4, then an asynchronous event completion may occur
    unsigned char   AvailableSpare;                 // Available Spare:  Contains a normalized percentage (0 to 100%) of the remaining spare capacity available
    unsigned char   AvailableSpareThreshold;        // Available Spare Threshold:  When the Available Spare falls below the threshold indicated in this field, an asynchronous event  completion may occur. The value is indicated as a normalized percentage (0 to 100%).
    unsigned char   PercentageUsed;                 // Percentage Used
    unsigned char   Reserved0[26];

    unsigned char   DataUnitRead[16];               // Data Units Read:  Contains the number of 512 byte data units the host has read from the controller; this value does not include metadata. This value is reported in thousands (i.e., a value of 1 corresponds to 1000 units of 512 bytes read)  and is rounded up.  When the LBA size is a value other than 512 bytes, the controller shall convert the amount of data read to 512 byte units. For the NVM command set, logical blocks read as part of Compare and Read operations shall be included in this value
    unsigned char   DataUnitWritten[16];            // Data Units Written: Contains the number of 512 byte data units the host has written to the controller; this value does not include metadata. This value is reported in thousands (i.e., a value of 1 corresponds to 1000 units of 512 bytes written)  and is rounded up.  When the LBA size is a value other than 512 bytes, the controller shall convert the amount of data written to 512 byte units. For the NVM command set, logical blocks written as part of Write operations shall be included in this value. Write Uncorrectable commands shall not impact this value.
    unsigned char   HostReadCommands[16];           // Host Read Commands:  Contains the number of read commands  completed by  the controller. For the NVM command set, this is the number of Compare and Read commands.
    unsigned char   HostWrittenCommands[16];        // Host Write Commands:  Contains the number of write commands  completed by  the controller. For the NVM command set, this is the number of Write commands.
    unsigned char   ControllerBusyTime[16];         // Controller Busy Time:  Contains the amount of time the controller is busy with I/O commands. The controller is busy when there is a command outstanding to an I/O Queue (specifically, a command was issued via an I/O Submission Queue Tail doorbell write and the corresponding  completion queue entry  has not been posted yet to the associated I/O Completion Queue). This value is reported in minutes.
    unsigned char   PowerCycle[16];                 // Power Cycles: Contains the number of power cycles.
    unsigned char   PowerOnHours[16];               // Power On Hours: Contains the number of power-on hours. This does not include time that the controller was powered and in a low power state condition.
    unsigned char   UnsafeShutdowns[16];            // Unsafe Shutdowns: Contains the number of unsafe shutdowns. This count is incremented when a shutdown notification (CC.SHN) is not received prior to loss of power.
    unsigned char   MediaErrors[16];                // Media Errors:  Contains the number of occurrences where the controller detected an unrecovered data integrity error. Errors such as uncorrectable ECC, CRC checksum failure, or LBA tag mismatch are included in this field.
    unsigned char   ErrorInfoLogEntryCount[16];     // Number of Error Information Log Entries:  Contains the number of Error Information log entries over the life of the controller
    unsigned long   WarningCompositeTemperatureTime;     // Warning Composite Temperature Time: Contains the amount of time in minutes that the controller is operational and the Composite Temperature is greater than or equal to the Warning Composite Temperature Threshold (WCTEMP) field and less than the Critical Composite Temperature Threshold (CCTEMP) field in the Identify Controller data structure
    unsigned long   CriticalCompositeTemperatureTime;    // Critical Composite Temperature Time: Contains the amount of time in minutes that the controller is operational and the Composite Temperature is greater the Critical Composite Temperature Threshold (CCTEMP) field in the Identify Controller data structure
    unsigned short  TemperatureSensor1;          // Contains the current temperature reported by temperature sensor 1.
    unsigned short  TemperatureSensor2;          // Contains the current temperature reported by temperature sensor 2.
    unsigned short  TemperatureSensor3;          // Contains the current temperature reported by temperature sensor 3.
    unsigned short  TemperatureSensor4;          // Contains the current temperature reported by temperature sensor 4.
    unsigned short  TemperatureSensor5;          // Contains the current temperature reported by temperature sensor 5.
    unsigned short  TemperatureSensor6;          // Contains the current temperature reported by temperature sensor 6.
    unsigned short  TemperatureSensor7;          // Contains the current temperature reported by temperature sensor 7.
    unsigned short  TemperatureSensor8;          // Contains the current temperature reported by temperature sensor 8.
    unsigned char   Reserved1[296];

} NVME_HEALTH_INFO_LOG, *PNVME_HEALTH_INFO_LOG;




#endif

