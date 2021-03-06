/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AC_VIDEO_BASEENCODER_H_
#define AC_VIDEO_BASEENCODER_H_

#include <memory>

#include "ac/non_copyable.h"

#include "ac/common/executable.h"

#include "ac/video/buffer.h"

namespace ac {
namespace video {

class BaseEncoder : public common::Executable {
public:
    typedef std::shared_ptr<BaseEncoder> Ptr;

    class Config {
    public:
        Config() :
            width(0),
            height(0),
            bitrate(0),
            framerate(0),
            profile(0),
            level(0),
            profile_idc(0),
            level_idc(0),
            constraint_set(0),
            i_frame_interval(0) {
        }

        bool operator==(const Config& other) const {
            return width == other.width &&
                    height == other.height &&
                    bitrate == other.bitrate &&
                    framerate == other.framerate &&
                    profile == other.profile &&
                    level == other.level &&
                    profile_idc == other.profile_idc &&
                    level_idc == other.level_idc &&
                    constraint_set == other.constraint_set &&
                    i_frame_interval == other.i_frame_interval &&
                    intra_refresh_mode == other.intra_refresh_mode;
        }

        unsigned int width;
        unsigned int height;
        unsigned int bitrate;
        int framerate;

        // H.264 specifics
        unsigned int profile;
        unsigned int level;
        unsigned int profile_idc;
        unsigned int level_idc;
        unsigned int constraint_set;
        unsigned int i_frame_interval;
        unsigned int intra_refresh_mode;
    };

    class Delegate : public ac::NonCopyable {
    public:
        virtual void OnBufferAvailable(const ac::video::Buffer::Ptr &buffer) = 0;
        virtual void OnBufferWithCodecConfig(const ac::video::Buffer::Ptr &buffer) = 0;
        virtual void OnBufferReturned() { }
    };

    virtual ~BaseEncoder() { }

    void SetDelegate(const std::weak_ptr<Delegate> &delegate);

    virtual BaseEncoder::Config DefaultConfiguration() = 0;

    virtual bool Configure(const Config &config) = 0;

    virtual void QueueBuffer(const ac::video::Buffer::Ptr &buffer) = 0;

    virtual Config Configuration() const = 0;

    virtual bool Running() const = 0;

    virtual void SendIDRFrame() = 0;

protected:
    BaseEncoder() = default;

protected:
    std::weak_ptr<Delegate> delegate_;
};

} // namespace video
} // namespace ac

#endif
