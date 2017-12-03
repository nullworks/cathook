
/*
 *
 *  This file helps deal with our modular functions
 *  The purpose to keep modularity without
 *
 */

#pragma once

#include <functional> // std::function

// Use when you have a function that you wish to be modular
// Give it a type of a function return type, then the function return parameters
// ex: CMFunction<CatEntity, const int&, const CatVector&> example_function
/*template <typename return_type, typename input_types> // add more as needed
class CMFunction {
public:
  CMFunction(std::function<)
    : fallback_function(_fallback), normal_function(_normal) {}
  inline return_type Run(input_type1 i1, input_type2 i2, input_type3 i3, input_type4 i4, input_type5 i5 = void) {
    return (normal_function != nullptr) ? normal_function(i1, i2, i3, i4, i5) : fallback_function(i1, i2, i3, i4, i5);
  }
private:
  return_type(*normal_function)(input_type1, input_type2, input_type3, input_type4, input_type5) = nullptr;         // When run is called, this will be run if it isnt nullptr
  const std::function<return_type()> fallback_func; // if normal_function is nullptr, this is the fallback function to be run instead
};*/

/*class CMFunction <void> {
public:
  CMFunction() {}
  void Run
  void Fallback;
private:
  void function = nullptr;

};*/

/*class CMFunctionGroup {
public:
  void RunFunctions() {

  }
  void AddFunction()
}*/
