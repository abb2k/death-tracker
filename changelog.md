# Changelog

## v3.1.3
- im not very smart and did a bit of an oopsie so death tracker menu for level info layer

## v3.1.2
- fixed decimal popup from placing in incorrect positions
- small crash fixes
- fixed overlay label scroll bugs
- richtextarea is now using geode label! allowing for emotes and other stuff :D
- the \<size\> key works better now and doesnt make character weird looking
- added the \<spacing\> key to adjust character spacing
- added the \<font\> key for changing fonts mid text
- you can now input any character into the label title

## v3.1.1
- fixed issue in sections key where it would cut off a key for no reason
- fixed the donor button and convert save buttons making them clickable again

## v3.1.0
- added decimal percentages support + more death data such as timestamps for deaths and such
- added "Overlay Labels" to display information of your deaths in-level!! :D
- "reset as death" is now turned ON by default!
- added donor cosmetic features like the gradient and wave/shake text effects!
- fixed RichTextArea to support many new things such as the default GD color tags
- the \<color\> key now supports opacity
- added section related graphs
- improved session grouping accuracy
- now when having "quick save" turned off, it will save your game on pause AND on level exit to reduce lost attempts
- added the "runatt" special key to show your death tracker attempts from each start percent of a run
- added the "recentDeaths" special key to display your top 10 most recent deaths!
- added a way to insert a column to the layout between existing columns instead of only at the end
- cheat detection now flags runs as cheatec instead of outright deleting ur runs
- added the "cheatedGeneral" "cheatedRuns" and "cheatedAttempts" to display your cheat flagged runs
- improved link menu loading time

- fixed lots of bugs

## v3.0.9
- fixed a bug where the first run after opening a level when not playing from 0 will not track for no reason
- fixed deleting save data while inside of a level causing data corruption
- created new sprites for the "mod options" and "level options"
- fixed timewarp bugs (i think)
- fixed deleting data while playtime tracker is installed not deleting the playtime correctly

## v3.0.8
- fixed invalid directory error not showing when its supposed to
- fixed new best desync (should prevent random unremovable new bests and other new best issues hopefully)
- improved some error messages
- changed "Late Save" to "Quick Save" flipping the action of the setting, this is due to feedback from people getting lag on death
- added session mode to calculator
- fixed many crashes and bugs
- added faded version text to bottom left of the screen
- added a "skip tutorial" button to the tutorial dialogues
- named my async tasks (about time)
- re-added "real end percent"
- added open save file directory button to save menu

## v3.0.7
- new best colors are back!
- added alternating bright/normal colors for runs
- made it so backups would not lag the game
- small fixes to text and in general
- hide header option added to label settings
- even more corruption safty added while playing a level
- layout presets added

## v3.0.6
- Removed the keyword "Error:" from some things that shouldt have it
- Extra safty protocols for corrupted saves
- Remove Invalid Runs button added to the modify tab
- Small changes to default graph and label layout
- Made backup creating and reverting safer
- Added a counter showing the amount of hidden runs according to your settings
- Made graph data appear lower down on mobile devices se it is seen
- Fixed some people having the old Seconds session method

## v3.0.5
- Fixed crash to do with unicode paths

## v3.0.4
- Fixed session groups not being time zone relative
- Fixed crash on level complete screen
- Added scrollable text input for easier layout management
- now with "Better Touch Prio", touch issues in layout editing are fixed
- fixed misspells

## v3.0.3
- Made sure "Shared Max To Hide" defaults to 0
- fixed misspells

## v3.0.2
- Improved speedhack detection
- Fixed crash on android on level entry
- Fixed two 4s on lvlruns key
- Made death tracker button disappear when clicking the eye button on the level complete screen
- Fixed session date not accounting for timezone
- Fixed an issue were sometimes it wont track runs into sessions
- Added "Shared Max To Hide" for easier filter management for runs
- Fixed safe mode randomly being enabled for some people

## v3.0.1
- Fixed crash on invalid save file location

## v3.0.0
- backend optimized and cleaned up
- layout system revamped!
  - more than 2 columns
  - custom column size
  - new labels!
  - more options for coloring
  - new label movement
  - async optimized special key loading
  - you can now ZOOM IN in the label view by holding control (or pinching)!
- settings menu revamped!
- Added sections!
- Graphs reworked!
  - you can now add multiple graphs and overlay them!
  - graphs have settings you can play around with to customize them
  - graphs can be turned on and off by clicking on the colored button below the name
  - graphs can be reordered to decided which is above which
  - you can zoom in to the graphs by scrolling and holding control (or pinching)
- Added many new special keys!!!!
- Added Tutorials!
- Accurate playtime and **Playtime Tracker** integration!
- Added calculator!
- Session grouping
- Cheat detection!!!

## v2.4.6
- iOS support

## v2.4.5
- lots of link layer fixes :)

## v2.4.4
- added playtime calculation! (use special keys {ptf0},{ptrun} and {ptall} to see it!!)
- changed default layout! (if you never changed your layout before, resetting it to default is recommended!)

## v2.4.3
- organized code!!!!! added comments to everything!!!!!!

## v2.4.2
- fixed a crash bug on launch (macos)

## v2.4.1
- fixed macos inline crash
- fixed a TON of text alignment issues
- made stat calculations happen in the background, should result in less lag when navigating the menus
- added the "hide runs by length" to let you remove some clutter in the menu
- now removing a run from zero completely will also erase it from your "new bests"
- added the option to have the death tracker button on the left menu in the level page
- added the option to change the graph points size
- some bug fixes

## v2.4.0
- updated for geode v4.0.0-beta.1
- completely revamped the 'LabelSettingsLayer', it is now a lot better and comprehensive
- reworked/organized some code

## v2.3.4
- fixed a crash when opening the link layer (i hope) why are there so many link layer related crashes :sob:

## v2.3.3
- removed all try catch statements from the code cz apparently exceptions dont work on android :) didnt know about that lol

## v2.3.2
- fixed a bug where exiting the link menu would crash (tysm firee :DD)

## v2.3.1
- fixed a bug where the session method would reset whenever opening the game for some people

## v2.3.0
- removed most UI from the main page
- added a level options page where all the level-specific settings are now
- fixed some typos
- added an option to hide all runs up to a certain percentage
- edit layout now has a new button
- removed "safe mode" due to bugs

## v2.1.2
- fixed a crash bug on macos
- fixed some typos

## v2.1.1
- fixed a bug on android where checkpoints will crash your game (hopefully)
- added some new text keys! {ssd} - selected session start date, and {sst} - selected session start time
- some small fixes

## v2.1.0
- added the option to change the new best colors
- added a new type of graph
- added a new logo made by taz!

## v2.0.1
- fixed a crash bug when clicking cancel on edit layout

## v2.0.0 full release!
- <cg>added Android support</c>
- <cg>added macOS support</c>
- <cg>code refactor</c>
- <cg>added a layout system</c>
- <cg>added run tracking</c>
- <cg>added multi-session tracking</c>
- <cg>complete UI overhaul</c>
- <cg>added graphs</c>
- <cg>linking between levels</c>
- <cg>saved level support</c>
- <cg>custom save data folder path</c>
- <cg>import/export save data</c>

## v2.0.0-beta.13
- 'Track any runs' is now on by default
- added the option to export your saved runs into text files
- added the option to import existing level progress files into death tracker
- a few bug fixes

## v2.0.0-beta.12
- added late save for people who get lag spikes on death
- added the option for a custom save data folder
- fixed bugs
- added support for logging runs on editor levels, ty cvolton for making 'Editor Level ID API' :D

## v2.0.0-beta.11
- updated for geode 3.0.0
- the mod now lets you see your saved level stats but they are still not seperated
- fixed some bugs

## v2.0.0-beta.10
- updated for beta 1

## v2.0.0-beta.9
- added the option to access the tracker menu though the pause menu
- added the option to copy text from labels

## v2.0.0-beta.8
- updated for alpha 2

## v2.0.0-beta.7
- updated to 2.206 :D

## v2.0.0-beta.6
- fixed run tracking being broken (oops)

## v2.0.0-beta.5
- stopped macOS support temporarily
- fixed platformer level tracking
- fixed lots of crash bugs
- added the option to delete the current level save
- added the option to revert your save back to a backup in the case of the saves corruption

## v2.0.0-beta.4
- added the new UI and removed the old one
- added linking levels
- added graphs
- added multi session tracking
- added run tracking
- added manual run management
- added layout customization
- a lot of fixes
- completions are now tracked as 100% instead of 0-100 runs

This update has been in the works for the last 1.5 months so I really hope you enjoy it :D

## v2.0.0-beta.2
- fixed a crash on android

## v2.0.0-beta.1
- added android support
- code rewrite
- added scrolling
- added multi-session tracking
- added run tracking

## v1.7.2
- fixed an incompatibility with xdBot

## v1.7.1
- added platformer per-checkpoint new bests
- added session new bests
- added death pass rates

## v1.6.1
- added a setting for customizing session length
- added a setting to disable death tracking from startpos
- added changelog
- added mod tags

## v1.5.1
- fixed a bug where dual deaths count twice

## v1.5.0
- added session death tracking

## v1.4.3
- fixed a crash

## v1.4.2
- fixed a crash with better info

## v1.4.1
- fixed sprite scaling issue on medium/low graphics
- fixed a crash

## v1.4.0
- added per-checkpoint death tracking in platformer mode

## v1.3.3
- fixed an incompatibility with better info

## v1.3.2
- fixed a crash

## v1.3.1
- removed console logs

## v1.3.0
- added copy deaths button

## v1.2.1
- fixed an incompatibility with better info
- fixed a crash in the editor
- added keybinds support for left/right page arrows

## v1.2.0
- added setting to disable death tracking on completed online levels

## v1.1.1
- moved skull button to an easier to spot location
- fixed a visual bug with skull button
- fixed a bug where skull button would appear on every popup

## v1.1.0
- made new bests yellow

## v1.0.3
- fixed an issue where daily/weekly/gauntlet level deaths counted for the same online level
- made the back button on the death tracker popup more visible

## v1.0.2
- fixed a crash with 1 attempt levels

## v1.0.1
- fixed a crash with better info
- fixed a bug that tracked deaths on your uploaded online level if you played your local copy of the level

## v1.0.0
- initial release :tada:
- why did you scroll down here