
/*
 *
 *
 *
 *
 */

 #include <string>
 #include <vector>

// The base command class
 class CatCommand {
   // Constructor, with name of your command and a callback to a function with a vector of strings to use as command arguments
   CatCommand(const char* _command_name, void(*_callback)(std::vector<std::string>));
   ~CatCommand();
   const char* command_name;
   void(*_callback)(std::vector<std::string>)
 };

 void CallCommand(std::string input);
