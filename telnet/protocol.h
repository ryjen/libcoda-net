#ifndef ARG3_NET_TELNET_PROTOCOL_H_
#define ARG3_NET_TELNET_PROTOCOL_H_

#include "../socket.h"

namespace arg3
{
    namespace net
    {
        namespace telnet
        {
            constexpr static const socket::data_type SE  = 240;//    End of subnegotiation parameters.
            constexpr static const socket::data_type NOP     =            241;//    No operation.
            constexpr static const socket::data_type DataMark =           242;//    The data stream portion of a Synch.
            // This should always be accompanied
            // by a TCP Urgent notification.
            constexpr static const socket::data_type Break              = 243;//    NVT character BRK.
            constexpr static const socket::data_type InterruptProcess   = 244 ;//   The function IP.
            constexpr static const socket::data_type AbortOutput        = 245;//    The function AO.
            constexpr static const socket::data_type AreYouThere       = 246;//    The function AYT.
            constexpr static const socket::data_type EraseCharacter     = 247;//    The function EC.
            constexpr static const socket::data_type EraseLine          = 248;//    The function EL.
            constexpr static const socket::data_type GoAhead            = 249;//    The GA signal.
            constexpr static const socket::data_type SB                  = 250;//    Indicates that what follows is subnegotiation of the indicated option.
            constexpr static const socket::data_type WILL =  251;//    Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
            constexpr static const socket::data_type WONT = 252; //    Indicates the refusal to perform,  or continue performing, the indicated option.
            constexpr static const socket::data_type DO =    253;//    Indicates the request that the  other party perform, or confirmation that you are expecting  the other party to perform, the indicated option.
            constexpr static const socket::data_type  DONT = 254;//    Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party  to perform, the indicated option.
            constexpr static const socket::data_type  IAC            =     255;//    Data Byte 255.
        }
    }
}

#endif
