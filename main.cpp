#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>

using namespace std;

// Variant to hold either int or string
using Value = variant<int, string>;

struct Variable {
    string type;  // "int" or "string"
    Value value;
};

class ScopeManager {
private:
    // Stack of scopes, each scope is an unordered_map of variable name -> Variable
    vector<unordered_map<string, Variable>> scopes;
    // Cache for variable lookup: maps variable name to scope index
    unordered_map<string, vector<int>> variableScopes;

public:
    ScopeManager() {
        // Start with global scope
        scopes.push_back(unordered_map<string, Variable>());
        scopes.reserve(1000);  // Pre-allocate for more scopes
    }

    void indent() {
        scopes.push_back(unordered_map<string, Variable>());
    }

    void dedent() {
        if (scopes.size() > 1) {
            int scopeIdx = scopes.size() - 1;
            // Remove variables from cache
            for (auto& [name, var] : scopes.back()) {
                auto& vec = variableScopes[name];
                if (!vec.empty() && vec.back() == scopeIdx) {
                    vec.pop_back();
                    if (vec.empty()) {
                        variableScopes.erase(name);
                    }
                }
            }
            scopes.pop_back();
        }
    }

    // Declare a variable in the current scope
    bool declare(const string& type, const string& name, const Value& value) {
        // Check if variable already exists in current scope
        if (scopes.back().find(name) != scopes.back().end()) {
            return false;  // Already declared in this scope
        }
        scopes.back()[name] = {type, value};
        // Add to cache
        variableScopes[name].push_back(scopes.size() - 1);
        return true;
    }

    // Find variable in current or parent scopes (optimized with cache)
    Variable* findVariable(const string& name) {
        auto it = variableScopes.find(name);
        if (it == variableScopes.end() || it->second.empty()) {
            return nullptr;
        }
        // Get the most recent (innermost) scope index for this variable
        int scopeIdx = it->second.back();
        auto varIt = scopes[scopeIdx].find(name);
        if (varIt != scopes[scopeIdx].end()) {
            return &varIt->second;
        }
        return nullptr;
    }
};

// Parse string value (remove quotes)
// Returns empty string and sets valid to false if invalid
string parseStringValue(const string& s, bool& valid) {
    valid = false;
    if (s.length() >= 2 && s[0] == '"' && s[s.length()-1] == '"') {
        valid = true;
        return s.substr(1, s.length() - 2);
    }
    return "";
}

// Check if a string is a valid string literal (enclosed in quotes)
bool isValidStringLiteral(const string& s) {
    return s.length() >= 2 && s[0] == '"' && s[s.length()-1] == '"';
}

// Check if a variable name is valid (C++ identifier rules)
bool isValidVariableName(const string& name) {
    if (name.empty()) return false;

    // Must start with letter or underscore
    if (!isalpha(name[0]) && name[0] != '_') return false;

    // Rest must be alphanumeric or underscore
    for (size_t i = 1; i < name.length(); i++) {
        if (!isalnum(name[i]) && name[i] != '_') return false;
    }

    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const char* invalid_msg = "Invalid operation\n";

    int n;
    cin >> n;
    cin.ignore();  // Ignore newline after n

    ScopeManager manager;

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);

        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "Indent") {
            manager.indent();
        }
        else if (command == "Dedent") {
            manager.dedent();
        }
        else if (command == "Declare") {
            string type, name, valueStr;
            iss >> type >> name;

            // Validate variable name
            if (!isValidVariableName(name)) {
                cout << invalid_msg;
                continue;
            }

            if (type == "int") {
                int value;
                if (!(iss >> value)) {
                    cout << invalid_msg;
                    continue;
                }
                if (!manager.declare(type, name, value)) {
                    cout << invalid_msg;
                }
            }
            else if (type == "string") {
                // Read rest of line for string value
                string restOfLine;
                getline(iss, restOfLine);
                // Trim leading space
                size_t start = restOfLine.find_first_not_of(" \t");
                if (start != string::npos) {
                    valueStr = restOfLine.substr(start);
                } else {
                    cout << invalid_msg;
                    continue;
                }

                // Validate string literal
                if (!isValidStringLiteral(valueStr)) {
                    cout << invalid_msg;
                    continue;
                }

                bool valid;
                string value = parseStringValue(valueStr, valid);
                if (!valid || !manager.declare(type, name, value)) {
                    cout << invalid_msg;
                }
            }
            else {
                cout << invalid_msg;
            }
        }
        else if (command == "Print") {
            string name;
            iss >> name;

            Variable* var = manager.findVariable(name);
            if (var == nullptr) {
                cout << invalid_msg;
            }
            else {
                cout << name << ":";
                if (var->type == "int") {
                    cout << get<int>(var->value);
                }
                else {
                    cout << get<string>(var->value);
                }
                cout << '\n';
            }
        }
        else if (command == "SelfAdd") {
            string name;
            iss >> name;

            Variable* var = manager.findVariable(name);
            if (var == nullptr) {
                cout << invalid_msg;
                continue;
            }

            if (var->type == "int") {
                int addValue;
                if (!(iss >> addValue)) {
                    cout << invalid_msg;
                    continue;
                }
                var->value = get<int>(var->value) + addValue;
            }
            else if (var->type == "string") {
                string restOfLine;
                getline(iss, restOfLine);
                size_t start = restOfLine.find_first_not_of(" \t");
                if (start == string::npos) {
                    cout << invalid_msg;
                    continue;
                }
                string valueStr = restOfLine.substr(start);

                // Validate string literal
                if (!isValidStringLiteral(valueStr)) {
                    cout << invalid_msg;
                    continue;
                }

                bool valid;
                string addValue = parseStringValue(valueStr, valid);
                if (!valid) {
                    cout << invalid_msg;
                    continue;
                }
                // Append directly to avoid creating a new string
                get<string>(var->value) += addValue;
            }
        }
        else if (command == "Add") {
            string result, value1Name, value2Name;
            iss >> result >> value1Name >> value2Name;

            // Validate that all names are provided and valid
            if (result.empty() || value1Name.empty() || value2Name.empty()) {
                cout << invalid_msg;
                continue;
            }

            Variable* resultVar = manager.findVariable(result);
            Variable* value1Var = manager.findVariable(value1Name);
            Variable* value2Var = manager.findVariable(value2Name);

            if (resultVar == nullptr || value1Var == nullptr || value2Var == nullptr) {
                cout << invalid_msg;
                continue;
            }

            // Check that all three have the same type
            if (resultVar->type != value1Var->type || resultVar->type != value2Var->type) {
                cout << invalid_msg;
                continue;
            }

            if (resultVar->type == "int") {
                int val1 = get<int>(value1Var->value);
                int val2 = get<int>(value2Var->value);
                resultVar->value = val1 + val2;
            }
            else {
                // For strings, read values first to handle case where result is same as operand
                // Use move to avoid copying
                if (resultVar == value1Var) {
                    // result is value1, just append value2
                    get<string>(resultVar->value) += get<string>(value2Var->value);
                } else if (resultVar == value2Var) {
                    // result is value2, prepend value1
                    string temp = get<string>(value1Var->value);
                    temp += get<string>(resultVar->value);
                    resultVar->value = move(temp);
                } else {
                    // result is different from both, just concatenate
                    resultVar->value = get<string>(value1Var->value) + get<string>(value2Var->value);
                }
            }
        }
    }

    return 0;
}
