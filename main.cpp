#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>

using namespace std;

// Variant to hold either long long or string
using Value = variant<long long, string>;

struct Variable {
    string type;  // "int" or "string"
    Value value;
};

class ScopeManager {
private:
    // Stack of scopes, each scope is an unordered_map of variable name -> Variable
    vector<unordered_map<string, Variable>> scopes;
    // Map from variable name to the scope level where it's most recently defined
    unordered_map<string, int> lastSeenScope;

public:
    ScopeManager() {
        // Start with global scope
        scopes.push_back(unordered_map<string, Variable>());
        scopes.reserve(1000);  // Pre-allocate for more scopes
    }

    void indent() {
        scopes.push_back(unordered_map<string, Variable>());
        scopes.back().reserve(100);  // Pre-allocate
    }

    void dedent() {
        if (scopes.size() > 1) {
            int scopeIdx = scopes.size() - 1;
            // Update cache for variables that were in this scope
            for (auto& [name, var] : scopes.back()) {
                // Remove from cache
                lastSeenScope.erase(name);
                // Find if it exists in outer scopes and update cache
                for (int i = scopeIdx - 1; i >= 0; i--) {
                    if (scopes[i].find(name) != scopes[i].end()) {
                        lastSeenScope[name] = i;
                        break;
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
        // Update cache
        lastSeenScope[name] = scopes.size() - 1;
        return true;
    }

    // Find variable in current or parent scopes (optimized)
    Variable* findVariable(const string& name) {
        auto cacheIt = lastSeenScope.find(name);
        if (cacheIt != lastSeenScope.end()) {
            int scopeIdx = cacheIt->second;
            if (scopeIdx < scopes.size()) {
                auto it = scopes[scopeIdx].find(name);
                if (it != scopes[scopeIdx].end()) {
                    return &it->second;
                }
            }
        }
        // Fallback to linear search (shouldn't happen with correct cache)
        for (int i = scopes.size() - 1; i >= 0; i--) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) {
                lastSeenScope[name] = i;  // Update cache
                return &it->second;
            }
        }
        return nullptr;  // Not found
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

            if (type == "int") {
                long long value;
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
                    cout << get<long long>(var->value);
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
                long long addValue;
                if (!(iss >> addValue)) {
                    cout << invalid_msg;
                    continue;
                }
                var->value = get<long long>(var->value) + addValue;
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
                long long val1 = get<long long>(value1Var->value);
                long long val2 = get<long long>(value2Var->value);
                resultVar->value = val1 + val2;
            }
            else {
                // For strings, always read both values first
                string val1 = get<string>(value1Var->value);
                string val2 = get<string>(value2Var->value);
                resultVar->value = val1 + val2;
            }
        }
    }

    return 0;
}
