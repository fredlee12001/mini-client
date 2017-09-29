
### Table of Contents
 - [Scope](#scope) 
 - [Prerequisities](#prerequisities)
 - [VS Project Creation](#vs-project-creation)
 - [VS Project Update](#vs-project-update)  
 - [Useful Shortcuts](#useful-shortcuts)

# Scope
The scope of this document is to explain how to create Visual Studio Project and update it.  
It also introduces very useful shortcuts

# Prerequisities
1. Visual Studio 2013 is installed.
1. Visual Assist is installed
1. Premake 5.0 is installed and added to your Windows PATH
1. Python is installed

# VS Project Creation
We use premake for VS Project creation from existing tree.
We don't commit VS Project files to the tree, but rather create it on our local machine.

1. Open cmd prompt in your Windows
1. Go to your Provisioning tree
1. Run `python devenv\visual_studio\vs_generate.py` command
1. VS Project files should be generated

# VS Project Update
There is list of all the paths that are taken to the VS Project in `devenv\visual_studio\vs_generate\premake5.lua` file.  
The list is in "files" tag.  
"remove files" tag includes all the files that need to be ignored and are not taken into the VS Project.

1. If you added a new file that is located in the path that is in the list, you just need to refresh your project by running `vs_generate.py` script again.
1. If the file that you've added is located in the path that isn't in the list, just update `premake5.lua` file and run `vs_generate.py` again.
Don't forget to commit `premake5.lua` file.

# Useful Shortcuts
1. F12 on function name - Go to definition
1. Ctrl+F12 on function name - Go to declaration
1. Ctrl+Shift+F - Find in files
1. Shift+Alt+F on function name - Find all references
1. Ctrl+i - Interactive search
1. Ctrl+F3 - Select for searching
1. F3 - Search next
1. Shift+F3 - Search previous
1. Shift+Alt+O - Open file
1. Ctrl+- - Go back
1. Ctrl+Shift+- - Go forward
1. Ctrl+Tab - Switch between open windows
1. Shift+Alt+R on function name - Rename function
1. Alt+G on fucntion name - Go to definition even on "gray" fucntions
1. Alt+G on include files - Open include files
1. Alt+Shift+G on include files - Open a dialog that gives you to open files that are included in the header file or the files that includes this header file
1. Shift+Alt+S - Find symbol
