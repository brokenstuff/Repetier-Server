= Repetier-Server =

== Objective ==

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

== Status ==

It is at the very beginning. It is not fully functional now and only compiles on a mac.
Makefiles for Linux and Windows will follow. It is only added to github for revision
control, not to be used in public.

== Licence ==

My part of the work is distributed under the Apache V2 licence.
The sources contain software from 3rd parties with different licences.

TODO: Add list of 3rd party software

== API ==

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

=== List configured printer ===

Command: /printer/list
Return: Array of printer. Each entry object has the following  content:
{name:"<Official name from config>",slug:"<short name for addressing printer requests>",
online:<0 or 1>,job:"<none|paused|printing|waitstart>"}
Possible error: None

Jobs that are send, but didn't get triggert to start have the state waitstart. This
allows it to send more then one job in a row. After completing a job, the state goes
to waitstart waiting for a user to tell that the new print can start.


=== Sending printer commands ===

Command: /printer/send/<slugname>?cmd=line
Return: Nothing
Possible errors:
 "Printer offline"
 "Unknown printer"

=== Get printer responses ===

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

=== Handling jobs ===

Jobs can be very long. During storage handling the send data resides in ram, so
on small computer like the Raspberry Pi it would be a problem to send large files.
To help with that fact, the job can send split into several chunks which are appended
on the disk. Jobs are stored in a job directory of the printer to release the memory.
This also allows a reprint of a job when the computer got shut down. The job will still
be there and can be startet.

Command: /printer/job/<slugname>?a=<start|pause|create|append|upload>

=== Managing the server ===

Coming soon ...
