
/*
 *
 *
 *
 *
 */

 #include <string>
 #include <vector>

// The base command class
class CatComBase {
public:
   // Constructor, with name of your command and a callback to a function with a vector of strings to use as command arguments
   CatComBase(std::string _command_name);
   ~CatComBase();
   std::string command_name;
   virtual void callback(std::vector<std::string>) = 0;
 };

 void CallCommand(std::string input);
