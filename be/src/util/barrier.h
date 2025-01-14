// This file is made available under Elastic License 2.0.
// This file is based on code available under the Apache license here:
//   https://github.com/apache/incubator-doris/blob/master/be/src/util/barrier.h

// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#pragma once

#include <condition_variable>
#include <mutex>

#include "gutil/macros.h"
#include "storage/olap_define.h"

namespace starrocks {

// Implementation of pthread-style Barriers.
class Barrier {
public:
    // Initialize the barrier with the given initial count.
    explicit Barrier(int count) : _cond(), _count(count), _initial_count(count) { DCHECK_GT(count, 0); }

    ~Barrier() {}

    // wait until all threads have reached the barrier.
    // Once all threads have reached the barrier, the barrier is reset
    // to the initial count.
    void wait() {
        std::unique_lock l(_mutex);
        if (--_count == 0) {
            _count = _initial_count;
            _cycle_count++;
            _cond.notify_all();
            return;
        }

        int initial_cycle = _cycle_count;
        while (_cycle_count == initial_cycle) {
            _cond.wait(l);
        }
    }

private:
    std::mutex _mutex;
    std::condition_variable _cond;
    int _count;
    uint32_t _cycle_count = 0;
    const int _initial_count;
    DISALLOW_COPY_AND_ASSIGN(Barrier);
};

} // namespace starrocks
