# Table of contents

- [Table of contents](#table-of-contents)
- [**SFPGenerator**](#sfpgenerator)
  - [**Support**](#support)
  - [**General structure**](#general-structure)
  - [**Creating a template**](#creating-a-template)
  - [**Registering a template**](#registering-a-template)
  - [**Generating from a template**](#generating-from-a-template)
  - [**Custom IDE**](#custom-ide)
  - [**Custom Macros**](#custom-macros)
  - [**Context / rightclick menu (WINDOWS ONLY)**](#context--rightclick-menu-windows-only)
  - [**Build**](#build)
  - [**Final Example**](#final-example)

# **SFPGenerator**

SFPGenerator, short for straight forward project generator, is a simple command line tool for generating projects from simple and easy to construct templates.

It can be used for any programming language, or anything else that needs templates for that matter.

Aswell as generating projects, it can also automatically open the newly generated projects in a user specified IDE.

If desired, the program can also be added to the right click menu for explorer. This makes the program both easy to use and quick to run.

## **Support**

This project was developed and tested on windows, although it should still be cross platform.

However, support for automatic context / right click menu is windows only.

cmake and cpack is needed to build from source as they function as the projects build/install-system. These programs are **only** needed to build the project, and not run it, so these will not be needed if an already generated installer is used.

the CARG submodule is also needed if build from source, so make to clone this repository recursively!

## **General structure**

The SFPGenerator requires a Templates.csv file specifying the locations of all the desired .sft (straight forward template) files.

The directories containing these .sft files is the template itself.
This file exists as a way for the SFPGenerator to know how the template folder shall be restructured and remapped when generating the project.

The name of the template is also defined in the .sft file.

## **Creating a template**

A template requires a folder structure already made and a .sft file, as explained earlier.

The .sft file contains the mapping of the current folder structure to the resulting structure, as well as the name of the template.

A .sft file always starts with the project name at the very top of the file. This is then followed by the mapping of the current template folder structure to the resulting generated structure.
Each mapping will have the syntax source:destination, where source and destination are both either a folder or a file.

### .sft file example

> *template.sft*
>
> TemplateName
>
> source_1 : destination_1
>
> source_folder/source_2 : destination2
>
> and so on...

*note how the template mapping does not need to correspond to the generated mapping.*

The source files can contain macros that will be replaced in their destination file by the corresponding macro values
Marco syntax functions as follows: |MACRO|, where MACRO is the name of the macro.
furthermore, a macro can be escaped by surrounding them with two pipes instead of one.

| source        | destination|
|-----------    |------------|
| \|MACRO\|     | MACRO_VALUE|
| \|\|MACRO\|\| | \|MACRO\|  |


pipes only need to be escaped if they otherwise would form the syntax of a macro, meaning a | b does not need an extra pipe.

[//]: # (Ironically enough, a lot of the pipes used in the taple needed to be escaped)

Macros available are:
> NAME (Name of the project
>
> TARGET_DIR (The parent directory of the generated project)
>
> PROJECT_DIR (The parent directory of the generated project)
>
> TEMPLATE_DIR (The directory of the template)

## **Registering a template**

A template can be registered by placing the path to the .sft file in the *Templates.csv* file.

The *Templates.csv* file should be placed in the same directory as the executable.

### Template registration example

> *Templates.csv*
>
> foo/template_a/template_file.sft
>
> bar/template_b/template file with space but no quotes.sft

paths should never be encapsulated with quotes (") and should always be separated by newlines.

## **Generating from a template**

The program needs 3 inputs to be able to generate a project, the target directory, the project name and finally the template name.

These arguments can either be provided when calling the program through the command line, or if not passed as arguments, then, the user will be prompted to input them into the terminal.

> SFGP --TargetDir TARGET_DIRECTORY --ProjectName PROJECT_NAME --Template TEMPLATE_NAME

The order of the flags do not matter.

### **Target directory**

The directory where the generated project will be placed

&nbsp;

### **Project name**

The name of the folder the generated project will be placed in, as well as the value of the |NAME| macro.

&nbsp;

### **Template name**

The name of the target template.

If no name has been provided, and the program has generated a template before, the user can input nothing when prompted to specify the template and the program will instead use the last used template as the target template.

If available, this default template will be displayed in parentheses next to the user prompt.

## **Custom IDE**

The custom IDE functionality is optional per template, and it is therefore not required to have this file in each template.

If no Custom IDE file is specified, SFPGenerator will simply quit when it has finished generating the project.

To specify a custom IDE that should open the project right after it has been generated, a *CustomIDE* file is needed.

this file should be placed in the same directory as the .sft file.

The file simply contains the path to the executable of the IDE and any optional arguments.
However, it is required that the IDE can open a project from the command line, as SFPGenerator simply calls the command in the file, suffixed with the path to the project.
The user should therefore make sure the necessary arguments for this to be possible, already exists in the *CustomIDE* file.

### CustomIDE example

this is an example of how a user might add visual studio as the default IDE for a specific template.

> *CustomIDE*
>
> path/to/vs/devenv.exe

### **pro tip**

It can sometimes be infuriating to copy the same *CustomIDE* file repeatedly, as you might only use this single IDE and will never use anything else.

Now you might wonder how this could be solved, well I have just the solution for you...

Simply create a template for your templates which contains this *CustomIDE* file and use this every time you need to generate a new template!

A sort of template-template one could say...

## **Custom Macros**

If an unknown macro appears in a file, the user will be prompted for a value which will then be tied to the macro for the rest of the generation. In order to define a macro without inputting it at generation time, one should define it by passing -D to *SFPGenerator.exe*

> -D MACRO_NAME MACRO_VALUE

There are no limits to the number of definitions passed, however, a macro must only be defined once.

another option is to create a CustomMacro file. This option also allows for a definition of a default value, meaning the user still will be prompted to define the macro, but they will have the option of passing nothing which will result in the macro using its default value.

A CustomMacro file should consist of a macro name followed by either a : (actual value) or a ; (default value) and lastly the value the macro should have. Alternatively the seperator and value can be omitted, this will result in the program prompting the user for a value without the option of a default value.

> *CustomMacro*
>
> MACRO_NAME : MACRO_VALUE
> MACRO_NAME_DEFAULT ; MACRO_VALUE_DEFAULT

## **Context / rightclick menu (WINDOWS ONLY)**

To set up a right click menu, the script "setup_rightclick.bat" should be called.

This will require administrator privileges as it is needed to modify the registry to add the menu.

The final menu will be set up as the example below

> No Template
>
> -----------
>
> Template 1
>
> Template 2
>
> ...
>
> Template n
>
> -----------
>
> Update

Each template name must be added to a *QuickTemplates.csv* file, that should be located inside the install directory.

### No Template
simply calls *SFPGenerator.exe* with the current directory as the target directory and with no templates.

### Template section
calls *SFPGenerator.exe* with the selected template passed to the --Template flag and uses the current directory as the target directory.

### Update
calls *setup_rightclick.bat*, meaning it will update the menu so its synced with the newest *QuickTempaltes.csv* file.

### !IMPORTANT!

if uninstalling, either run *remove_rightclick.bat* and then *uninstall.exe* or run the *copmlete_uninstall.bat* script, in order to make sure the right clikc menus are removed as well.

## **Build**

This project uses cmake to generate, build and install the project.

For a quick build on windows, the script *build.bat* should be executed. This will generate an installer that automatically installs the necessary files to a user specified install directory.

The generated installer is called *SFPGenerator-installer* and will be located in the build directory




For the right click setup project to be generated -DSFPG_RIGHTCLICK=ON should be passed to cmake when generating the build.

> *with right click setup*
>
> cmake -B./build -DSFPG_RIGHTCLICK=ON 

Otherwise -DSFPG_RIGHTCLICK=OFF should be passed

> *without right click setup*
>
> cmake -B./build -DSFPG_RIGHTCLICK=OFF 


To build the project, cd into the build directory and call cmake --build with the wanted configuration

> cd ./build && cmake --build --config (Debug/Release)

To generate the installer call cpack with the same configuration passed as in the build step

> cpack -C (Debug/Release)

Finally, run the generated installer to install the project.


## **Final Example**

A small example of a possible file structure can be seen below

```txt
============= Folder structure ===============================================
.
└── templates_folder
    └── template
        ├── folderA
        |   └── fileA.txt
        ├── folderB
        |   └──
        ├── fileB.txt
        ├── template.sft
        └── CustomIDE
------------------------------------------------------------------------------
.
└── bin
    ├── SFPGenerator.exe
    └── Templates.csv

============= template.sft ===================================================
TemplateName
folderA/fileA.txt : A.txt
folderB/ : Hello/
fileB.txt : folder/B.txt

============= fileA.txt ======================================================
Hello, this projects name is |NAME|!!!

============= fileB.txt ======================================================
Hello, this projects name is ||NAME||!!!

============= CustomIDE (vs 2022) ============================================
C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe

============= Command ========================================================
>> SFPGenerator.exe --TargetDir projects --ProjectName MyProject --Template TemplateName

============= Resulting structure ============================================
.
└── projects
    └── MyProject
        ├── A.txt
        ├── folder
        |   └── B.txt
        └── Hello
            └──

============= A.txt ==========================================================
Hello, this projects name is MyProject!!!

============= B.txt ==========================================================
Hello, this projects name is |NAME|!!!


```

