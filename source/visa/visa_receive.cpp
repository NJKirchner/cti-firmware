#include "visa/visa_core.h"
#include "display/hub75.h"
#include "display/ws2811.h"

namespace CTI::Visa {

void Visa::MainLoop() {
    char input[64];
    unsigned idleReads = 0;

    while (true) {
        Hub75::service(&_parser);
        WS2811::service(&_parser);
        // Return as soon as any bytes arrive; never wait for the batch to fill.
        int count = gPlatform.IO.ReadStdinTimeout(input, sizeof(input), 20000);

        if (count < 0) {
            _parser.enqueueError(-200, "SCPI input read failed");
        } else if (count == 0) {
            gPlatform.IO.StatusLED(false, Comms);
            if (_parser.hasPendingInput() && ++idleReads >= 100) {
                _parser.enqueueError(-161, "Incomplete SCPI message timed out after 2 seconds");
                _parser.reset();
                idleReads = 0;
            }
        } else {
            idleReads = 0;
            if (_parser.bufferInput(input, count) != count) {
                _parser.enqueueError(-363, "SCPI input buffer overflow");
                while (true) {
                    int val = gPlatform.IO.FGetCtimeout(20000);
                    if (val == -1 || val == '\n') {
                        _parser.reset();
                        break;
                    }
                }
            }
        }
    }
}

}
