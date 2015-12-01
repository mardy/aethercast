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

#include "networkdevice.h"
#include "wfddeviceinfo.h"

namespace mcs {
std::string NetworkDevice::StateToStr(NetworkDeviceState state) {
    switch (state) {
    case kIdle:
        return "idle";
    case kFailure:
        return "failure";
    case kAssociation:
        return "connecting";
    case kConnected:
        return "connected";
    case kDisconnected:
        return "disconnected";
    default:
        break;
    }

    return "unknown";
}

NetworkDevice::NetworkDevice() :
    state_(kIdle),
    config_methods_(0),
    role_(kUndecided) {
}

NetworkDevice::~NetworkDevice() {
}

std::string NetworkDevice::Address() const {
    return address_;
}

std::string NetworkDevice::Name() const {
    return name_;
}

NetworkDeviceState NetworkDevice::State() const {
    return state_;
}

std::string NetworkDevice::StateAsString() const {
    return StateToStr(state_);
}

WfdDeviceInfo NetworkDevice::DeviceInfo() const {
    return wfd_device_info_;
}

int NetworkDevice::ConfigMethods() const {
    return config_methods_;
}

NetworkDeviceRole NetworkDevice::Role() const {
    return role_;
}

void NetworkDevice::SetAddress(const std::string &address) {
    address_ = address;
}

void NetworkDevice::SetName(const std::string &name) {
    name_ = name;
}

void NetworkDevice::SetState(NetworkDeviceState state) {
    state_ = state;
}

void NetworkDevice::SetWfdDeviceInfo(const WfdDeviceInfo &wfd_device_info) {
    wfd_device_info_ = wfd_device_info;
}

void NetworkDevice::SetConfigMethods(int config_methods) {
    config_methods_ = config_methods;
}

void NetworkDevice::SetRole(NetworkDeviceRole role) {
    role_ = role;
}
} // namespace mcs