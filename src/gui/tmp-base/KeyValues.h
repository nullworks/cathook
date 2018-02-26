
/*
 *
 * Nekohooks fix for keyvalues
 *
 */

#include <unordered_map>

class KeyValues {
  std::unordered_map<std::string, bool> bool_values;
  std::unordered_map<std::string, int> int_values;
  std::unordered_map<std::string, std::string> string_values;
public:
  void SetBool(std::string in_string, bool in_value) {

  }
  bool GetBool(std::string in_string){

  }
  void SetInt(std::string in_string, int in_value){

  }
  int GetInt(std::string in_string){

  }
  void SetFloat(std::string in_string, int in_value){

  }
  int GetFloat(std::string in_string){

  }
  void SetString(std::string in_string, std::string in_value){

  }
  std::string GetString(std::string in_string){

  }
};
