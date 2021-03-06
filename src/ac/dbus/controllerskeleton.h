/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef AC_CONTROLLERSKELETON_H_
#define AC_CONTROLLERSKELETON_H_

extern "C" {
// Ignore all warnings coming from the external headers as we don't
// control them and also don't want to get any warnings from them
// which will only pollute our build output.
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-w"
#include "aethercastinterface.h"
#pragma GCC diagnostic pop
}

#include <memory>
#include <unordered_map>

#include "ac/scoped_gobject.h"
#include "ac/forwardingcontroller.h"

#include "ac/dbus/networkdeviceskeleton.h"

namespace ac {
namespace dbus {
class ControllerSkeleton : public std::enable_shared_from_this<ControllerSkeleton>,
                           public ForwardingController,
                           public Controller::Delegate {
public:
    static constexpr const char *kBusName{"org.aethercast"};
    static constexpr const char *kManagerPath{"/org/aethercast"};
    static constexpr const char *kManagerIface{"org.aethercast.Manager"};

    static std::shared_ptr<ControllerSkeleton> Create(const std::shared_ptr<Controller> &controller);

    ~ControllerSkeleton();

    void OnStateChanged(NetworkDeviceState state) override;
    void OnDeviceFound(const NetworkDevice::Ptr &device) override;
    void OnDeviceLost(const NetworkDevice::Ptr &device) override;
    void OnDeviceChanged(const NetworkDevice::Ptr &peer) override;
    void OnChanged() override;

private:
    static void OnNameAcquired(GDBusConnection *connection, const gchar *name, gpointer user_data);

    static void OnEnabledChanged(GObject *source, GParamSpec *spec, gpointer user_data);

    static gboolean OnHandleScan(AethercastInterfaceManager *skeleton, GDBusMethodInvocation *invocation,
                                 gpointer user_data);
    static gboolean OnHandleDisconnectAll(AethercastInterfaceManager *skeleton, GDBusMethodInvocation *invocation,
                                          gpointer user_data);

    static gboolean OnSetProperty(GDBusConnection *connection, const gchar *sender,
                                  const gchar *object_path,const gchar *interface_name,
                                  const gchar *property_name, GVariant *variant,
                                  GError **error, gpointer user_data);

    ControllerSkeleton(const std::shared_ptr<Controller> &controller);
    std::shared_ptr<ControllerSkeleton> FinalizeConstruction();

    void SyncProperties();

    std::string GenerateDevicePath(const NetworkDevice::Ptr &device) const;

private:
    ScopedGObject<AethercastInterfaceManager> manager_obj_;
    SharedGObject<GDBusConnection> bus_connection_;
    guint bus_id_;
    ScopedGObject<GDBusObjectManagerServer> object_manager_;
    std::unordered_map<std::string,NetworkDeviceSkeleton::Ptr> devices_;
};
} // namespace dbus
} // namespace ac

#endif
