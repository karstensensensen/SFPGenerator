#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <unordered_map>

using std::filesystem::path;

#define ERR(msg) std::cout << msg << "\nPress any key to exit..."; std::cin.get(); exit(-1);

// maps the name of the template to the directory of the template file
using TemplateMap = std::map<std::string, std::filesystem::path>;

static std::unordered_map<std::string, std::string> macro_map;

TemplateMap loadavailableTemplates(path template_file)
{
	TemplateMap available_templates;

	// load all the available templates from the template file
	std::ifstream available_templates_file(template_file);
	std::string template_file_name;

	while (std::getline(available_templates_file, template_file_name))
	{
		// get the name of the tempalte
		// the name should always be at the top of the template file

		std::string template_name;

		std::ifstream template_file(template_file_name);
		std::getline(template_file, template_name);

		available_templates[template_name] = template_file_name;
	}

	return available_templates;
}

void printavailableTemplates(TemplateMap& templates)
{
	size_t max_columns = 5;

	size_t col = 0;

	for (auto& [key, _] : templates)
	{
		if (col > max_columns)
		{
			col = 0;
			std::cout << '\n';
		}

		std::cout << std::left << std::setw(32) << key;

		col++;
	}

	std::cout << '\n';
}

// replaces maros with their corresponding values and copies the resulting file to the target path
void transferTemplateFile(path source_file, path target_file)
{
	std::ifstream source_in(source_file);
	std::ofstream target_out(target_file);

	char c;

	while (source_in.read(&c, 1))
	{
		if (c == '|') // macro detected
		{
			std::streampos fallback_pos = source_in.tellg();
			source_in.read(&c, 1);

			if (c != '|') // if double pipes, macro should be ignored
			{
				source_in.seekg(fallback_pos); // undo read of first character in macro name
				std::string macro_name;
				std::getline(source_in, macro_name, '|');

				if (!macro_map.contains(macro_name))
				{
					ERR("No such macro exists |" << macro_name << "|\n");
				}

				target_out.write(macro_map[macro_name].data(), macro_map[macro_name].size());

				continue; // c should not be written to the file
			}
		}

		target_out.write(&c, 1);
	}

	target_out.close();
}

// Takes the name of the template as well as the name of the project and generates a resulting CMake project out of it, which can be opened with visual studio
int main(size_t argc, char** argv)
{
	// Check if needed files exist

	path templates_file = path(argv[0]).parent_path() / "Templates.csv";

	if (!std::filesystem::exists(templates_file))
	{
		ERR("Missing \"Templates.csv\" file, stopping program\n");
	}

	std::string target_dir;
	std::string project_name;
	std::string target_template_name;
	TemplateMap available_templates = loadavailableTemplates(templates_file);
	
	// load template used from last session as the default target_tempalte
	if (std::filesystem::exists(path(argv[0]).parent_path() / ".LastTemplate"))
	{
		std::ifstream last_template(path(argv[0]).parent_path() / ".LastTemplate");
		std::getline(last_template, target_template_name);

		if (!available_templates.contains(target_template_name))
		{
			std::cout << "Template \"" << target_template_name << "\", used from last session, was not found";

			target_template_name = "";
		}
	}

	if (argc <= 1)
	{
		std::cout << "Target Directory: ";
		std::getline(std::cin, target_dir);
	}
	else
	{
		target_dir = argv[1];
	}

	if (argc <= 2)
	{
		std::cout << "Project Name: ";

		std::getline(std::cin, project_name);
	}
	else
	{
		project_name = argv[2];
	}

	if (argc <= 3)
	{
		std::cout << "Avalible templates:\n";

		printavailableTemplates(available_templates);

		bool found_template = false;

		while (!found_template)
		{
			if (target_template_name != "")
			{
				std::cout << "\nTemplate (" << target_template_name << "): ";
			}
			else
			{
				std::cout << "\nTemplate: ";
			}

			std::string result;

			std::getline(std::cin, result);

			if (result == "" && target_template_name == "") // use default template, if available
			{
				std::cout << "No default template was found\n";
			}
			else if (result != "")
			{
				if (available_templates.contains(result))
				{
					target_template_name = result;

					found_template = true;
				}
				else
				{
					std::cout <<  "No template with the name \"" << result << "\" was found\n";
				}
			}
			else // default template will be used
			{
				found_template = true;
			}
		}

	}
	else
	{
		target_template_name = argv[3];
	}

	if (!available_templates.contains(target_template_name))
	{
		ERR("No template with the name \"" << target_template_name << "\" was found, stopping program");
	}

	// save the last used template as the default template
	std::ofstream default_template_out(path(argv[0]).parent_path() / ".LastTemplate");

	default_template_out << target_template_name << '\n';
	default_template_out.close();

	if(argc > 4)
	{
		ERR("Too many command line arguments passed, expected at max 3, got " << argc - 1 << '\n');
	}
	
	std::cout << "Project \"" << project_name << "\" will be created at \"" << target_dir << "\" using template \"" << target_template_name << "\"\n\n";

	
	// setup paths

	path template_path = available_templates[target_template_name].parent_path();
	path target_path = path(target_dir);
	path project_path = target_path / project_name;

	if (std::filesystem::exists(project_path) && !std::filesystem::is_empty(project_path))
	{
		ERR("Project directory \"" << project_path << "\" must be empty, stopping program");
	}

	
	// set macros
	
	macro_map["NAME"] = project_name;
	macro_map["TARGET_DIR"] = target_dir;
	macro_map["PROJECT_DIR"] = project_path.string();
	macro_map["TEMPLATE_DIR"] = template_path.string();

	// =========== generate project ===========

	std::ifstream cmake_template_file(available_templates[target_template_name]);

	std::string cmt_line;
	// skip the name
	std::getline(cmake_template_file, cmt_line);

	// read from template file
	while (std::getline(cmake_template_file, cmt_line))
	{
		std::string_view source_file;
		size_t seperation_index = cmt_line.find(':');

		if (seperation_index != cmt_line.find_last_of(':'))
		{
			ERR("More than one ':' character found in item entry, stopping program\n" << cmt_line);
		}

		if (seperation_index == std::string::npos)
		{
			ERR("No ':' character was found in item entry, stopping program\n" << cmt_line);
		}

		source_file = std::string_view(cmt_line.begin(), cmt_line.begin() + seperation_index);
		
		std::string_view destination_file;
		
		//                                                     makes sure there is no space in the target name " TEST.txt" -> "TEST.txt"
		destination_file = std::string_view(cmt_line.begin() + cmt_line.find_first_not_of(' ', seperation_index + 1), cmt_line.end());
		
		path source_path = template_path / source_file;
		path destination_path = project_path / destination_file;

		if (!std::filesystem::exists(source_path))
		{
			ERR("Source item must exist in the tempalte file directory, stopping program\n" << source_path);
		}

		if (!std::filesystem::is_regular_file(source_path))
		{
			std::filesystem::create_directories(destination_path);
		}
		else
		{
			std::filesystem::create_directories(destination_path.parent_path());
			transferTemplateFile(source_path, destination_path);
		}
	}

	path custom_ide_path = template_path / "CustomIDE";
	// check if a custom ide has been supplied
	if (std::filesystem::exists(custom_ide_path))
	{
		std::cout << "Found CustomIDE file\n\n";

		std::ifstream custom_ide_in(custom_ide_path);
		std::stringstream command;
		std::string ide_exec;
		
		// first (and only) line should be the executable path
		std::getline(custom_ide_in, ide_exec);

		if (std::filesystem::exists(ide_exec))
		{
			// surround with quotes incase there are spaces in the path
			command << "\"\"" << ide_exec << "\" " << project_path << '"';
			
			std::cout << "Attempting to open IDE...\n";

			int res = std::system(command.str().data());

			if (res != 0)
			{
				// pause and display command executed, if return code is not 0
				std::cout << "\nProcess exited with a failure code\n";
				std::cout << "Command: " << command.str() << '\n';
				std::cin.get();
			}
		}
		else
		{
			std::cout << "Could not find executable provided in the CustomIDE file\n" << ide_exec << "\nPress any key to exit...\n";
			std::cin.get();
		}
	}
	else
	{
		std::cout << "Succesfully created project from template\nPress any key to exit...";
		std::cin.get();
	}
}
