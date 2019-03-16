#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <ctime>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;

// A strucutre that matches a layout in a json file. When reading this type
// of json, its values are loaded into an insteance of this struct.
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

  // Read from the file
  FILE *fp = fopen(filename, "r");
  if (!fp)
    return nullptr;

  // Get the length of the file
  fseek(fp, 0, SEEK_END);
  const size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char* buffer = (char*)malloc(size);
  FileReadStream frs(fp, buffer, size);

  fclose(fp);

  // Load the JSON into the struct
  Document j;
  j.Parse(buffer);

  TestJSON *test = (TestJSON*)malloc(sizeof(TestJSON));

  test->pi = j["pi"].GetDouble();
  test->happy = j["happy"].GetBool();

  // Note(Dustin): GetStringLength and strlen() can return different values, but I am not sure which one is more
  // reliable quite yet
  size_t name_len = j["name"].GetStringLength();
  test->name = (char*)malloc(name_len + 1);
  strncpy(test->name, j["name"].GetString(), name_len);
  test->name[name_len] = '\0';
  
  test->answer.everything = j["answer"]["everything"].GetInt();

  test->size = j["list"].Size();
  test->list = (int*)malloc(sizeof(int) *  test->size);
  for (int i = 0; i <  test->size; ++i)
  {
    test->list[i] = j["list"][i].GetInt();
  }

  size_t curr_len = j["object"]["currency"].GetStringLength();
  test->object.currency = (char*)malloc(curr_len + 1);
  strncpy(test->object.currency, j["object"]["currency"].GetString(), curr_len);
  test->object.currency[curr_len] = '\0';

  test->object.value = j["object"]["value"].GetDouble();

  free(buffer);

  return test;
}

void SaveTestJSON(char* filename, TestJSON* test)
{
  // Write the struct to the json
  Document j;
  j.SetObject();
  Document::AllocatorType& allocator = j.GetAllocator();

  j.AddMember("pi", Value().SetDouble(test->pi), allocator);
  j.AddMember("happy", Value().SetBool(test->happy), allocator);
  j.AddMember("name", StringRef(test->name), allocator);

  Value answer(kObjectType);
  j.AddMember("answer", answer, allocator);
  j["answer"].AddMember("everything", Value().SetInt(test->answer.everything), allocator);

  Value list(kArrayType);
  for (int i = 0; i < test->size; ++i)
  {
    list.PushBack(Value().SetInt(test->list[i]), allocator);
  }
  j.AddMember("list", list, allocator);


  Value object(kObjectType);
  j.AddMember("object", object, allocator);
  j["object"].AddMember("currency", StringRef(test->object.currency), allocator);
  j["object"].AddMember("value", test->object.value, allocator);

  
  // Write to the specified file
  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  j.Accept(writer);

  FILE* fp = fopen(filename, "w+");
  if (!fp)
  {
    printf("Failed to write to file %s!\n", filename);
  }
  fputs(buffer.GetString(), fp);
  fclose(fp);
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

  clock_t a = clock();
  TestJSON* test = LoadTestJSON("../../json/test.json");
  clock_t b = clock();
  std::cout << "Read time: " << b - a << std::endl;

  // Make some changes to the json data



  // Create new json and write new data to it
  a = clock();
  SaveTestJSON("../../json/test.json", test);
  b = clock();
  std::cout << "Write time: " << b - a << std::endl;

  FreeTestJSON(test);

  return (0);
}