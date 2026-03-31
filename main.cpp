#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <cctype>

using namespace std;

// Basic Scope language interpreter
// This is a minimal implementation that will be refined based on OJ feedback

class ScopeInterpreter {
private:
    stack<map<string, int>> scopes;

public:
    ScopeInterpreter() {
        // Initialize with global scope
        scopes.push(map<string, int>());
    }

    void enterScope() {
        scopes.push(map<string, int>());
    }

    void exitScope() {
        if (scopes.size() > 1) {
            scopes.pop();
        }
    }

    void setVariable(const string& name, int value) {
        scopes.top()[name] = value;
    }

    int getVariable(const string& name) {
        // Search from current scope to outer scopes
        stack<map<string, int>> temp;
        int result = 0;
        bool found = false;

        while (!scopes.empty()) {
            map<string, int>& current = scopes.top();
            if (current.find(name) != current.end()) {
                result = current[name];
                found = true;
                break;
            }
            temp.push(current);
            scopes.pop();
        }

        // Restore stack
        while (!temp.empty()) {
            scopes.push(temp.top());
            temp.pop();
        }

        return found ? result : 0;
    }

    void run() {
        string line;
        while (getline(cin, line)) {
            if (line.empty()) continue;

            // Parse and execute commands
            istringstream iss(line);
            string cmd;
            iss >> cmd;

            if (cmd == "{") {
                enterScope();
            } else if (cmd == "}") {
                exitScope();
            } else if (cmd == "print") {
                string var;
                iss >> var;
                cout << getVariable(var) << endl;
            } else {
                // Assignment: var = value
                string var = cmd;
                string eq;
                int value;
                iss >> eq >> value;
                if (eq == "=") {
                    setVariable(var, value);
                }
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ScopeInterpreter interpreter;
    interpreter.run();

    return 0;
}
