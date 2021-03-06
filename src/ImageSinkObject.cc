/******************************************************************************
 * Copyright 2013, doubleTwist Corporation and Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#include "ImageSinkObject.h"

#include <qcc/Debug.h>

#define QCC_MODULE "ALLJOYN_AUDIO"

using namespace ajn;

namespace ajn {
namespace services {

ImageSinkObject::ImageSinkObject(BusAttachment* bus, const char* path, StreamObject* stream) :
    PortObject(bus, path, stream) {
    mDirection = DIRECTION_SINK;

    mNumCapabilities = 2;
    mCapabilities = new Capability[mNumCapabilities];

    int i = 0;

    /* Support for JPEG */
    mCapabilities[i  ].type = MIMETYPE_IMAGE_JPEG;
    mCapabilities[i  ].numParameters = 0;
    mCapabilities[i++].parameters = NULL;

    /* Support for PNG */
    mCapabilities[i  ].type = MIMETYPE_IMAGE_PNG;
    mCapabilities[i  ].numParameters = 0;
    mCapabilities[i++].parameters = NULL;

    /* Add Port.ImageSink interface */
    const InterfaceDescription* imageSinkIntf = bus->GetInterface(IMAGE_SINK_INTERFACE);
    assert(imageSinkIntf);
    AddInterface(*imageSinkIntf);

    const InterfaceDescription* imageSourceIntf = bus->GetInterface(IMAGE_SOURCE_INTERFACE);
    assert(imageSourceIntf);
    const InterfaceDescription::Member* imageDataMember = imageSourceIntf->GetMember("Data");
    assert(imageDataMember);
    QStatus status = bus->RegisterSignalHandler(this,
                                                static_cast<MessageReceiver::SignalHandler>(&ImageSinkObject::ImageDataSignalHandler),
                                                imageDataMember, NULL);
    if (status != ER_OK)
        QCC_LogError(status, ("Failed to register image signal handler"));
}

QStatus ImageSinkObject::Get(const char* ifcName, const char* propName, MsgArg& val) {
    QStatus status = ER_OK;

    QCC_DbgTrace(("GetProperty called for %s.%s", ifcName, propName));

    if (0 == strcmp(ifcName, IMAGE_SINK_INTERFACE)) {
        if (0 == strcmp(propName, "Version")) {
            val.Set("q", INTERFACES_VERSION);
        } else {
            status = ER_BUS_NO_SUCH_PROPERTY;
        }
    } else {
        status = ER_BUS_NO_SUCH_INTERFACE;
    }

    if (status == ER_BUS_NO_SUCH_INTERFACE || status == ER_BUS_NO_SUCH_PROPERTY)
        return PortObject::Get(ifcName, propName, val);

    return status;
}

void ImageSinkObject::DoConnect(const InterfaceDescription::Member* member, Message& msg) {
    QCC_DbgHLPrintf(("Configured type=[%s] numParameters=[%zu]", mConfiguration->type.c_str(), mConfiguration->numParameters));

    REPLY_OK();
}

void ImageSinkObject::ImageDataSignalHandler(const InterfaceDescription::Member* member,
                                             const char* sourcePath, Message& msg)
{
    if (mConfiguration == NULL) {
        QCC_LogError(ER_WARNING, ("Not configured, ignoring Image Data signal"));
        return;
    }

    size_t numArgs = 0;
    const MsgArg* args = NULL;
    msg->GetArgs(numArgs, args);

    if (numArgs != 1 || args[0].typeId != ALLJOYN_BYTE_ARRAY) {
        QCC_LogError(ER_BAD_ARG_COUNT, ("Received Image Data signal with invalid argument(s)"));
        return;
    }

    QCC_DbgHLPrintf(("Received %zu bytes of image data", args[0].v_scalarArray.numElements));
}

}
}
