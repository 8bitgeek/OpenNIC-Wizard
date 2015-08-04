##About The OpenNIC Project

"The OpenNIC Project" is an organization of dedicated volunteers who run an 
alternative DNS network. OpenNIC is owned and operated by the OpenNIC community. 

Membership is open to all who share an interest in keeping DNS free for all users. 

The goal of The OpenNIC Project is to provide you with quick and reliable DNS services and access
to domains not administered by ICANN.

More information about The OpenNIC Project can be found at http://www.opennicproject.org

OpenNIC provides resolution to all ICANN domains as well as OpenNIC's own TLDs:

.geek, .free, .bbs, .parody, .oss, .indy, .fur, .ing, .micro, .dyn and .gopher

##OpenNIC Wizard Features
  - OpenNIC Wizard service continually seeks DNS resolvers with best response times
  - No-pain approach to setting up your PC to work with The OpenNIC Project
  - OpenNIC Wizard Task tray applet for observing and optionally tweeking the configuration
  - To start using The OpenNIC Project today, use OpenNIC Wizard. 
  - Simply run the installer and you're instantly using OpenNIC resolve

##Screen Captures

http://www.8bitgeek.net/public/index.php?cmd=smarty&id=14_len

-------------------------------------

##Microsoft Windows XP,7,8,10 Installer
###OpenNic Wizard Version 0.3RC2: 
  https://github.com/8bitgeek/OpenNIC-Wizard/blob/master/OpenNIC%20Wizard%200.3.rc2.exe?raw=true

  - Run the installer and at the end, allow the installer to launch OpenNIC Wizard.

  - Wait for the initialization progress dialog to complete.

  - The OpenNIC Wizard Task Tray Icon should appear shortly.

  - Right click on OpenNIC Wizard task tray icon to access controls.

  - Left click on the OpenNIC Wizard task tray icon to see current resolver cache.

  - The OpenNIC Wizard Task Tray applet should appear from now on at each restart.

**NOTE:** OpenNIC Wizard is split into two parts, a so called 'client', and a so called 'server'.
      The 'server' performs the bulk of the functionality, while the 'client' performs the
      user interface function. On Windows(tm), OpenNICServer.exe runs as a service with elevated
      privilages, while OpenNIC.exe runs as a task-tray applet. The two communicate over a 
      client/server style protocol over a local-domain socket on port 19803.

**NOTE:** It has been observed on Windows 8.1 that after the OpenNIC Wizard Task Tray Applet
      is launched by the installer, that it may continue to say "Connecting...". 
      Restarting the system should resolve this issue. 
