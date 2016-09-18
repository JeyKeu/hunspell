#include <string>
#include <vector>
#include <istream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include <cctype>

#include <iterator>

namespace hunspell {

/** \brief Low level parser for .AFF file.
 *
 * Parses the .aff file  where each line is in the format
 * COMMAND [PARAMETAR_LINE].
 * Since one command can appear multiple times, for each distinct command
 * the class stores a vector of parameter lines in the ordered as in the file.
 * All commands are stored in uppercase. Querying should be in uppercase.
 * The parameters are unchanged.
 *
 * E.g.
 *
 * SFX A Y 2
 * SFX A abc qwe .
 * SFX A zxc abc .
 *
 * The above will be stored as
 * "SFX" -> {"A Y 2", "A abc qwe .", "A zxc abc ."}
 */
class aff_parser {
private:
  std::unordered_map<std::string, std::vector<std::string>> table;
  const std::vector<std::string> empty_vec;
public:

  /** \brief Clears the data in the parser
   */
  void clear();


  /** \brief Parses an aff file
   *
   * \param in Input stream of the aff file.
   * \return true on success, false on failiure
   *
   */
  bool parse(std::istream& in);


  /** \brief Checks if a command was present in the file.
   *
   * \param command in all uppercase
   * \return true if the command exists, false otherwise
   *
   */
  bool is_command_present(const std::string& command);

  /** \brief Gets the command vector of parameter lines.
   *
   * If the command does not exists, empty vector is returned. If the command
   * exists, but it has no parameters, also empty vector is returned.
   *
   * \param command in all uppercase
   * \return vector of all parametar lines
   *
   */
  const std::vector<std::string>& get_command_parametars(const std::string& command);

  const std::unordered_map<std::string, std::vector<std::string>>& data();
};

}

using namespace std;

namespace hunspell {

void aff_parser::clear() {
  table.clear();
}

bool aff_parser::parse(std::istream& in)
{
  string line;
  string command;
  string parametar_line;
  do {
    getline(in, line);
    if (in.fail() && !in.eof()) {
      clear();
      return false;
    }
    stringstream ss(line);
    ss >> ws;
    if (ss.eof() || ss.peek() == '#') {
        continue; //skip comment or empty lines
    }
    ss >> command;
    for (auto & c: command) c = toupper(c);
    auto& vec = table[command];
    ss >> ws;
    if (ss.eof()) {
        continue; //nothing more to read
    }
    getline(ss, parametar_line);
    vec.push_back(parametar_line);
  } while (!in.eof());

  return true;
}

bool aff_parser::is_command_present(const std::string& command)
{
  return table.count(command);
}

const std::vector<std::string>& aff_parser::get_command_parametars
  (const std::string& command)
{
  auto it = table.find(command);
  if (it != table.end()) {
    return it->second;
  } else {
    return empty_vec;
  }
}

const std::unordered_map<std::string, std::vector<std::string>>&
aff_parser::data()
{
  return table;
}

}

int main() {
  string test =
    "SET UTF-8\n"
    "\n"
    "TRY abcdef \n"
    "\n"
    "SFX A Y 2\n"
    "#comment1\n"
    "SFX A abc qwe .\n"
    "  #comment2\n"
    "  sfx A zxc abc .\n"
    "  COMPLEXPREFIXES  \n"
    "lang hu_HU #this is not comment. It's part of the parameter";
  stringstream ss(test);
  hunspell::aff_parser p;
  p.parse(ss);

  for(auto& key_value : p.data()) {
    cout << key_value.first << ':';
    std::ostream_iterator<string> out_it (std::cout,", ");
    std::copy ( key_value.second.begin(), key_value.second.end(), out_it );
    cout << endl;
  }
}
