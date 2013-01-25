/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <glib.h>

#include <transfer/gfal_transfer_internal.h>
#include <transfer/gfal_transfer_types_internal.h>

#include <ctime>


GQuark GFAL_EVENT_PREPARE_ENTER = g_quark_from_static_string("PREPARE:ENTER");
GQuark GFAL_EVENT_PREPARE_EXIT = g_quark_from_static_string("PREPARE:EXIT");
GQuark GFAL_EVENT_TRANSFER_ENTER = g_quark_from_static_string("TRANSFER:ENTER");
GQuark GFAL_EVENT_TRANSFER_EXIT = g_quark_from_static_string("TRANSFER:EXIT");
GQuark GFAL_EVENT_CLOSE_ENTER = g_quark_from_static_string("CLOSE:ENTER");
GQuark GFAL_EVENT_CLOSE_EXIT = g_quark_from_static_string("CLOSE:EXIT");
GQuark GFAL_EVENT_CHECKSUM_ENTER = g_quark_from_static_string("CHECKSUM:ENTER");
GQuark GFAL_EVENT_CHECKSUM_EXIT = g_quark_from_static_string("CHECKSUM:EXIT");
GQuark GFAL_EVENT_CANCEL_ENTER = g_quark_from_static_string("CANCEL:ENTER");
GQuark GFAL_EVENT_CANCEL_EXIT = g_quark_from_static_string("CANCEL:EXIT");


int plugin_trigger_event(gfalt_params_t params, GQuark domain,
                         gfal_event_side_t side, GQuark stage,
                         const char* fmt, ...) {
  if (params->event_callback) {
    struct _gfalt_event event;
    char buffer[512] = {0};

    event.domain = domain;
    event.side   = side;
    event.stage  = stage;
    event.timestamp = std::time(NULL);

    va_list msg_args;
    va_start(msg_args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, msg_args);
    va_end(msg_args);
    event.description = buffer;

    params->event_callback(&event, params->user_data);
  }
  return 0;
}
