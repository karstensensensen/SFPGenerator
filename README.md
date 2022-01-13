# Usage
The program will create the project in the folder passed as the first argument
> CPG *path*

afterwards the user will be prompted for the project name.

&nbsp;

*The project name is not passed as an commandline argument in case the user decides to add the program to their right click menu.*

&nbsp;

and finally the name of the template will be asked for.

If the user inputs nothing, the last loaded template will be used.

# Registrering a template
A tempalte can be regristrered by placing the path to the .cmt file in the *Templates.dat* file.

If no such file is present it should be created in the same directory as the executable.

# Creating a template
A CMake template requires a folder structure already made and a .cmt file.

The .cmt file contains the mapping of the current folder structure to the resulting structure as well as the name of the template.

> TemplateName
> 
> OriginFile1:DestinationFile1
> 
> OriginFolder/OriginFile2:DestinationFile2
> 
> and so on...

These files can contain macros that will be replaced in their destination file by the corresponding macro value
Macros should be used as |MACRO|.

Macros avaliable are:
> NAME (Name of the project)

## Example

>TEST.txt
>
> Hello, |NAME|

>Template.cmt
>
> Template
>
> "TEST.txt" "Destination.txt"
>

The result with generating a project with the name "Project", from this template is visualized below

> Project/Destinaion.txt


>Destination.txt
> 
> Hello, Project
> 
