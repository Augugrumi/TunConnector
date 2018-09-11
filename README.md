# TunConnector
Code to create a Tun tunnel that both serve as a server and as a client
---
Usage
```bash
   $ sudo ./TunConnector [OPTIONS]"
```
with the following options
 - --serverinterface   -p X    To set the name if the interface used by the server component
 - --clientinterface   -n X    To set the name if the interface used by the client component
 - --serverip          -s X    To set the ip on the server interface
 - --clientip          -c X    To set the ip on the client interface
 - --outport           -o X    To set the port used by the client component. Default 55555
 - --inport            -i X    To set the port used by the server component. Default 55556
 - --remoteip          -r X    To set the ip of the remote server to connect to
 - --mode              -m [client|server|both] To set run mode. Default 'both'
 - --help              -h      Show this message
