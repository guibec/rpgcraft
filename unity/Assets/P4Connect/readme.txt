P4Connect - Perforce integration for Unity 

Release Notes:
Last Update : June 26, 2015 by Norman Morse

Description
-----------

P4Connect is the simplest Perforce integration tool for Unity yet. P4Connect will:

	- Allow you to perform all basic version control operations directly from inside Unity
	- Show you the list of pending changes
	- Let you enter submission notes and diff your files against the Depot
	- Automatically check out / move / add / delete files when you make changes in the project view
    - Check out project settings and Solution files
	
	
Additionally, P4Connect supports:
	- SSL connections
	- Perforce streams
    - UnityVS
	- Assembla workspaces


Support
-------------
P4Connect is a fully supported product of Perforce Software.  To report an issue, please email support@perforce.com

There are also Community resources available to assist you with P4connect on the Perforce Forum.

Source Code
-------------
The source code for P4Connect has been open sourced using the BSD-2 license.

You can now improve P4Connect yourself by contributing to the project!
If you wish to contribute to the P4Connect project, please join the Perforce Workshop, and coordinate
updates with the Workshop Project Curator.  You can contact the curator by emailing support@perforce.com

You can access the workshop project for P4CONNECT here:
https://swarm.workshop.perforce.com/projects/perforce-software-p4connect

Look in the source code for build.txt which contains instructions for building.

Release Notes
-------------
Version 2.7 GA.9 Release
- Some fixes in debugging code to prevent spurious exceptions.
- Improved performance of Pending Changes Dialog with large change sets
- Reworked Configuration to allow saving settings with Perforce disabled
- Improved automatic initialization and connection after recompile.

Version 2.7 GA.8 Release
- Some fixes in debugging code to prevent duplicate element error and null exception
- Wrapped support file src/Assets/Editor/PackageIcons.cs differently to prevent warnings

Version 2.7 GA.7 Release
- Fixed issue with "Could not Submit" involving files and metas in mismatched states
- Added code path to handle "replaced" (deleted then added) files in the AssetPostProcessor
- GetAssetData now guarantees one result for each input
- Removed access to Perforce Changes Window (not ready for prime time yet)
- Limited queries to default change during RefreshChangeLists
- Removed stripignore from PerformOperation() since it was being called at inappropriate times.
- Fixed issues with CheckOut and Lock menu items
- Refactored Pending Changes dialog to simplify readability.

Version 2.7 GA.6 Release
- Fixed issue connecting to Unicode servers under OSX.
-  Cleaned up some minor compile warnings in the bridge code.
-  Current fix is a work-around pending a fix for the broken environ class under OSX.

Version 2.7 GA.5 Release
- Now validate that the default change only contains files which are under the project root.
- Fixed an issue with bogus asset paths being generated.
- Simplified Initialization sequence
- Modified logging and debug code.

Version 2.7 GA.4 Release
- Changed code to validate username against owner when creating the default change list.
- Added better conversion path from FileMetaData to FileSpec, using a constructor.
- Changed other code to use the new conversion code.
- Fixed occasional exception during submit.

Version 2.7 GA.3 Release
- Changed support file src/Assets/Editor/PackageIcons.cs to #ifdef Asset calls which are now obsolete in Unity 5
- Further fixed initialization problems with config dialog
- Fixed crash in GetLockStatus() on submit

Version 2.7 GA.2 Release
- Fixed initialization problem with config dialog
- Changed to attempt connection to Perforce automatically

Version 2.7 GA.1 Release
- Fixed crash problem with empty logpath
- Fixed refresh problem with empty change lists
- Commented out a bunch of Log calls

Version 2.7 GA Release
- Updated configuration dialog.  Improved connection error handling and speed.
- Added ChangeManager static class to handle queries about which files belong to which change
- Added Perforce Changes window, this doesn't do anything except display changes, their file contents and shelves.  This is intended to be improved later.
- Changes made to support automated builds by Electric Commander
- Many Bug Fixes

Version 2.7 beta3
- Rewrote the "Perforce Settings" dialog.  Removed many settings, provided buttons to get settings and write settings. Minor optimizations.
- The Perforce Settings will reinitialize if Perforce is Disabled then Enabled, and Perforce will automatically connect if the settings are functional.
- Added log4net library and logging entry points for common methods.   Some logging entries only log in the DEBUG build.
- Log4net can be configured to echo specific log levels to the console, while storing all levels in the logfile.
- Log4net also handles TRACE calls, and events / exceptions from inside the Bridge.
- Add NUnit to Debug Builds,  This works out of the box with the Unity Test Tools Asset Package. Nunit is in it's own project P4ConnectTest.
- Fixed several problems with mixed case and directory separators.
- Added Change Manager and passive dialog "Perforce Changes" in preparation to support multiple change lists and shelves.
- Fixed problems with "stale settings" and duplicate file paths.

Version 2.7 beta1
- This release fixes problems in the "Perforce Settings" dialog which could crash Unity.
- Fixed inconsistencies in the "Signature" which is reported by the Perforce Server
- Fixed an issue where P4API.NET would silently look for .P4Config and could find a different one than the one we selected.

Version 2.7 beta
- This release should work with either 64 or 32 bit versions of Unity3d 4.1+ and 5.X on both Windows and OSX
- Added support for both 32bit and 64bit versions of the p4bridge.  Both are included with each release, the appropriate DLL is selected at run time.
- Updated the P4API.NET source and C++ libraries to 2014.2 release. 
  There were many bug fixes and optimizations which these latest releases provide.
- Changed preferences to use a serialized editor asset "Assets/P4Connect/Editor/Config.Asset".  
  This allows multiple P4Connect instances to be run in different workspaces on a single machine.
- If the Config.Asset is missing P4Connect will fall back to using global EditorPreferences, which were used exclusively in earlier releases.
- Added Configuration option to allow writing of the Config.Asset, if this is unset, p4connect will not create a Config.Asset and will write 
  EditorPreferences like in previous releases.
- If a Configuration asset exists in the project, it will be used.  
  Remove any Configuration Asset and uncheck the "Write Config Asset" option to get the previous release behavior.
- Added Configuration options to enable logging and to set the logfile name and location.  This log is written to when the Bridge code calls LOG_DEBUG etc.
- Added a P4Ignore option and the ability to specify the P4Ignore name.   I never got this to work :( Needs more investigation.
- Added "additional ignore lines" configuration which are checked in addition to what is in .p4ignore (if enabled)
  Additional ignore lines can either be an exact match of the path to an asset, or if the line ends with a "/" it ignores all files and directories which are children.
  There is currently no "!" support or "wild card" support like you would expect in .gitignore or .p4ignore
- Added P4Connect.VersionInfo file which is updated as the release version changes, and is used in the Configuration Dialog and also in Perforce log entries.
- Made changes to the P4Connect P4API.NET library to support OSX.
    
Version 2.6
- Added P4CONFIG support - see http://www.perforce.com/perforce/doc.current/manuals/cmdref/P4CONFIG.html
- Fixed problem with "Unable to load DLL"

Version 2.5
- Perforce has purchased P4CONNECT from Artificial Heart and open sourced it on the Perforce workshop.
- Changed various strings and links to be more Perforce specific.
- First release from Asset Store after change of owner.

Version 2.3.4
- Added message when Check out operation fails
- Fixed exception when getting latest

Version 2.3.3
- Fixed slow-down when querying the state of many files at once. This would affect entering play mode for instance.
- Fixed exception thrown when P4Connect ends up deleting a file as a result of a Sync operation

Version 2.3.2
- Fixed issue when Serialization is set to Force Text. P4Connect now properly uses text only for the Unity-serialized files (like prefabs), not everything.

Version 2.3.1
- Fixed issue with Perforce Enable flag that wasn't sticking, and consequently triggering exceptions
- Fixed issue with periods in directory names

Version 2.3
- Better handling of binary and mixed mode serialization.
- Pending changes window now displays the type of the files on the server (text or binary)
- Allow adding files with special characters in them to the depot (%, #, @, *)

Version 2.2.1
- Fixed freeze when attempting to revert a directory from the project view.
- Don't update the project view icons when the editor enters play mode.

Version 2.2
- Added Refresh And Commit contextual menus in Project View. Refresh will make sure the status icons are up to date, and Commit
  will open up the Pending Changes Window and Select only the files of the items you have selected (folder, etc...)
- Minor bug fix to the Log messages.

Version 2.1
- Avoid initial sln exception on startup due to Unity trying to modify the .sln file
- It is no longer necessary to manually set the project root, P4Connect will grab it automatically
- P4Connect will tell you when you try to manually perform an operation but that operation causes no change

Version 2.0
- UnityP4 is now P4Connect
- OSX Support!!!
- Properly warn when assets contain @ or # in their names
- auto check out on edit (make it an option)
- Check out / Revert / Etc... menu items
- Settings per-user / per-project
- Icons in project view
- Pending changes window
    - Allow sorting by name/dir/type/opType
    - Revert unchanged
    - Space to se/deselect
    - Double-click opens
    - Right click reveal menu
    - Multi selection
    - Arrow keys
    - Launch Diff tool
    - Show selected count
- Revert on Move/Add file
- Combined file operations for better performance and console output

Version 1.4
- Improved responsiveness when reloading assemblies
- Perforce settings window no longer white while on first checking connection

Version 1.3
- Fixed some SSL support issues

Version 1.2
- Support for SSL Connections
- Improved .meta file change detection. Hopefully this means fewer missed deletion, move or renames
- Cosmetic changes to the UI

Installation
------------

Note: P4Connect will do its best to hold your hand through the configuration process, only allowing you to enter the next configuration setting if the previous one was validated.

- Start by importing the P4Connect Package from the Asset Store.
- Open the Perforce Settings dialog window from Edit->Perforce Settings.
  You should see it complain about invalid server URI.
- Enter the address of your server. Click Verify when you're done.
  If you entered a correct server address, the Verify button should turn into Valid! and P4Connect should have enabled the next section, your username and password.
- Enter a username and password (leave password empty if your server doesn't need one).
- Enter the name of your Perforce workspace
- (optionally) Enter a Project Root.
  Use this when your Unity Project is NOT at the root of your workspace. Typically you want to leave this blank.

P4Connect should now tell you that all the settings are correct and that Perforce Integration is active. You are ready to go!


Advanced Options
----------------

P4Connect allows you to customize its behavior further with a few advanced options:
- Display Status Icon: Show P4V-style icons on your files to indicate their version control status
- Show File Paths in Console: If unchecked, P4Connect will strip the directories from the file names when telling you what it's doing
- Gray out invalid menu options: If checked, P4Connect will first look at the current state of the file and gray out options that do not apply/would do nothing
- Diff Tool Executable: The tool that P4Connect will launch when diffing a file against the current head revision on the depot
- Ask before checkout on edit: When double clicking to open a file, tells P4Connect whether or not it should auto-checkout the file or ask you first
- Include Solution Files/Project Files: By default P4Connect will check out project and solution files from the Perforce depot. If you did not add them to source control (Unity usually recreates these for you), then you can disable those options.
- Integrate with UnityVS: If you use UnityVS to integrate Unity with Visual Studio, you can make sure that P4Connect checks out the UnityVS project files anytime UnityVS needs to regenerate these.
- Periodically Refresh Status: If turned on, P4Connect will periodically poll the server to update the status of files (so for instance it can show up to date blue icons)


Thanks for using P4Connect!
