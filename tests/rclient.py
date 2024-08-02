#!/usr/bin/env python3

## @package rclient
#
#  @ingroup gateway_tests
#
#  @author  Wieslaw Grygo
#
#  @date    05 Jul 2017
#
#  @brief   Remote client written in python
#
#  $Id: //prod/cortex/c/modules/common/common-6.4/src/comms/gateway/tests/rclient.py#4 $
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
# @brief    Add encoded length to message
#
# @param    [in] message Message to send
#
# @return   message
#
def add_length(message):
    return struct.pack('!H', len(message)) + message

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
# @brief    Generate output message using input mesage and template.
#
#           Format of template. The '[]' is replaced with '['.
#           Espresions like '[x:y]' are replaced with proper part of input message.
#           Where x and y can be replaced with a number or nothing.
#
# @param    [in] txt Input message.
# @param    [in] msg Template for a message.
#
# @return   Output message
#
def make_message(txt, msg):
    out=''
    l=0
    p=0
    p=msg.find('[',l)
    while p!=-1:
        #print 'p', p
        out += msg[l:p]
        q = msg.find(']',p+1)
        #print 'q', q
        if p+1==q:
            out += '['
            l=q+1
        elif q!=-1:
            s=msg.find(':',p+1,q)
            #print 's', s
            if s==-1:
                out += txt[int(msg[p+1:q])]
            elif s>p+1:
                if q>s+1:
                    out += txt[int(msg[p+1:s]):int(msg[s+1:q])]
                else:
                    out += txt[int(msg[p+1:s]):]
            else:
                if q>s+1:
                    out += txt[:int(msg[s+1:q])]
                else:
                    out += txt[:]
            l=q+1
        else:
            return
        p=msg.find('[',l)
    out += msg[l:]
    return out

##
# @brief    Read requests and send responses
# 
# @param    [in] addr Address of server
# @param    [in] msg Output message template
# @param    [in] sleep Time to sleep before sending response
#
# @return   None
#
def work(addr, msg, sleep):
    (err,sock) = connect_server(addr)
    if err==0:
        #sock.send(msg)
        #print "Send:", repr(msg) 
        buf = sock.recv(4096)        
        while len(buf):
            #print request
            print('Input', repr(buf))

            # wait before sending response
            if sleep:
                time.sleep(sleep)

            # output response
            out = make_message(buf[2:], msg)
            print('Output', repr(out))            
            sock.send( add_length(out) )

            # read next message
            buf = sock.recv(4096)
    else:
        print('Error on socket', errno.errorcode[err])
    sock.close()
        

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
    parser.add_option("-m", dest="message", default='',
        help="Template for output message. The '[]' is replaced with '['. "
             "Expresion '[x:y]' copy some data from imput string. The x and y could be a number or nothing.")
    parser.add_option("-w", type="float", dest="wait", default=0.0,
            metavar="N", help="Wait N seconds before sending response (default 0.0).")
    

    (options, args) = parser.parse_args()

    if not options.service:
        parser.error("Option -s is required! Use -? or --help to see usage information.")

    # resolve address and use first value
    addresses = socket.getaddrinfo(options.hostname, options.service, socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP, socket.AI_PASSIVE)
    (family, socktype, proto, canonname, sockaddr) = addresses[0]

    msg=options.message.decode('string_escape')
    
    work(sockaddr,msg,options.wait)

main()

# Some test of generation of output messages
#out=make_message("ALA", '[:]')
#out=make_message("OLA", 'x[1]x')
#out=make_message("OLA", '][1:]x')
#out=make_message("BASIA", '[:4]z')
#out=make_message("BASIA", '[]z')
#out=make_message("ZOSIA", 'G[1:4]a')

#out=make_message("123456789123456789aabb", '[3:5]__[13:]x')
#print 'Output', repr(out)
