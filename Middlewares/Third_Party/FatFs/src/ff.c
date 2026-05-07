#include "ff.h"

/*
 * Weak fallback symbols keep the project buildable when STM32Cube FatFS has
 * not been generated yet. Add the real FatFS middleware to replace these.
 */
__attribute__((weak)) FRESULT f_mount(FATFS *fs, const char *path, uint8_t opt)
{
  (void)fs;
  (void)path;
  (void)opt;
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FRESULT f_open(FIL *fp, const char *path, uint8_t mode)
{
  (void)fp;
  (void)path;
  (void)mode;
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw)
{
  (void)fp;
  (void)buff;
  (void)btw;
  if (bw != 0)
  {
    *bw = 0U;
  }
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FRESULT f_sync(FIL *fp)
{
  (void)fp;
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FRESULT f_close(FIL *fp)
{
  (void)fp;
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FRESULT f_lseek(FIL *fp, FSIZE_t ofs)
{
  (void)fp;
  (void)ofs;
  return FR_NOT_ENABLED;
}

__attribute__((weak)) FSIZE_t f_size(FIL *fp)
{
  (void)fp;
  return 0U;
}
