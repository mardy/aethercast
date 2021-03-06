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

#ifndef W11TNG_TESTING_P2PDEVICE_SKELETON_H_
#define W11TNG_TESTING_P2PDEVICE_SKELETON_H_

#include <ac/non_copyable.h>

#include "baseskeleton.h"

#include "w11tng/p2pdevicestub.h"

namespace w11tng {
namespace testing {

class P2PDeviceSkeleton : public std::enable_shared_from_this<P2PDeviceSkeleton>,
        public BaseSkeleton<WpaSupplicantInterfaceP2PDevice> {
public:
    typedef std::shared_ptr<P2PDeviceSkeleton> Ptr;

    class Delegate : public ac::NonCopyable {
    public:
        virtual void OnFind() = 0;
        virtual void OnStopFind() = 0;
    };

    static Ptr Create(const std::string &object_path);

    ~P2PDeviceSkeleton();

    void SetDelegate(const std::weak_ptr<Delegate> &delegate);

    void EmitDeviceFound(const std::string &path);
    void EmitDeviceLost(const std::string &path);
    void EmitGroupOwnerNegotiationSuccess(const std::string &path, const P2PDeviceStub::Status status,
                                          const P2PDeviceStub::Frequency freq, const P2PDeviceStub::FrequencyList &freqs,
                                          const P2PDeviceStub::WpsMethod wps_method);
    void EmitGroupOwnerNegotiationFailure(const std::string &path);
    void EmitGroupStarted(const std::string &group_path, const std::string &interface_path, const std::string &role);
    void EmitGroupFinished(const std::string &group_path, const std::string &interface_path);
    void EmitGroupRequest(const std::string &path, int dev_passwd_id);

private:
    P2PDeviceSkeleton(const std::string &object_path);
    Ptr FinalizeConstruction();

private:
    static gboolean OnHandleFind(WpaSupplicantInterfaceP2PDevice *device, GDBusMethodInvocation *invocation, GVariant *properties, gpointer user_data);
    static gboolean OnHandleStopFind(WpaSupplicantInterfaceP2PDevice *device, GDBusMethodInvocation *invocation, gpointer user_data);

private:
    std::weak_ptr<Delegate> delegate_;
};

} // namespace testing
} // namespace w11tng

#endif
