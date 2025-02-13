//
// Created by borz7zy on 13.2.25..
//

#include "event_processing.h"

#include "../utils/logger.hpp"
#include "../utils/my_thread.h"

#include <string>
#include <vector>
#include <mutex>

// ===================[ PRIVATE METHODS ]===================

void EventProcessing::processing_event(const eventInfo &event){
#ifdef DEBUG
    Logger::Get()->Log("Обработка события №%d", event.event_id);
#endif
}

// ===================[ PUBLIC  METHODS ]===================

void EventProcessing::put_event(const eventInfo &event)
{
    events.push_back(event);
}

void EventProcessing::start_processing()
{
    while(!MainShared::Get()->g_shutdownRequested){
//        std::cout<< "test\n";
    }
}