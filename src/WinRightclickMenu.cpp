#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <carg.h>
#include <filesystem>
#include <ShObjIdl_core.h>

// will be true if a windows error occured at some point during the setup
static bool finishedWerror = false;

// prints the last Win32 error, in string format.
void WinErr(HRESULT err)
{
    if (err == 0) {
        return;
    }

    finishedWerror = true;

    LPSTR message = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, NULL);


    std::cout << message << '\n';

    LocalFree(message);

}

#define KEY_STR(str) (BYTE*)TEXT(str), DWORD(strlen(TEXT(str)) + 1)

std::string getTempalteName(std::string sft_file)
{
    std::string name;
    std::ifstream file_in(sft_file);
    
    std::getline(file_in, name);

    return name;
}

void install(HKEY key_handle, const char* exec_dir)
{
    // set the name of the context menu

    LSTATUS result;

    result = RegSetValueEx(key_handle, TEXT("MUIVerb"), NULL, REG_SZ, KEY_STR("SFPGenerator"));

    WinErr(result);

    // enable nested context menu

    result = RegSetValueEx(key_handle, TEXT("SubCommands"), NULL, REG_SZ, KEY_STR(""));

    // assumes the context menu setup executable is in the same location as the SFPGenerator executable
    std::filesystem::path sfpg_path = std::filesystem::path(exec_dir).parent_path() / "SFPGenerator.exe";
    std::filesystem::path templates_path = std::filesystem::path(exec_dir).parent_path() / "QuickTemplates.csv";

    std::ifstream tempalte_file(templates_path);


    // start off with creating a "no template" shortcut
    {
        HKEY sub_key;
        const DWORD seperator_after = ECF_SEPARATORAFTER;

        // 0_ makes sure the refresh command always will be on the top of the menu
        result = RegCreateKeyEx(key_handle, TEXT("shell\\0_No Template\\command"), NULL, NULL, NULL, KEY_WRITE, NULL, &sub_key, NULL);

        WinErr(result);

        std::stringstream command;

        command << sfpg_path << " --TargetDir \"%V\"";

        result = RegSetValueEx(sub_key, NULL, NULL, REG_SZ, KEY_STR(command.str().data()));

        WinErr(result);

        result = RegCloseKey(sub_key);

        WinErr(result);

        result = RegOpenKeyEx(key_handle, TEXT("shell\\0_No Template"), NULL, KEY_WRITE, &sub_key);

        WinErr(result);

        result = RegSetValueEx(sub_key, TEXT("CommandFlags"), NULL, REG_DWORD, (BYTE*) &seperator_after, sizeof(DWORD));

        WinErr(result);

        result = RegSetValueEx(sub_key, TEXT("MUIVerb"), NULL, REG_SZ, KEY_STR("No Template"));

        WinErr(result);

        RegCloseKey(sub_key);
     
    }

    {
        // then create an "update" shortcut
        const DWORD seperator_before = ECF_SEPARATORBEFORE;
        HKEY sub_key;

        // 2_ makes sure the update command always will be on the botton of the menu
        result = RegCreateKeyEx(key_handle, TEXT("shell\\2_Update\\command"), NULL, NULL, NULL, KEY_WRITE, NULL, &sub_key, NULL);

        WinErr(result);

        std::string command = (std::filesystem::path(exec_dir).parent_path() / "SFPGenerator_rightclick.exe").string();

        result = RegSetValueEx(sub_key, NULL, NULL, REG_SZ, KEY_STR(command.c_str()));

        result = RegCloseKey(sub_key);

        WinErr(result);

        result = RegOpenKeyEx(key_handle, TEXT("shell\\2_Update"), NULL, KEY_WRITE, &sub_key);

        WinErr(result);

        result = RegSetValueEx(sub_key, TEXT("CommandFlags"), NULL, REG_DWORD, (BYTE*)&seperator_before, sizeof(DWORD));

        WinErr(result);


        result = RegSetValueEx(sub_key, TEXT("MUIVerb"), NULL, REG_SZ, KEY_STR("Update"));

        WinErr(result);

        RegCloseKey(sub_key);
    }

    // loop over every template and add a new subkey, with a command calling SFPG with the template

    std::string template_name;

    while (std::getline(tempalte_file, template_name))
    {
        // extract template name from .sft file
        template_name = getTempalteName(template_name);
        HKEY sub_key;

        std::stringstream name;

        // 1_ makes sure templates always will be in the middle of the menu
        name << "shell\\1_" << template_name;

        // create key for each quick access tempalte
        result = RegCreateKeyEx(key_handle, TEXT(name.str().data()), NULL, NULL, NULL, KEY_WRITE, NULL, &sub_key, NULL);

        WinErr(result);

        result = RegSetValueEx(sub_key, TEXT("MUIVerb"), NULL, REG_SZ, KEY_STR(template_name.c_str()));

        WinErr(result);

        result = RegCloseKey(sub_key);

        WinErr(result);

        name << "\\command";

        result = RegCreateKeyEx(key_handle, TEXT(name.str().data()), NULL, NULL, NULL, KEY_WRITE, NULL, &sub_key, NULL);

        WinErr(result);

        std::stringstream command;

        // setup command where the template passed is the wanted quick access tempalte
        command << sfpg_path << " --Template \"" << template_name << "\" --TargetDir \"%V\"";

        result = RegSetValueEx(sub_key, NULL, NULL, REG_SZ, KEY_STR(command.str().data()));

        WinErr(result);

        RegCloseKey(sub_key);
    }
}

int main(int argc, char** argv)
{
	HKEY local_machine_handle = HKEY_LOCAL_MACHINE;

	HKEY key_handle;

    LSTATUS result;

    try
    {
        carg::ArgParser cmd_parser({ {"/uninstall", carg::ArgProp(0, 1, carg::CMDTypes::None)}, {" ", carg::ArgProp(0, 0, carg::CMDTypes::None)} });
        carg::ParsedArgs parsed = cmd_parser.parseArgs(argc, argv);

        if (parsed["/uninstall"].size() == 1)
        {
            // simply deletes the key from the regristry, if it existts

	        LPCSTR target_key_name = TEXT("SOFTWARE\\Classes\\Directory\\background\\shell");

            // open the key holding the SFPG key as a sub key
            result = RegOpenKeyEx(local_machine_handle, target_key_name, NULL, DELETE | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE, &key_handle);

            WinErr(result);

            {
                // only used to close the key as soon as it has been opened
                HKEY tmp_handle;

                // attempting to open the key to check if it exists
                result = RegOpenKeyEx(key_handle, "SFPG", NULL, NULL, &tmp_handle);

                WinErr(RegCloseKey(tmp_handle));
            }

            if (result == ERROR_FILE_NOT_FOUND || result == ERROR_PATH_NOT_FOUND)
            {
                // no keys were installed in the first place
                return 0;
            }

            WinErr(result);

            // delete the SFPG key and all of its values
            result = RegDeleteTree(key_handle, "SFPG");

            WinErr(result);

            result = RegCloseKey(key_handle);

            WinErr(result);

            if (!finishedWerror)
                std::cout << "Sucsesfully removed right click menu!\n";
            else
                std::cout << "Setup exited with errors!\n";

        }
        else
        {
            LPCSTR new_key_name = TEXT("SOFTWARE\\Classes\\Directory\\background\\shell\\SFPG");
	        DWORD operation_result; // created key or opened key
            
            // open or create key depending wether it exists or not
            result = RegCreateKeyEx(local_machine_handle, new_key_name, NULL, NULL, NULL,
                KEY_WRITE, NULL, &key_handle, &operation_result);

            WinErr(result);

            // start of by removing any existing values in the SFPG key
            if (operation_result == REG_OPENED_EXISTING_KEY)
            {
                result = RegDeleteTree(key_handle, "Shell");

                
                // ignore if SFPG contains no subkeys -> either an unproper uninstall or an install with no templates
                if(result != ERROR_FILE_NOT_FOUND)
                    WinErr(result);
            }


            install(key_handle, argv[0]);

            result = RegCloseKey(key_handle);

            WinErr(result);

            if (!finishedWerror)
                std::cout << "Sucsesfully setup right click menu!\n";
            else
                std::cout << "Setup exited with errors!\n";
        }
    }
    catch (carg::InvalidCArg& e)
    {
        std::cout << e.what() << '\n';
    }

}
