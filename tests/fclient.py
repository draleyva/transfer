#!/usr/bin/env python3

## @package fclient
#
#  @ingroup gateway_tests
#
#  @author  Wieslaw Grygo
#
#  @date    27 May 2014
#
#  @brief   Fast GDI client written in python
#
#  $Id: //prod/cortex/c/modules/common/common-6.4/src/comms/gateway/tests/fclient.py#4 $
#
#  @copyright FIS Global
#

import socket
import struct
import errno
import time
import datetime
import sys

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
# @brief    Initialize data for speed mesure
#
# @return   None
#
def init_speed():
    global last
    last = datetime.datetime.now()

##
# @brief    Print speed for given number of operations
# 
# @param    [in] count Number of operations
# 
# @return   None
#
def print_speed(count):
    global last
    prev = last
    last = datetime.datetime.now()
    d = last-prev
    s = d.days*86400+d.seconds+d.microseconds*0.000001
    print(count/s)
    
##
# @brief    Connect to server
# 
# @param    [in] addr Address of server
# 
# @return   Returns (error, socket) 
#
def connect_server(addr):
    sock = None
    while True:
        if not sock:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            
        err=sock.connect_ex(addr)
        if err:
            if err==errno.EADDRNOTAVAIL:
                pass
            elif err==errno.EAGAIN:
                sock.close()
                sock = None
            else:
                return (err,None)
        else:
            return (err,sock)
        
    
##
# @brief    Send group of messages
# 
# @param    [in] addr Address of server
# @param    [in] count Number of operations
# @param    [in] msg Message to send
# @param    [in] sleep Time to sleep before printing speed and return
#
# @return   None
#
def send_group(addr, count, msg, sleep):
    for i in range(count):
        (err,sock) = connect_server(addr)
        if err==0:
            sock.send(msg)
            #print "Send:", repr(msg) 
            out=sock.recv(4)        
            if out!='\x00\x00\x00\x04':
                print("Recived 1:", repr(out))
            if True:
                out=sock.recv(4)
                if out!='\x00\x00\x03\xe8':
                    print("Recived 2:", repr(out))
        else:
            print('Error on socket', errno.errorcode[err])
        sock.close()
        
    if sleep:
        time.sleep(sleep)
    print_speed(count)

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
    
    parser.add_option("-n", type="int", dest="num", default=0,
        help="How many messages to send.")
    parser.add_option("-g", type="int", dest="group", default=1000,
        help="Print speed for group of messages.")
    parser.add_option("-w", type="float", dest="wait", default=0.0,
            metavar="N", help="Wait N seconds after sending group of messages (default 0.0).")
    

    (options, args) = parser.parse_args()

    if not options.service:
        parser.error("Option -s is required! Use -? or --help to see usage information.")

    # resolve address and use first value
    addresses = socket.getaddrinfo(options.hostname, options.service, socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP, socket.AI_PASSIVE)
    (family, socktype, proto, canonname, sockaddr) = addresses[0]
    
    msg = make_message(options.message, options.event)
    
    init_speed()
    if options.num:
        # main
        for g in range(options.num/options.group):
            send_group(sockaddr,options.group,msg,options.wait)
        # rest
        g = options.num % options.group
        if g:
            send_group(sockaddr,g,msg,options.wait)
    else:
        while True:
            send_group(sockaddr,options.group,msg,options.wait)

main()
