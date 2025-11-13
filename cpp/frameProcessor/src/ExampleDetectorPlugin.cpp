/*
 * ExampleDetectorPlugin.cpp
 *
 *  Created on: 11 Nov 2025
 *      Author: gnx91527
 */

#include "ExampleDetectorPlugin.h"
#include "DebugLevelLogger.h"
#include "version.h"

// Timeout for IDLE packets after which we assume we are in an acquisition
#define IDLE_PACKET_TIMEOUT_MS 1500

namespace FrameProcessor
{
ExampleDetectorPlugin::ExampleDetectorPlugin()
{
  // Setup logging for the class
  logger_ = Logger::getLogger("FP.ExampleDetectorPlugin");
  LOG4CXX_INFO(logger_, "LATRDProcessPlugin version " << this->get_version_long() << " loaded");
}

ExampleDetectorPlugin::~ExampleDetectorPlugin()
{
	// TODO Auto-generated destructor stub
}

/**
 * Set configuration options for the LATRD processing plugin.
 *
 * This sets up the process plugin according to the configuration IpcMessage
 * objects that are received. The options are searched for:
 * CONFIG_PROCESS - Calls the method processConfig
 *
 * \param[in] config - IpcMessage containing configuration data.
 * \param[out] reply - Response IpcMessage.
 */
void ExampleDetectorPlugin::configure(OdinData::IpcMessage& config, OdinData::IpcMessage& reply)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, config.encode());
}

void ExampleDetectorPlugin::requestConfiguration(OdinData::IpcMessage& reply)
{
//  reply.set_param(get_name() + "/" + LATRDProcessPlugin::CONFIG_MODE, this->mode_);
}

void ExampleDetectorPlugin::status(OdinData::IpcMessage& status)
{
//  status.set_param(get_name() + "/dropped_packets", dropped_packets);
//  status.set_param(get_name() + "/invalid_packets", invalid_packets);
}

void ExampleDetectorPlugin::process_frame(boost::shared_ptr<Frame> frame)
{
}

/**
  * Get the plugin major version number.
  *
  * \return major version number as an integer
  */
int LATRDProcessPlugin::get_version_major()
{
  return ODIN_DATA_VERSION_MAJOR;
}

/**
  * Get the plugin minor version number.
  *
  * \return minor version number as an integer
  */
int LATRDProcessPlugin::get_version_minor()
{
  return ODIN_DATA_VERSION_MINOR;
}

/**
  * Get the plugin patch version number.
  *
  * \return patch version number as an integer
  */
int LATRDProcessPlugin::get_version_patch()
{
  return ODIN_DATA_VERSION_PATCH;
}

/**
  * Get the plugin short version (e.g. x.y.z) string.
  *
  * \return short version as a string
  */
std::string LATRDProcessPlugin::get_version_short()
{
  return ODIN_DATA_VERSION_STR_SHORT;
}

/**
  * Get the plugin long version (e.g. x.y.z-qualifier) string.
  *
  * \return long version as a string
  */
std::string LATRDProcessPlugin::get_version_long()
{
  return ODIN_DATA_VERSION_STR;
}

} /* namespace FrameProcesser */
