/*
 * ExampleDetectorPluginLib.cpp
 *
 *  Created on: 11 Nov 2025
 *      Author: gnx91527
 */

#include "ExampleDetectorPlugin.h"
#include "ClassLoader.h"

namespace FrameProcessor
{
    /**
     * Registration of this decoder through the ClassLoader.  This macro
     * registers the class without needing to worry about name mangling
     */
    REGISTER(FrameProcessorPlugin, ExampleDetectorPlugin, "ExampleDetectorPlugin");

} // namespace FrameProcessor
