/*
 * Copyright (C) 2020 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include <functional>
#include <mutex>
#include <carma_wm_ctrl/WMBroadcaster.h>
#include <carma_wm_ctrl/MapConformer.h>
#include <lanelet2_extension/utility/message_conversion.h>
#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <type_traits>
// TODO add ros logging

// TODO consider applying the lanelet2_extension for overwriting lanelet centerlines

// TODO remove includes below here
#include <carma_wm_ctrl/ROSTimerFactory.h>
#include <carma_wm_ctrl/GeofenceScheduler.h>
#include <carma_wm/lanelet/DigitalSpeedLimit.h>
#include <carma_wm/lanelet/PassingControlLine.h>
#include <carma_wm/lanelet/RegionAccessRule.h>
#include <carma_wm/lanelet/CarmaUSTrafficRules.h>


namespace carma_wm_ctrl  // TODO should this be carma_wm or carma_wm_ctrl?
{

  using std::placeholders::_1;

  // NEW PLAN 1/17/2020
  // 1. Clean up existing header only classes. 
  //    -- Completed 
  // 2. Develop logic for generating regulatory elements from existing map data
  //    -- Completed
  // 3. Develop place holder functions for geofence data
  //    -- Completed
  // 3.5. Implement Node
  // 4. Develop unit tests for this package
  // 5. Update carma_wm to use new traffic rules
  // 6. Clean up TODOs

WMBroadcaster::WMBroadcaster(PublishMapCallback map_pub, std::unique_ptr<TimerFactory> timer_factory) 
  : map_pub_(map_pub), scheduler_(std::move(timer_factory)) {  

  scheduler_.onGeofenceActive(std::bind(&WMBroadcaster::addGeofence, this, _1));
  scheduler_.onGeofenceInactive(std::bind(&WMBroadcaster::removeGeofence, this, _1));

};

void WMBroadcaster::baseMapCallback(const autoware_lanelet2_msgs::MapBinConstPtr& map_msg) {
  std::lock_guard<std::mutex> guard(map_mutex_);
  
  static bool firstCall = true;
  firstCall = false;
  // This function should generally only ever be called one time so log a warning if it occurs multiple times
  if (firstCall) {
    ROS_INFO("WMBroadcaster::baseMapCallback called for first time with new map message");
  } else {
    ROS_WARN("WMBroadcaster::baseMapCallback called multiple times in the same node");
  }

  lanelet::LaneletMapPtr new_map(new lanelet::LaneletMap);

  lanelet::utils::conversion::fromBinMsg(*map_msg, new_map);

  base_map_ = new_map; // Store map
  
  lanelet::MapConformer::ensureCompliance(base_map_); // Update map to ensure it complies with expectations

  // Publish map
  autoware_lanelet2_msgs::MapBin compliant_map_msg;
  lanelet::utils::conversion::toBinMsg(base_map_, &compliant_map_msg);
  map_pub_(compliant_map_msg);
};

void WMBroadcaster::geofenceCallback(/*TODO add message type here once defined*/){
  std::lock_guard<std::mutex> guard(map_mutex_);
  Geofence gf;
  scheduler_.addGeofence(gf); // Add the geofence to the schedule
  ROS_INFO_STREAM("New geofence message received by WMBroadcaster with id" << gf.id_);
};

void WMBroadcaster::addGeofence(const Geofence& gf){
  std::lock_guard<std::mutex> guard(map_mutex_);
  ROS_INFO_STREAM("Adding active geofence to the map with geofence id: " << gf.id_);
  // TODO Add implementation for adding a geofence
};

void WMBroadcaster::removeGeofence(const Geofence& gf){
  std::lock_guard<std::mutex> guard(map_mutex_);
  ROS_INFO_STREAM("Removing inactive geofence to the map with geofence id: " << gf.id_);
  // TODO Add implementation for removing a geofence
};

}  // namespace carma_wm_ctrl