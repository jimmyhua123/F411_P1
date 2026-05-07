#ifndef FF_H
#define FF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint32_t UINT;
typedef uint32_t FSIZE_t;

typedef enum
{
  FR_OK = 0,
  FR_DISK_ERR,
  FR_INT_ERR,
  FR_NOT_READY,
  FR_NO_FILE,
  FR_NO_PATH,
  FR_INVALID_NAME,
  FR_DENIED,
  FR_EXIST,
  FR_INVALID_OBJECT,
  FR_WRITE_PROTECTED,
  FR_INVALID_DRIVE,
  FR_NOT_ENABLED,
  FR_NO_FILESYSTEM
} FRESULT;

typedef struct
{
  uint8_t mounted;
} FATFS;

typedef struct
{
  FSIZE_t fptr;
  FSIZE_t objsize;
  uint8_t opened;
} FIL;

#define FA_READ         0x01U
#define FA_WRITE        0x02U
#define FA_OPEN_EXISTING 0x00U
#define FA_CREATE_NEW   0x04U
#define FA_CREATE_ALWAYS 0x08U
#define FA_OPEN_ALWAYS  0x10U
#define FA_OPEN_APPEND  0x30U

FRESULT f_mount(FATFS *fs, const char *path, uint8_t opt);
FRESULT f_open(FIL *fp, const char *path, uint8_t mode);
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT f_sync(FIL *fp);
FRESULT f_close(FIL *fp);
FRESULT f_lseek(FIL *fp, FSIZE_t ofs);
FSIZE_t f_size(FIL *fp);

#ifdef __cplusplus
}
#endif

#endif /* FF_H */
