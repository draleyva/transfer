#!/usr/bin/env python3

## @package sclient
#
#  @ingroup gateway_tests
#
#  @author  Wieslaw Grygo
#
#  @date    27 May 2014
#
#  @brief   Slow GDI client written in python
#
#  $Id: //prod/cortex/c/modules/common/common-6.4/src/comms/gateway/tests/sclient.py#4 $
#
#  @copyright FIS Global
#

import socket
import struct
import time

from optparse import OptionParser


##
# @brief    Make message
#
# @param    [in] message Message to send
# @param    [in] event Type of message
#
# @return   message
#
def make_message(message, event):
    return struct.pack('=II', len(message)+4, event+1000) + message

##
# @brief    Main function
# 
# @return   None
#
def main():
    # Parser of commandline parameters
    parser = OptionParser(add_help_option=False)
    
    parser.add_option("-?", "--help", action="help")

    parser.add_option("-s", dest="service",
        help="Service to connect")
    parser.add_option("-h", dest="hostname", default=None,
        metavar="HOST", help="Address of host")
    parser.add_option("-m", dest="message", default='Ok',
        help="What to send.")
    parser.add_option("-e", type="int", dest="event", default=2,
        help="What GDI event to send: 2 - message, 3 - reset, 4 - restart comms, 5 - close")
    
    parser.add_option("-w", type="float", dest="wait", default=1.0,
        metavar="W", help="Wait W seconds after walking through all connections (default 1.0).")
    parser.add_option("-c", type="int", dest="count", default=3,
        metavar="N", help="Use N connections (default 3).")

    (options, args) = parser.parse_args()
    
    if not options.service:
        parser.error("Option -s is required! Use -? or --help to see usage information.")

    # resolve address and use first value
    addresses = socket.getaddrinfo(options.hostname, options.service, socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP, socket.AI_PASSIVE)
    (family, socktype, proto, canonname, sockaddr) = addresses[0]

    msg = make_message(options.message, options.event)
        
    socks=[]
    mode=0
    nmsg=0
    while True:
        # update all socks
        if len(socks)<options.count:
            socks.insert(0,None)
            mode=0
        else:
            mode=(mode+1)%3
        m=mode
        for i in range(len(socks)):
            if m==0:
                #print 'Create', i
                #if socks[i]:
                #    socks[i].recv(1024)
                #    socks[i].close()                
                socks[i] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                socks[i].connect(sockaddr)
            elif m==1:
                print('Send', i)
                socks[i].send(msg)
            else:
                #print 'Destroy', i
                socks[i].recv(1024)
                socks[i].close()
            m=(m+1)%3
        #sock.send(msg)
        
        #print
        if options.wait>0:
            time.sleep(options.wait)

main()
