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

#ifndef W11TNG_RFKILL_MANAGER_H_
#define W11TNG_RFKILL_MANAGER_H_

#include "ac/non_copyable.h"
#include "ac/glib_wrapper.h"

#include <memory>

namespace w11tng {
class RfkillManager {
public:
    typedef std::shared_ptr<RfkillManager> Ptr;

    enum class Type : std::uint32_t {
        kAll = 0,
        kWLAN,
        kBluetooth,
        kUWB,
        kWWAN,
        kGPS,
        kFM,
    };

    class Delegate : public ac::NonCopyable {
    public:
        virtual void OnRfkillChanged(const Type &type) = 0;
    };

    virtual ~RfkillManager();

    void SetDelegate(const std::weak_ptr<Delegate> &delegate);
    void ResetDelegate();

    virtual bool IsBlocked(const Type &type) const = 0;

protected:
    std::weak_ptr<Delegate> delegate_;

};
} // namespace w11tng

#endif
