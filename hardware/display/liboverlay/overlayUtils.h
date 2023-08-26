/*
* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*    * Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*    * Redistributions in binary form must reproduce the above
*      copyright notice, this list of conditions and the following
*      disclaimer in the documentation and/or other materials provided
*      with the distribution.
*    * Neither the name of The Linux Foundation nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OVERLAY_UTILS_H
#define OVERLAY_UTILS_H

#include <cutils/log.h> // ALOGE, etc
#include <errno.h>
#include <fcntl.h> // open, O_RDWR, etc
#include <hardware/hardware.h>
#include <hardware/gralloc.h> // buffer_handle_t
#include <linux/msm_mdp.h> // flags
#include <linux/msm_rotator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/Log.h>
#include "gralloc_priv.h" //for interlace

// Older platforms do not support Venus
#ifndef VENUS_COLOR_FORMAT
#define MDP_Y_CBCR_H2V2_VENUS MDP_IMGTYPE_LIMIT
#endif

/*
*
* Collection of utilities functions/structs/enums etc...
*
* */

// comment that out if you want to remove asserts
// or put it as -D in Android.mk. your choice.
#define OVERLAY_HAS_ASSERT

#ifdef OVERLAY_HAS_ASSERT
# define OVASSERT(x, ...) if(!(x)) { ALOGE(__VA_ARGS__); abort(); }
#else
# define OVASSERT(x, ...) ALOGE_IF(!(x), __VA_ARGS__)
#endif // OVERLAY_HAS_ASSERT

#define DEBUG_OVERLAY 0
#define PROFILE_OVERLAY 0

#ifndef MDSS_MDP_RIGHT_MIXER
#define MDSS_MDP_RIGHT_MIXER 0x100
#endif

#ifndef MDP_OV_PIPE_FORCE_DMA
#define MDP_OV_PIPE_FORCE_DMA 0x4000
#endif

#ifndef MDSS_MDP_DUAL_PIPE
#define MDSS_MDP_DUAL_PIPE 0x200
#endif

#define FB_DEVICE_TEMPLATE "/dev/graphics/fb%u"

namespace overlay {

// fwd
class Overlay;
class OvFD;

/* helper function to open by using fbnum */
bool open(OvFD& fd, uint32_t fbnum, const char* const dev,
    int flags = O_RDWR);

namespace utils {
struct Whf;
struct Dim;

inline uint32_t setBit(uint32_t x, uint32_t mask) {
    return (x | mask);
}

inline uint32_t clrBit(uint32_t x, uint32_t mask) {
    return (x & ~mask);
}

/* Utility class to help avoid copying instances by making the copy ctor
* and assignment operator private
*
* Usage:
*    class SomeClass : utils::NoCopy {...};
*/
class NoCopy {
protected:
    NoCopy(){}
    ~NoCopy() {}
private:
    NoCopy(const NoCopy&);
    const NoCopy& operator=(const NoCopy&);
};

bool isMdssRotator();
void normalizeCrop(uint32_t& xy, uint32_t& wh);

template <class Type>
void swapWidthHeight(Type& width, Type& height);

struct Dim {
    Dim () : x(0), y(0),
    w(0), h(0),
    o(0) {}
    Dim(uint32_t _x, uint32_t _y, uint32_t _w, uint32_t _h) :
        x(_x), y(_y),
        w(_w), h(_h) {}
    Dim(uint32_t _x, uint32_t _y, uint32_t _w, uint32_t _h, uint32_t _o) :
        x(_x), y(_y),
        w(_w), h(_h),
        o(_o) {}
    bool check(uint32_t _w, uint32_t _h) const {
        return (x+w <= _w && y+h <= _h);

    }

    bool operator==(const Dim& d) const {
        return d.x == x && d.y == y &&
                d.w == w && d.h == h &&
                d.o == o;
    }

    bool operator!=(const Dim& d) const {
        return !operator==(d);
    }

    void dump() const;
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
    uint32_t o;
};

// TODO have Whfz

struct Whf {
    Whf() : w(0), h(0), format(0), size(0) {}
    Whf(uint32_t wi, uint32_t he, uint32_t f) :
        w(wi), h(he), format(f), size(0) {}
    Whf(uint32_t wi, uint32_t he, uint32_t f, uint32_t s) :
        w(wi), h(he), format(f), size(s) {}
    // FIXME not comparing size at the moment
    bool operator==(const Whf& whf) const {
        return whf.w == w && whf.h == h &&
                whf.format == format;
    }
    bool operator!=(const Whf& whf) const {
        return !operator==(whf);
    }
    void dump() const;
    uint32_t w;
    uint32_t h;
    uint32_t format;
    uint32_t size;
};

enum { MAX_PATH_LEN = 256 };

enum { DEFAULT_PLANE_ALPHA = 0xFF };

/**
 * Rotator flags: not to be confused with orientation flags.
 * Usually, you want to open the rotator to make sure it is
 * ready for business.
 * */
 enum eRotFlags {
    ROT_FLAGS_NONE = 0,
    //Use rotator for 0 rotation. It is used anyway for others.
    ROT_0_ENABLED = 1 << 0,
    //Enable rotator downscale optimization for hardware bugs not handled in
    //driver. If downscale optimizatation is required,
    //then rotator will be used even if its 0 rotation case.
    ROT_DOWNSCALE_ENABLED = 1 << 1,
    ROT_PREROTATED = 1 << 2,
};

enum eRotDownscale {
    ROT_DS_NONE = 0,
    ROT_DS_HALF = 1,
    ROT_DS_FOURTH = 2,
    ROT_DS_EIGHTH = 3,
};

/*
 * Various mdp flags like PIPE SHARE, DEINTERLACE etc...
 * kernel/common/linux/msm_mdp.h
 * INTERLACE_MASK: hardware/qcom/display/libgralloc/badger/fb_priv.h
 * */
enum eMdpFlags {
    OV_MDP_FLAGS_NONE = 0,
    OV_MDP_PIPE_SHARE =  MDP_OV_PIPE_SHARE,
    OV_MDP_PIPE_FORCE_DMA = MDP_OV_PIPE_FORCE_DMA,
    OV_MDP_DEINTERLACE = MDP_DEINTERLACE,
    OV_MDP_SECURE_OVERLAY_SESSION = MDP_SECURE_OVERLAY_SESSION,
    OV_MDP_SECURE_DISPLAY_OVERLAY_SESSION = MDP_SECURE_DISPLAY_OVERLAY_SESSION,
    OV_MDP_SOURCE_ROTATED_90 = MDP_SOURCE_ROTATED_90,
    OV_MDP_BACKEND_COMPOSITION = MDP_BACKEND_COMPOSITION,
    OV_MDP_BLEND_FG_PREMULT = MDP_BLEND_FG_PREMULT,
    OV_MDP_FLIP_H = MDP_FLIP_LR,
    OV_MDP_FLIP_V = MDP_FLIP_UD,
    OV_MDSS_MDP_RIGHT_MIXER = MDSS_MDP_RIGHT_MIXER,
    OV_MDP_PP_EN = MDP_OVERLAY_PP_CFG_EN,
    OV_MDSS_MDP_BWC_EN = MDP_BWC_EN,
    OV_MDSS_MDP_DUAL_PIPE = MDSS_MDP_DUAL_PIPE,
    OV_MDP_SOLID_FILL = MDP_SOLID_FILL,
};

enum eZorder {
    ZORDER_0 = 0,
    ZORDER_1,
    ZORDER_2,
    ZORDER_3,
    Z_SYSTEM_ALLOC = 0xFFFF
};

enum eMdpPipeType {
    OV_MDP_PIPE_RGB = 0,
    OV_MDP_PIPE_VG,
    OV_MDP_PIPE_DMA,
    OV_MDP_PIPE_ANY, //Any
};

// Identify destination pipes
// TODO Names useless, replace with int and change all interfaces
enum eDest {
    OV_P0 = 0,
    OV_P1,
    OV_P2,
    OV_P3,
    OV_P4,
    OV_P5,
    OV_P6,
    OV_P7,
    OV_P8,
    OV_P9,
    OV_INVALID,
    OV_MAX = OV_INVALID,
};

/* Used when a buffer is split over 2 pipes and sent to display */
enum {
    OV_LEFT_SPLIT = 0,
    OV_RIGHT_SPLIT,
};

/* values for copybit_set_parameter(OVERLAY_TRANSFORM) */
enum eTransform {
    /* No rot */
    OVERLAY_TRANSFORM_0 = 0x0,
    /* flip source image horizontally 0x1 */
    OVERLAY_TRANSFORM_FLIP_H = HAL_TRANSFORM_FLIP_H,
    /* flip source image vertically 0x2 */
    OVERLAY_TRANSFORM_FLIP_V = HAL_TRANSFORM_FLIP_V,
    /* rotate source image 180 degrees
     * It is basically bit-or-ed  H | V == 0x3 */
    OVERLAY_TRANSFORM_ROT_180 = HAL_TRANSFORM_ROT_180,
    /* rotate source image 90 degrees 0x4 */
    OVERLAY_TRANSFORM_ROT_90 = HAL_TRANSFORM_ROT_90,
    /* rotate source image 90 degrees and flip horizontally 0x5 */
    OVERLAY_TRANSFORM_ROT_90_FLIP_H = HAL_TRANSFORM_ROT_90 |
                                      HAL_TRANSFORM_FLIP_H,
    /* rotate source image 90 degrees and flip vertically 0x6 */
    OVERLAY_TRANSFORM_ROT_90_FLIP_V = HAL_TRANSFORM_ROT_90 |
                                      HAL_TRANSFORM_FLIP_V,
    /* rotate source image 270 degrees
     * Basically 180 | 90 == 0x7 */
    OVERLAY_TRANSFORM_ROT_270 = HAL_TRANSFORM_ROT_270,
    /* rotate invalid like in Transform.h */
    OVERLAY_TRANSFORM_INV = 0x80
};

enum eBlending {
    OVERLAY_BLENDING_UNDEFINED = 0x0,
    /* No blending */
    OVERLAY_BLENDING_OPAQUE,
    /* src.rgb + dst.rgb*(1-src_alpha) */
    OVERLAY_BLENDING_PREMULT,
    /* src.rgb * src_alpha + dst.rgb (1 - src_alpha) */
    OVERLAY_BLENDING_COVERAGE,
};

// Used to consolidate pipe params
struct PipeArgs {
    PipeArgs() : mdpFlags(OV_MDP_FLAGS_NONE),
        zorder(Z_SYSTEM_ALLOC),
        rotFlags(ROT_FLAGS_NONE),
        planeAlpha(DEFAULT_PLANE_ALPHA),
        blending(OVERLAY_BLENDING_COVERAGE){
    }

    PipeArgs(eMdpFlags f, Whf _whf,
            eZorder z, eRotFlags r,
            int pA = DEFAULT_PLANE_ALPHA, eBlending b = OVERLAY_BLENDING_COVERAGE) :
        mdpFlags(f),
        whf(_whf),
        zorder(z),
        rotFlags(r),
        planeAlpha(pA),
        blending(b){
    }

    eMdpFlags mdpFlags; // for mdp_overlay flags
    Whf whf;
    eZorder zorder; // stage number
    eRotFlags rotFlags;
    int planeAlpha;
    eBlending blending;
};

// Cannot use HW_OVERLAY_MAGNIFICATION_LIMIT, since at the time
// of integration, HW_OVERLAY_MAGNIFICATION_LIMIT was a define
enum { HW_OV_MAGNIFICATION_LIMIT = 20,
    HW_OV_MINIFICATION_LIMIT  = 8
};

inline void setMdpFlags(eMdpFlags& f, eMdpFlags v) {
    f = static_cast<eMdpFlags>(setBit(f, v));
}

inline void clearMdpFlags(eMdpFlags& f, eMdpFlags v) {
    f = static_cast<eMdpFlags>(clrBit(f, v));
}

enum { FB0, FB1, FB2 };

struct ScreenInfo {
    ScreenInfo() : mFBWidth(0),
    mFBHeight(0),
    mFBbpp(0),
    mFBystride(0) {}
    void dump(const char* const s) const;
    uint32_t mFBWidth;
    uint32_t mFBHeight;
    uint32_t mFBbpp;
    uint32_t mFBystride;
};

int getMdpFormat(int format);
int getMdpFormat(int format, bool tileEnabled);
int getHALFormat(int mdpFormat);
void getDecimationFactor(const int& src_w, const int& src_h,
        const int& dst_w, const int& dst_h, uint8_t& horzDeci,
        uint8_t& vertDeci);

/* flip is upside down and such. V, H flip
 * rotation is 90, 180 etc
 * It returns MDP related enum/define that match rot+flip*/
int getMdpOrient(eTransform rotation);
const char* getFormatString(int format);

template <class T>
inline void memset0(T& t) { ::memset(&t, 0, sizeof(t)); }

template <class T> inline void swap ( T& a, T& b )
{
    T c(a); a=b; b=c;
}

template<typename T> inline T max(T a, T b) { return (a > b) ? a : b; }

template<typename T> inline T min(T a, T b) { return (a < b) ? a : b; }

inline int alignup(int value, int a) {
    //if align = 0, return the value. Else, do alignment.
    return a ? ((((value - 1) / a) + 1) * a) : value;
}

inline int aligndown(int value, int a) {
    //if align = 0, return the value. Else, do alignment.
    return a ? ((value) & ~(a-1)) : value;
}

// FIXME that align should replace the upper one.
inline int align(int value, int a) {
    //if align = 0, return the value. Else, do alignment.
    return a ? ((value + (a-1)) & ~(a-1)) : value;
}

inline bool isYuv(uint32_t format) {
    switch(format){
        case MDP_Y_CBCR_H2V1:
        case MDP_Y_CBCR_H2V2:
        case MDP_Y_CRCB_H2V2:
        case MDP_Y_CRCB_H1V1:
        case MDP_Y_CRCB_H2V1:
        case MDP_Y_CRCB_H2V2_TILE:
        case MDP_Y_CBCR_H2V2_TILE:
        case MDP_Y_CR_CB_H2V2:
        case MDP_Y_CR_CB_GH2V2:
        case MDP_Y_CBCR_H2V2_VENUS:
        case MDP_YCBYCR_H2V1:
        case MDP_YCRYCB_H2V1:
            return true;
        default:
            return false;
    }
    return false;
}

inline bool isRgb(uint32_t format) {
    switch(format) {
        case MDP_RGBA_8888:
        case MDP_BGRA_8888:
        case MDP_RGBX_8888:
        case MDP_RGB_565:
            return true;
        default:
            return false;
    }
    return false;
}

inline const char* getFormatString(int format){
    #define STR(f) #f;
    static const char* formats[MDP_IMGTYPE_LIMIT + 1] = {0};
    formats[MDP_RGB_565] = STR(MDP_RGB_565);
    formats[MDP_XRGB_8888] = STR(MDP_XRGB_8888);
    formats[MDP_Y_CBCR_H2V2] = STR(MDP_Y_CBCR_H2V2);
    formats[MDP_Y_CBCR_H2V2_ADRENO] = STR(MDP_Y_CBCR_H2V2_ADRENO);
    formats[MDP_ARGB_8888] = STR(MDP_ARGB_8888);
    formats[MDP_RGB_888] = STR(MDP_RGB_888);
    formats[MDP_Y_CRCB_H2V2] = STR(MDP_Y_CRCB_H2V2);
    formats[MDP_YCBYCR_H2V1] = STR(MDP_YCBYCR_H2V1);
    formats[MDP_YCRYCB_H2V1] = STR(MDP_YCRYCB_H2V1);
    formats[MDP_CBYCRY_H2V1] = STR(MDP_CBYCRY_H2V1);
    formats[MDP_Y_CRCB_H2V1] = STR(MDP_Y_CRCB_H2V1);
    formats[MDP_Y_CBCR_H2V1] = STR(MDP_Y_CBCR_H2V1);
    formats[MDP_Y_CRCB_H1V2] = STR(MDP_Y_CRCB_H1V2);
    formats[MDP_Y_CBCR_H1V2] = STR(MDP_Y_CBCR_H1V2);
    formats[MDP_RGBA_8888] = STR(MDP_RGBA_8888);
    formats[MDP_BGRA_8888] = STR(MDP_BGRA_8888);
    formats[MDP_RGBX_8888] = STR(MDP_RGBX_8888);
    formats[MDP_Y_CRCB_H2V2_TILE] = STR(MDP_Y_CRCB_H2V2_TILE);
    formats[MDP_Y_CBCR_H2V2_TILE] = STR(MDP_Y_CBCR_H2V2_TILE);
    formats[MDP_Y_CR_CB_H2V2] = STR(MDP_Y_CR_CB_H2V2);
    formats[MDP_Y_CR_CB_GH2V2] = STR(MDP_Y_CR_CB_GH2V2);
    formats[MDP_Y_CB_CR_H2V2] = STR(MDP_Y_CB_CR_H2V2);
    formats[MDP_Y_CRCB_H1V1] = STR(MDP_Y_CRCB_H1V1);
    formats[MDP_Y_CBCR_H1V1] = STR(MDP_Y_CBCR_H1V1);
    formats[MDP_YCRCB_H1V1] = STR(MDP_YCRCB_H1V1);
    formats[MDP_YCBCR_H1V1] = STR(MDP_YCBCR_H1V1);
    formats[MDP_BGR_565] = STR(MDP_BGR_565);
    formats[MDP_BGR_888] = STR(MDP_BGR_888);
    formats[MDP_Y_CBCR_H2V2_VENUS] = STR(MDP_Y_CBCR_H2V2_VENUS);
    formats[MDP_BGRX_8888] = STR(MDP_BGRX_8888);
    formats[MDP_RGBA_8888_TILE] = STR(MDP_RGBA_8888_TILE);
    formats[MDP_ARGB_8888_TILE] = STR(MDP_ARGB_8888_TILE);
    formats[MDP_ABGR_8888_TILE] = STR(MDP_ABGR_8888_TILE);
    formats[MDP_BGRA_8888_TILE] = STR(MDP_BGRA_8888_TILE);
    formats[MDP_RGBX_8888_TILE] = STR(MDP_RGBX_8888_TILE);
    formats[MDP_XRGB_8888_TILE] = STR(MDP_XRGB_8888_TILE);
    formats[MDP_XBGR_8888_TILE] = STR(MDP_XBGR_8888_TILE);
    formats[MDP_BGRX_8888_TILE] = STR(MDP_BGRX_8888_TILE);
    formats[MDP_RGB_565_TILE] = STR(MDP_RGB_565_TILE);
    formats[MDP_IMGTYPE_LIMIT] = STR(MDP_IMGTYPE_LIMIT);

    if(format < 0 || format >= MDP_IMGTYPE_LIMIT) {
        ALOGE("%s wrong fmt %d", __FUNCTION__, format);
        return "Unsupported format";
    }
    if(formats[format] == 0) {
        ALOGE("%s: table missing format %d from header", __FUNCTION__, format);
        return "";
    }
    return formats[format];
}

inline void Whf::dump() const {
    ALOGE("== Dump WHF w=%d h=%d f=%d s=%d start/end ==",
            w, h, format, size);
}

inline void Dim::dump() const {
    ALOGE("== Dump Dim x=%d y=%d w=%d h=%d start/end ==", x, y, w, h);
}

template <class Type>
void swapWidthHeight(Type& width, Type& height) {
    Type tmp = width;
    width = height;
    height = tmp;
}

inline void ScreenInfo::dump(const char* const s) const {
    ALOGE("== Dump %s ScreenInfo w=%d h=%d"
            " bpp=%d stride=%d start/end ==",
            s, mFBWidth, mFBHeight, mFBbpp, mFBystride);
}

inline bool openDev(OvFD& fd, int fbnum,
    const char* const devpath, int flags) {
    return overlay::open(fd, fbnum, devpath, flags);
}

template <class T>
inline void even_ceil(T& value) {
    if(value & 1)
        value++;
}

template <class T>
inline void even_floor(T& value) {
    if(value & 1)
        value--;
}

/* Prerotation adjusts crop co-ordinates to the new transformed values within
 * destination buffer. This is necessary only when the entire buffer is rotated
 * irrespective of crop (A-family). If only the crop portion of the buffer is
 * rotated into a destination buffer matching the size of crop, we don't need to
 * use this helper (B-family).
 * @Deprecated as of now, retained for the case where a full buffer needs
 * transform and also as a reference.
 */
void preRotateSource(const eTransform& tr, Whf& whf, Dim& srcCrop);
void getDump(char *buf, size_t len, const char *prefix, const mdp_overlay& ov);
void getDump(char *buf, size_t len, const char *prefix, const msmfb_img& ov);
void getDump(char *buf, size_t len, const char *prefix, const mdp_rect& ov);
void getDump(char *buf, size_t len, const char *prefix,
        const msmfb_overlay_data& ov);
void getDump(char *buf, size_t len, const char *prefix, const msmfb_data& ov);
void getDump(char *buf, size_t len, const char *prefix,
        const msm_rotator_img_info& ov);
void getDump(char *buf, size_t len, const char *prefix,
        const msm_rotator_data_info& ov);

} // namespace utils ends

//--------------------Class Res stuff (namespace overlay only) -----------

class Res {
public:
    // /dev/graphics/fb%u
    static const char* const fbPath;
    // /dev/msm_rotator
    static const char* const rotPath;
};


//--------------------Class OvFD stuff (namespace overlay only) -----------

/*
* Holds one FD
* Dtor will NOT close the underlying FD.
* That enables us to copy that object around
* */
class OvFD {
public:
    /* Ctor */
    explicit OvFD();

    /* dtor will NOT close the underlying FD */
    ~OvFD();

    /* Open fd using the path given by dev.
     * return false in failure */
    bool open(const char* const dev,
            int flags = O_RDWR);

    /* populate path */
    void setPath(const char* const dev);

    /* Close fd if we have a valid fd. */
    bool close();

    /* returns underlying fd.*/
    int getFD() const;

    /* returns true if fd is valid */
    bool valid() const;

    /* like operator= */
    void copy(int fd);

    /* dump the state of the instance */
    void dump() const;
private:
    /* helper enum for determine valid/invalid fd */
    enum { INVAL = -1 };

    /* actual os fd */
    int mFD;

    /* path, for debugging */
    char mPath[utils::MAX_PATH_LEN];
};

//-------------------Inlines--------------------------

inline bool open(OvFD& fd, uint32_t fbnum, const char* const dev, int flags)
{
    char dev_name[64] = {0};
    snprintf(dev_name, sizeof(dev_name), dev, fbnum);
    return fd.open(dev_name, flags);
}

inline OvFD::OvFD() : mFD (INVAL) {
    mPath[0] = 0;
}

inline OvFD::~OvFD() {
    //no op since copy() can be used to share fd, in 3d cases.
}

inline bool OvFD::open(const char* const dev, int flags)
{
    mFD = ::open(dev, flags, 0);
    if (mFD < 0) {
        // FIXME errno, strerror in bionic?
        ALOGE("Cant open device %s err=%d", dev, errno);
        return false;
    }
    setPath(dev);
    return true;
}

inline void OvFD::setPath(const char* const dev)
{
    ::strlcpy(mPath, dev, sizeof(mPath));
}

inline bool OvFD::close()
{
    int ret = 0;
    if(valid()) {
        ret = ::close(mFD);
        mFD = INVAL;
    }
    return (ret == 0);
}

inline bool OvFD::valid() const
{
    return (mFD != INVAL);
}

inline int OvFD::getFD() const { return mFD; }

inline void OvFD::copy(int fd) {
    mFD = fd;
}

inline void OvFD::dump() const
{
    ALOGE("== Dump OvFD fd=%d path=%s start/end ==",
            mFD, mPath);
}

//--------------- class OvFD stuff ends ---------------------

} // overlay


#endif // OVERLAY_UTILS_H
