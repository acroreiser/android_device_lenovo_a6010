/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (c) 2010-2012, The Linux Foundation. All rights reserved.
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "overlayRotator.h"
#include "overlayUtils.h"
#include "mdp_version.h"
#include "sync/sync.h"
#include "gr.h"

namespace ovutils = overlay::utils;

namespace overlay {

//============Rotator=========================

Rotator::Rotator() {
    char property[PROPERTY_VALUE_MAX];
    mRotCacheDisabled = false;
    if((property_get("debug.rotcache.disable", property, NULL) > 0) &&
       (!strncmp(property, "1", PROPERTY_VALUE_MAX ) ||
        (!strncasecmp(property,"true", PROPERTY_VALUE_MAX )))) {
        /* Used in debugging to turnoff rotator caching */
        mRotCacheDisabled = true;
    }
}

Rotator::~Rotator() {}

Rotator* Rotator::getRotator() {
    int type = getRotatorHwType();
    if(type == TYPE_MDP) {
        return new MdpRot(); //will do reset
    } else if(type == TYPE_MDSS) {
        return new MdssRot();
    } else {
        ALOGE("%s Unknown h/w type %d", __FUNCTION__, type);
        return NULL;
    }
}

int Rotator::getDownscaleFactor(const int& srcW, const int& srcH,
        const int& dstW, const int& dstH, const uint32_t& mdpFormat,
        const bool& isInterlaced) {
    if(getRotatorHwType() == TYPE_MDSS) {
        return MdssRot::getDownscaleFactor(srcW, srcH, dstW, dstH,
                mdpFormat, isInterlaced);
    }
    return MdpRot::getDownscaleFactor(srcW, srcH, dstW, dstH,
            mdpFormat, isInterlaced);
}

uint32_t Rotator::calcOutputBufSize(const utils::Whf& destWhf) {
    //dummy aligned w & h.
    int alW = 0, alH = 0;
    int halFormat = ovutils::getHALFormat(destWhf.format);
    //A call into gralloc/memalloc
    return getBufferSizeAndDimensions(
            destWhf.w, destWhf.h, halFormat, alW, alH);
}

int Rotator::getRotatorHwType() {
    int mdpVersion = qdutils::MDPVersion::getInstance().getMDPVersion();
    if (mdpVersion == qdutils::MDSS_V5)
        return TYPE_MDSS;
    return TYPE_MDP;
}

bool Rotator::isRotCached(int fd, uint32_t offset) const {
    if(mRotCacheDisabled or rotConfChanged() or rotDataChanged(fd,offset))
        return false;
    return true;
}

bool Rotator::rotDataChanged(int fd, uint32_t offset) const {
    /* fd and offset are the attributes of the current rotator input buffer.
     * At this instance, getSrcMemId() and getSrcOffset() return the
     * attributes of the previous rotator input buffer */
    if( (fd == getSrcMemId()) and (offset == getSrcOffset()) )
        return false;
    return true;
}

//============RotMem=========================

bool RotMem::close() {
    bool ret = true;
    if(valid()) {
        if(mem.close() == false) {
            ALOGE("%s error in closing rot mem", __FUNCTION__);
            ret = false;
        }
    }
    return ret;
}

RotMem::RotMem() : mCurrIndex(0) {
    utils::memset0(mRotOffset);
    for(int i = 0; i < ROT_NUM_BUFS; i++) {
        mRelFence[i] = -1;
    }
}

RotMem::~RotMem() {
    for(int i = 0; i < ROT_NUM_BUFS; i++) {
        ::close(mRelFence[i]);
        mRelFence[i] = -1;
    }
}

void RotMem::setCurrBufReleaseFd(const int& fence) {
    int ret = 0;

    if(mRelFence[mCurrIndex] >= 0) {
        //Wait for previous usage of this buffer to be over.
        //Can happen if rotation takes > vsync and a fast producer. i.e queue
        //happens in subsequent vsyncs either because content is 60fps or
        //because the producer is hasty sometimes.
        ret = sync_wait(mRelFence[mCurrIndex], 1000);
        if(ret < 0) {
            ALOGE("%s: sync_wait error!! error no = %d err str = %s",
                __FUNCTION__, errno, strerror(errno));
        }
        ::close(mRelFence[mCurrIndex]);
    }
    mRelFence[mCurrIndex] = fence;
}

void RotMem::setPrevBufReleaseFd(const int& fence) {
    uint32_t numRotBufs = mem.numBufs();
    uint32_t prevIndex = (mCurrIndex + numRotBufs - 1) % (numRotBufs);

    if(mRelFence[prevIndex] >= 0) {
        /* No need of any wait as nothing will be written into this
         * buffer by the rotator (this func is called when rotator is
         * in cache mode) */
        ::close(mRelFence[prevIndex]);
    }

    mRelFence[prevIndex] = fence;
}

//============RotMgr=========================
RotMgr * RotMgr::sRotMgr = NULL;

RotMgr* RotMgr::getInstance() {
    if(sRotMgr == NULL) {
        sRotMgr = new RotMgr();
    }
    return sRotMgr;
}

RotMgr::RotMgr() {
    for(int i = 0; i < MAX_ROT_SESS; i++) {
        mRot[i] = 0;
    }
    mUseCount = 0;
    mRotDevFd = -1;
}

RotMgr::~RotMgr() {
    clear();
}

void RotMgr::configBegin() {
    //Reset the number of objects used
    mUseCount = 0;
}

void RotMgr::configDone() {
    //Remove the top most unused objects. Videos come and go.
    for(int i = mUseCount; i < MAX_ROT_SESS; i++) {
        if(mRot[i]) {
            delete mRot[i];
            mRot[i] = 0;
        }
    }
}

Rotator* RotMgr::getNext() {
    //Return a rot object, creating one if necessary
    overlay::Rotator *rot = NULL;
    if(mUseCount >= MAX_ROT_SESS) {
        ALOGW("%s, MAX rotator sessions reached, request rejected", __func__);
    } else {
        if(mRot[mUseCount] == NULL)
            mRot[mUseCount] = overlay::Rotator::getRotator();
        rot = mRot[mUseCount++];
    }
    return rot;
}

void RotMgr::clear() {
    //Brute force obj destruction, helpful in suspend.
    for(int i = 0; i < MAX_ROT_SESS; i++) {
        if(mRot[i]) {
            delete mRot[i];
            mRot[i] = 0;
        }
    }
    mUseCount = 0;
    ::close(mRotDevFd);
    mRotDevFd = -1;
}

void RotMgr::getDump(char *buf, size_t len) {
    for(int i = 0; i < MAX_ROT_SESS; i++) {
        if(mRot[i]) {
            mRot[i]->getDump(buf, len);
        }
    }
    char str[4] = {'\0'};
    snprintf(str, 4, "\n");
    strlcat(buf, str, len);
}

int RotMgr::getRotDevFd() {
    if(mRotDevFd < 0 && Rotator::getRotatorHwType() == Rotator::TYPE_MDSS) {
        mRotDevFd = ::open("/dev/graphics/fb0", O_RDWR, 0);
        if(mRotDevFd < 0) {
            ALOGE("%s failed to open fb0", __FUNCTION__);
        }
    }
    return mRotDevFd;
}

}
