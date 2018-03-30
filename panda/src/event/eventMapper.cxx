/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eventMapper.cxx
 * @author Mitchell Stokes
 * @date 2018-3-30
 */

#include "config_event.h"
#include "eventMapper.h"
#include "eventHandler.h"
#include "throw_event.h"
#include "configVariableManager.h"
#include "configVariableCore.h"
#include "configVariableString.h"

TypeHandle EventMapper::_type_handle;
EventMapper *EventMapper::_global_ptr = nullptr;

const string g_item_prefix = "event-map-item-";
const size_t g_item_prefix_len = g_item_prefix.length();

/**
 * Initializes the event mapper and checks for config variables
 */
EventMapper::EventMapper::
EventMapper() {
    this->reload_config();
}

/**
 * Creates the global event mapper.
 */
void EventMapper::
make_global_ptr() {
  _global_ptr = new EventMapper();
}

void EventMapper::
throw_events(const Event *event, EventSet *events, string suffix) {
  event_cat->debug() << "throwing mapped events for input: " << event->get_name() << endl;

  for (const string event : *events) {
    throw_event(event+suffix);
  }
}

void EventMapper::
event_callback(const Event *event, void *data) {
    EventSet *events = (EventSet*)data;
    throw_events(event, events, "");
}

void EventMapper::
event_callback_up(const Event *event, void *data) {
    EventSet *events = (EventSet*)data;
    throw_events(event, events, "-up");
}

void EventMapper::
event_callback_down(const Event *event, void *data) {
    EventSet *events = (EventSet*)data;
    throw_events(event, events, "-down");
}
  

/**
 * Description: Clear mappings and rebuild them from config variables
 */
void EventMapper::
reload_config() {
  EventHandler *event_handler = EventHandler::get_global_event_handler();
  ConfigVariableManager *cvmgr = ConfigVariableManager::get_global_ptr();

  // Remove previous hooks
  for (const auto& map_pair : this->_event_map) {
    event_handler->remove_hook(
      map_pair.first,
      EventMapper::event_callback,
      (void*)&map_pair.second
    );
    event_handler->remove_hook(
      map_pair.first,
      EventMapper::event_callback_up,
      (void*)&map_pair.second
    );
    event_handler->remove_hook(
      map_pair.first,
      EventMapper::event_callback_down,
      (void*)&map_pair.second
    );
  }

  // Clear out all mappings
  this->_event_map.clear();

  // Build mappings from ConfigVariables
  for (size_t i = 0; i < cvmgr->get_num_variables(); ++i) {
    ConfigVariableCore *var = cvmgr->get_variable(i);
    string confname = var->get_name();

    if (confname.find(g_item_prefix) == 0) { 
      ConfigVariableString cvar = ConfigVariableString(confname);
      string outevent = confname.substr(g_item_prefix_len, confname.length());

      for (size_t j = 0; j < cvar.get_num_words(); ++j) {
        string inevent = cvar.get_word(j);
        auto search = this->_event_map.find(inevent);

        if (inevent == outevent) {
            // Prevent circular reference
            event_cat->warning()
              << "skipping circular reference mapping "
              << inevent << " to " << outevent
              << endl;
            continue;
        }

        if (search != this->_event_map.end()) {
          search->second.insert(outevent);
        }
        else {
          EventSet events;
          events.insert(outevent);
          this->_event_map[inevent] = events;
        }
      }
    }
  }

  event_cat->info() << "Event Map" << endl;
  for (const auto& map_pair : this->_event_map) {
    event_cat->info() << "Input: " << map_pair.first << endl;
    for (const auto& input : map_pair.second) {
      event_cat->info() << "  Output: " << input << endl;
    }
  }

  // Add hooks based on mappings
  for (const auto& map_pair : this->_event_map) {
    event_handler->add_hook(
      map_pair.first,
      EventMapper::event_callback,
      (void*)&map_pair.second
    );
    event_handler->add_hook(
      map_pair.first+"-up",
      EventMapper::event_callback_up,
      (void*)&map_pair.second
    );
    event_handler->add_hook(
      map_pair.first+"-down",
      EventMapper::event_callback_down,
      (void*)&map_pair.second
    );
  }
}

/**
 * Description: Return a list of events that will map to the given event.
 */
EventMapper::EventSet EventMapper::
get_inputs_for_event(const string event_name) {
    EventSet events;
    for (const auto& map_pair: this->_event_map) {
        if (map_pair.second.count(event_name) != 0) {
            events.insert(map_pair.first);
        }
    }

    return events;
}
