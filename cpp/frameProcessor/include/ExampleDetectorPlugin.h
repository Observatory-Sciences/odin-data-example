/*
 * LATRDProcessPlugin.h
 *
 *  Created on: 26 Apr 2017
 *      Author: gnx91527
 */

#ifndef FRAMEPROCESSOR_INCLUDE_EXAMPLEDETECTORPLUGIN_H_
#define FRAMEPROCESSOR_INCLUDE_EXAMPLEDETECTORPLUGIN_H_

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

#include <stack>
#include "ExampleDetectorDefinitions.h"
#include "FrameProcessorPlugin.h"
#include "ClassLoader.h"

namespace FrameProcessor {

    class ExampleDetectorPlugin : public FrameProcessorPlugin {
    public:
        ExampleDetectorPlugin();

        virtual ~ExampleDetectorPlugin();

        void configure(OdinData::IpcMessage &config, OdinData::IpcMessage &reply);

        void requestConfiguration(OdinData::IpcMessage &reply);

        void status(OdinData::IpcMessage& status);

        void process_frame(boost::shared_ptr<Frame> frame);

        int get_version_major();
        int get_version_minor();
        int get_version_patch();
        std::string get_version_short();
        std::string get_version_long();

    private:

        /** Constant for this decoders name when publishing meta data */
        static const std::string META_NAME;

        /** Pointer to logger */
        LoggerPtr logger_;
    };

} /* namespace FrameProcessor */

#endif /* FRAMEPROCESSOR_INCLUDE_EXAMPLEDETECTORPLUGIN_H_ */
