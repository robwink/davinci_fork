You can compile plugins within ImageJ using with either the Plugins->Compile 
and Run command or using the File->Compile and Run command in ImageJ's built 
in editor (Plugins->Edit).

The QT_Movie_Opener and QuickTime_Capture plugins (contained in 
QuickTime_Plugins.jar) are preinstalled as the File>Import>Using
QuickTime... and File>Import>Video commands.

With Java 1.3 and earlier, the QuitHandler plugin in the Utilities 
folder is used to process the "About" and "Quit" commands in the 
Apple menu and to open files dropped on ImageJ and files with
creator code "imgJ" that are double-clicked. With Java 1.4 and
later, the MacAdapter plugin in ij.jar is used.