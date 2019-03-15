#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string.h>
#include <jsonhpp/json.hpp>

using json = nlohmann::json;

/*
  "pi": 3.141,
  "happy": true,
  "name": "Niels",
  "nothing": null,
  "answer": {
    "everything": 42
  },
  "list": [1, 0, 2],
  "object": {
    "currency": "USD",
    "value": 42.99
  }
*/

struct TestJSON {
  double pi;
  bool happy;
  char* name;
  //double nothing;

  struct Answer {
    int everything;
  } answer;
  
  int* list;
  int size;
  struct Object {
    char* currency;
    double value;
  } object;

};

TestJSON* LoadTestJSON(char* filename)
{
  std::ifstream i(filename);
  json j;
  i >> j;

  TestJSON *test = (TestJSON*)malloc(sizeof(TestJSON));

  test->pi = j["pi"];
  test->happy = j["happy"];

  // We are probably going to want to avoid using strings as much as possible with this
  std::string name = j["name"];
  size_t len = name.size();
  test->name = (char*)malloc(len);
  strncpy(test->name, name.data(), len);
  
  //test.nothing = j["nothing"];
  test->answer.everything = j["answer"]["everything"];

  std::vector<int> list = j["list"];
  test->size = (int)list.size();
  test->list = (int*)malloc(sizeof(int) *  test->size);
  for (int i = 0; i <  test->size; ++i)
  {
    test->list[i] = list[i];
  }
  list.resize(0);

  // We are probably going to want to avoid using strings as much as possible with this
  std::string object_currency = j["object"]["currency"];
  size_t oc_len = object_currency.size();
  test->object.currency = (char*)malloc(oc_len);
  strncpy(test->object.currency, object_currency.data(), oc_len);

  test->object.value = j["object"]["value"];

  return test;
}

void SaveTestJSON(char* filename, TestJSON* test)
{

  // Fill in the JSON data to write
  json j;
  j["pi"] = test->pi;
  j["happy"] = test->happy;
  j["name"] = std::string(test->name);
  j["nothing"] = nullptr;
  j["answer"]["everything"] = test->answer.everything;

  // Another downside: it doesn't handle pointers too well: have to convert this list
  // to a std::vector
  //int size = sizeof(test->list) / sizeof(test->list[0]);
  std::vector<int> list;
  for (int i = 0; i < test->size; ++i)
  {
    list.push_back(test->list[i]);
  }
  j["list"] = list;

  j["object"] = { {"currency", std::string(test->object.currency)}, {"value", test->object.value} };

  // write to a json file with new json
  std::ofstream o(filename);
  o << std::setw(2) << j << std::endl;
}

void FreeTestJSON(TestJSON* test)
{
  free(test->name);
  free(test->list);
  free(test->object.currency);
  free(test);
}

int main(void)
{

  // read from a json file
  TestJSON* test = LoadTestJSON("../../json/test.json");
  

  // Make some changes to the json data



  // Create new json and write new data to it



  
  SaveTestJSON("../../json/test_out.json", test);

  FreeTestJSON(test);

  return (0);
}