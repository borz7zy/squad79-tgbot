//
// Created by borz7zy on 13.2.25..
//

#pragma once

#include "../main.hpp"
#include "../singleton.hpp"

#include "core_structs.h"

#include <vector>

class EventProcessing : public Singleton<EventProcessing> {
private:
    std::vector<eventInfo> events;

    void processing_event(const eventInfo &event);

public:
    EventProcessing() = default;
    virtual ~EventProcessing() = default;

    void put_event(const eventInfo &event);

    static void start_processing();
};
