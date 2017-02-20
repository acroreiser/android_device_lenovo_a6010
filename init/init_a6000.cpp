/*
   Copyright (c) 2016, The CyanogenMod Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "init_msm8916.h"
#include "property_service.h"
#include <sys/sysinfo.h>
#include "vendor_init.h"
#include "log.h"
#include "util.h"

 #define MAX(a, b) (((a) > (b)) ? (a) : (b))
 
 #define ALPHABET_LEN 256
 #define KB 1024
 
 #define IMG_PART_PATH "/dev/block/bootdevice/by-name/modem"
 #define IMG_VER_STR "QC_IMAGE_VERSION_STRING="
 #define IMG_VER_STR_LEN 24
 #define IMG_VER_BUF_LEN 255
 #define IMG_SZ 32000 * KB    /* MMAP 32000K of modem, modem partition is 64000K */

static int read_file2(const char *fname, char *data, int max_size)
{
    int fd, rc;

    if (max_size < 1)
        return 0;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        ERROR("failed to open '%s'\n", fname);
        return 0;
    }

    rc = read(fd, data, max_size - 1);
    if ((rc > 0) && (rc < max_size))
        data[rc] = '\0';
    else
        data[0] = '\0';
    close(fd);

    return 1;
}

static void init_alarm_boot_properties()
{
    char const *alarm_file = "/proc/sys/kernel/boot_reason";
    char buf[64];
    std::string tmp = property_get("ro.boot.alarmboot");

    if (read_file2(alarm_file, buf, sizeof(buf))) {
        /*
         * Setup ro.alarm_boot value to true when it is RTC triggered boot up
         * For existing PMIC chips, the following mapping applies
         * for the value of boot_reason:
         *
         * 0 -> unknown
         * 1 -> hard reset
         * 2 -> sudden momentary power loss (SMPL)
         * 3 -> real time clock (RTC)
         * 4 -> DC charger inserted
         * 5 -> USB charger insertd
         * 6 -> PON1 pin toggled (for secondary PMICs)
         * 7 -> CBLPWR_N pin toggled (for external power supply)
         * 8 -> KPDPWR_N pin toggled (power key pressed)
         */
        if (buf[0] == '3' || tmp == "true")
            property_set("ro.alarm_boot", "true");
        else
            property_set("ro.alarm_boot", "false");
    }
}
/* Boyer-Moore string search implementation from Wikipedia */
 
 /* Return longest suffix length of suffix ending at str[p] */
 static int max_suffix_len(const char *str, size_t str_len, size_t p) {
     uint32_t i;
 
     for (i = 0; (str[p - i] == str[str_len - 1 - i]) && (i < p); ) {
         i++;
     }
 
     return i;
 }
 
 /* Generate table of distance between last character of pat and rightmost
  * occurrence of character c in pat
  */
 static void bm_make_delta1(int *delta1, const char *pat, size_t pat_len) {
     uint32_t i;
     for (i = 0; i < ALPHABET_LEN; i++) {
         delta1[i] = pat_len;
     }
     for (i = 0; i < pat_len - 1; i++) {
         uint8_t idx = (uint8_t) pat[i];
         delta1[idx] = pat_len - 1 - i;
     }
 }
 
 /* Generate table of next possible full match from mismatch at pat[p] */
 static void bm_make_delta2(int *delta2, const char *pat, size_t pat_len) {
     int p;
     uint32_t last_prefix = pat_len - 1;
 
     for (p = pat_len - 1; p >= 0; p--) {
         /* Compare whether pat[p-pat_len] is suffix of pat */
         if (strncmp(pat + p, pat, pat_len - p) == 0) {
             last_prefix = p + 1;
         }
         delta2[p] = last_prefix + (pat_len - 1 - p);
     }
 
     for (p = 0; p < (int) pat_len - 1; p++) {
         /* Get longest suffix of pattern ending on character pat[p] */
         int suf_len = max_suffix_len(pat, pat_len, p);
         if (pat[p - suf_len] != pat[pat_len - 1 - suf_len]) {
             delta2[pat_len - 1 - suf_len] = pat_len - 1 - p + suf_len;
         }
     }
 }
 
 static char * bm_search(const char *str, size_t str_len, const char *pat,
         size_t pat_len) {
     int delta1[ALPHABET_LEN];
     int delta2[pat_len];
     int i;
 
     bm_make_delta1(delta1, pat, pat_len);
     bm_make_delta2(delta2, pat, pat_len);
 
     if (pat_len == 0) {
         return (char *) str;
     }
 
     i = pat_len - 1;
     while (i < (int) str_len) {
         int j = pat_len - 1;
         while (j >= 0 && (str[i] == pat[j])) {
             i--;
             j--;
         }
         if (j < 0) {
             return (char *) (str + i + 1);
         }
         i += MAX(delta1[(uint8_t) str[i]], delta2[j]);
     }
 
     return NULL;
 }
 
 static int get_img_version(char *ver_str, size_t len) {
     int ret = 0;
     int fd;
     char *img_data = NULL;
     char *offset = NULL;
 
     fd = open(IMG_PART_PATH, O_RDONLY);
     if (fd < 0) {
         ret = errno;
         goto err_ret;
     }
 
     img_data = (char *) mmap(NULL, IMG_SZ, PROT_READ, MAP_PRIVATE, fd, 0);
     if (img_data == (char *)-1) {
         ret = errno;
         goto err_fd_close;
     }
 
     /* Do Boyer-Moore search across IMG data */
     offset = bm_search(img_data, IMG_SZ, IMG_VER_STR, IMG_VER_STR_LEN);
     if (offset != NULL) {
         strncpy(ver_str, offset + IMG_VER_STR_LEN, len);
     } else {
         ret = -ENOENT;
     }
 
     munmap(img_data, IMG_SZ);
 err_fd_close:
     close(fd);
 err_ret:
     return ret;
 }

void init_target_properties()
 {
     char modem_version[IMG_VER_BUF_LEN];
     int rc;
    std::string product = property_get("ro.product.name");
    if ((strstr(product.c_str(), "a6000") == NULL))
     return;
    rc = get_img_version(modem_version, IMG_VER_BUF_LEN);
    if (!rc) {
        property_set("gsm.version.baseband", modem_version);
        ERROR("Detected modem version=%s\n", modem_version);
}

}
void vendor_load_properties()
{
    init_target_properties();
    init_alarm_boot_properties();
}
