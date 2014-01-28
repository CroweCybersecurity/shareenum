shareenum
==

This is a tool that attempts to enumerate the permissions shares, directories, and files on Microsoft Windows systems.  Using an implementation of the Samba (http://www.samba.org/) libsmbclient libraries this tool makes a SMB connection to a host and recursively gathers information over the file and directory entities, compared to several other share enumeration tools that use RPC calls to gather similar information.  Both ways work, this was a bit easier to implement.

License
--
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/

Command Line Arguments
--
Usage: shareenum -o FILE TARGET

| Option | Description | Default |
| --- | --- | --- |
| TARGET | Full path or a file of paths to list the shares, files and directories within. Can be just an IP, a hostname, or even a full path inside a share, such as smb://COOL-DC.DOMAIN/NETLOGON. | |
|-o FILE | File to write results to in CSV format. | |
|-u USER | Username, otherwise go anonymous.  If using a domain, it should be in the format of DOMAIN\\user. | Null Session |
|-p PASS | Password, otherwise go anonymous.  This can be a NTLM has in the format LMHASH:NTLMHASH.  If so, we'll pass the hash. | Null Session |
|-r NUM  | How many levels into the share or directory should we go. Depending on the setting, this can pull a listing of every file and directory.  | No Recursion |
|-t NUM  | Seconds to wait before connection timeout. | 3 seconds |
|-s NUM  | In the event that the scan needs restarted, skip the first NUM lines in the input file.  All output will be appended to the output file so previous results will not be lost. | First Line |
|-d NUM  | How many seconds should we wait between each target. | No Delay |

> TARGET and FILE are required.

Installation
--

```sh
git clone https://github.com/emperorcow/shareenum.git shareenum
cd shareenum
make
```

This tool requires the following packages to run on Kali: 

* libsmbclient
* samba
* samba-common-bin

If you want to compile it, you'll also need:

* libsmbclient-dev
* samba-dev

