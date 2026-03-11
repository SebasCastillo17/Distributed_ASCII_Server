#include <map>
#include <string>

class ColabProtocolHandler{
public:
  ColabProtocolHandler();
  ~ColabProtocolHandler() = default;

  std::string buildAnswer(std::string request);

 private:
  std::string deleteFlags(std::string request);
  int determOperation(std::string request);
  std::string onCase(std::string);
  std::string offCase(std::string);
  std::string objCase();
  std::string okCase(std::string);
  std::string errorCase(std::string);
  std::string getCase(std::string);

 private:
  std::string nameList = "CocaCola.txt\nExitante.txt\nFoca.txt\nGogeta.txt\n"
                          "Goku.txt\nGoyoCat.txt\nPersonal.txt\n";
  std::map<std::string, int> OPP = {
      {"ON", 1},
      {"OFF", 2},
      {"OK", 3},
      {"ERROR", 4},
      {"GET", 5}};
};