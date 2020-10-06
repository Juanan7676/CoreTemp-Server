# CoreTemp-Server
An open-source Core Temp plugin that acts as a server.

#Installation
Copy the .dll in x64\Debug or \Debug into %CoreTempInstallationFolder%\plugins\CoreTempServer, and enable the plugin in Plugin Manager (F8)

#Usage
The plugin starts a server that listens on port 3480. To get info on your CPU's temperatures, send GET_INFO and the server will reply back.
Send EXIT to terminate the connection.
