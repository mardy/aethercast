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

#include "ac/common/executorpool.h"

namespace ac {
namespace common {

ExecutorPool::ExecutorPool(const ExecutorFactory::Ptr &factory, const size_t &size) :
    size_(size),
    running_(false),
    factory_(factory) {
}

ExecutorPool::~ExecutorPool() {
    Stop();
}

bool ExecutorPool::Add(const Executable::Ptr &executable) {
    if (items_.size() == size_ || running_)
        return false;

    auto executor = factory_->Create(executable);
    items_.emplace_back(Item{executable, executor});

    return true;
}

bool ExecutorPool::Start() {
    if (running_)
        return false;

    bool result = true;
    for (const auto &item : items_) {
        result &= item.executor->Start();
        if (!result)
            break;
    }

    // If we failed to start all we stop those we already
    // started to come back into a well known state.
    if (!result) {
        for (const auto &item : items_) {
            if (item.executor->Running())
                item.executor->Stop();
        }
    }

    running_ = result;

    return result;
}

bool ExecutorPool::Stop() {
    if (!running_)
        return false;

    bool result = true;
    for (auto &item : items_)
        result &= item.executor->Stop();

    if (result)
        running_ = false;

    return result;
}

bool ExecutorPool::Running() const {
    return running_;
}

} // namespace common
} // namespace ac
