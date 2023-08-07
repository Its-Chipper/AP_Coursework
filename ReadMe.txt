========================================================================
    CONSOLE APPLICATION : Pool Game Project Overview
========================================================================

AppWizard has created this Pool Game application for you.

This file contains a summary of what you will find in each of the files that
make up your Pool Game application.


Pool Game.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

Pool Game.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Pool Game.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

To run the program open the threaded server solution file in visual studio found in the threaded server folder. Run this file once it has opened, once it has opened to run on the same machine, please open the poolGame solution file in visual studio. Then run this and input a name for your player, when it comes up with “HostName/IP:” please input ‘localhost’. And when it comes up with “Port:” please input the first port that was shown when you ran the server for the first user as it will be a master user. After that you can user the sockets in any order as they are treated the same. To move the cue use the arrow keys and press enter to shoot the stone. If you wish to change the number of clients please change the MAX_NUM_CLIENTS variable in the threaded server file.
