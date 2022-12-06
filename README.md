# tcp-server-client-ping-pong-timing-win
C++ socket ping-pong timing

The base source code belongs to:
- https://learn.microsoft.com/en-us/windows/win32/winsock/complete-server-code
- https://learn.microsoft.com/en-us/windows/win32/winsock/complete-client-code

To use on Windows:
- build
- execute: (for IP = 127.0.0.1; port = 9999; ping-pongs = 10000)
  - `server.exe 9999`
  - `client.exe 127.0.0.1 9999 10000`

Note: This is for only Local usage.
