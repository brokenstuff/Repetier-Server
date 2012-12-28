# Repetier-Server

## Objective

The aim of this server is it, to offer a link between 3d printer and host software.
It allows the concurrent connection from different sources over the internet/intranet to
your printer. It stores jobs to run and sends them on your printer. Data is send as the
printer allows it. 

Why do you want this instead of direct communication?

You can put it on any old device you have or even on a Raspberry Pi. The server needs
very low resources and is very stable and reliable. It can run several printer at once.
You can move your printer anywhere you want, if you have a ethernet connection to the 
device running the server. And last, but not least, sending a job is now lightning fast.
It is a fire and forget thing. Once you have send the job from your host, it does its work
without the host. You can disconnect the host, close it and reconnect later. Nothing will
interrupt the printer. You can even spool new jobs while the last one is running, also
the server will ask you to clear the printer bed before starting the next job.

## Status

It is at the very beginning. It is not fully functional now and only compiles on a mac.
Makefiles for Linux and Windows will follow. It is only added to github for revision
control, not to be used in public.

## Licence

My part of the work is distributed under the Apache V2 licence.
The sources contain software from 3rd parties with different licences.

TODO: Add list of 3rd party software

### moFileReader - A simple .mo-File-Reader
Copyright (C) 2009 Domenico Gentner (scorcher24@gmail.com)
All rights reserved.    

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  3. The names of its contributors may not be used to endorse or promote 
     products derived from this software without specific prior written 
     permission.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### JSON Spirit

http://www.codeproject.com/Articles/20027/JSON-Spirit-A-C-JSON-Parser-Generator-Implemented
Copyright John W. Wilkinson 2007 - 2011
Distributed under the MIT License, see accompanying file LICENSE.txt

### Mongoose webserver

Copyright (c) 2004-2010 Sergey Lyubka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

### libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2010  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   
### Twitter Bootstrap

https://github.com/twitter/bootstrap

Copyright 2012 Twitter, Inc.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this work
except in compliance with the License. You may obtain a copy of the License in the
 LICENSE file, or at:

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the
License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
either express or implied. See the License for the specific language governing
permissions and limitations under the License.

### jQuery

http://jquery.com/
MIT Licence

### jQuery form

### Bootbox.js

Copyright (C) 2011-2012 by Nick Payne <nick@kurai.co.uk>
http://bootboxjs.com/index.html
MIT Licence http://bootboxjs.com/license.txt

## API 

The server uses communication over a webserver. With some simple calls you can
upload new jobs, send any command you want or manage the server.
The benefit of a simple API for all function is, that you can easily write new
frontends to control the printer. For the start I plan the communication with the host
first. When finished I will also add a webinterface, allowing to use the server from any
webserver including tablets and smartphones.

All commands are send as web requests to the configured port. If the path of the
uri starts with /printer/ it is handled as an API request. These requests answer
with a JSON formatet response. The first level always has this format:
{error:"<empty or error message>",data:<request dependend object>}
The JSON string is UTF-8 encoded.

### List configured printer

Command: /printer/list
Return: Array of printer. Each entry object has the following  content:
{name:"<Official name from config>",slug:"<short name for addressing printer requests>",
online:<0 or 1>,job:"<none|paused|printing|waitstart>"}
Possible error: None

Jobs that are send, but didn't get triggert to start have the state waitstart. This
allows it to send more then one job in a row. After completing a job, the state goes
to waitstart waiting for a user to tell that the new print can start.


### Sending printer commands

Command: /printer/send/<slugname>?cmd=line
Return: Nothing
Possible errors:
 "Printer offline"
 "Unknown printer"

### Get printer responses

If you want to monitor printer responses, you need every response. A difficult task
with a polling system. To overcome this problem, the server stores the last x responses.
With this command you get all responses following the given start response id.
If you need a filter, you can do with the filter parameter.

Filter is a binary value removing items where the bit value is set:
1 : Commands
2 : ACK responses like ok, wait, temperature
4 : Other responses
8 : Non maskable messages

Command: /printer/response/<slugname>?start=<after>&filter=<0|1|2>
Return: {lastid:<lastid>,lines:[{id:<respid>,time:"<time>",text:"<text>",type:<0-8>}],state:{state vars here}}

### Handling jobs

Jobs can be very long. During storage handling the send data resides in ram, so
on small computer like the Raspberry Pi it would be a problem to send large files.
To help with that fact, the job can send split into several chunks which are appended
on the disk. Jobs are stored in a job directory of the printer to release the memory.
This also allows a reprint of a job when the computer got shut down. The job will still
be there and can be startet.

Command: /printer/job/<slugname>?a=<list|start|pause|create|append|upload|remove>

Show all jobs: /printer/job/<slugname>?a=list
Upload job: /printer/job/<slugname>?a=upload&name=<name>&autostart=<0|1>&file=<file data as forum-multipart document>
Delete job:/printer/job/<slugname>?a=remove&id=<jobid>
Start job: /printer/job/<slugname>?a=start&id=<jobid>
Kill job: /printer/job/<slugname>?a=kill&id=<jobid>

### Model handling

You can store any number of models as gcode in the model database. From there you can
copy them any time into the print queue.

Show all models: /printer/model/<slugname>?a=list
Upload model: /printer/model/<slugname>?a=upload&name=<name>&autostart=<0|1>&file=<file data as forum-multipart document>
Delete model:/printer/model/<slugname>?a=remove&id=<jobid>
Start model: /printer/model/<slugname>?a=copy&id=<jobid>


### Managing the server

Coming soon ...
