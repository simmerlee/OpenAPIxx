#include "OA_ArgParser.h"
#include <iostream>

using namespace std;
void printResult(int ret, OpenAPIxx::ArgMap& args, string& msg) {
    cout << "ret: " << ret << " msg: " << msg << " args: ";
    if (args.size() == 0) {
        cout << "empty!" << endl;
        return;
    }
    for (auto it = args.begin(); it != args.end(); it++) {
        cout << "(" << it->first << " " << it->second << ") ";
    }
    cout << endl;
}

int main() {
    char* args1[] = { "test", "-a", "aaa", "-b", "bbb", "--abc", "456", "--qwe", "789" };
    char* args2[] = { "test", "-a", "aaa", "-b", "bbb" };
    OAArgParser ap;
    ap.setArg("-a", true, false, "", "disp -a", "NAME");
    ap.setArg("-b", true, false, "", "disp -b", "NAME");
    ap.setArg("-c", true, true, "default_c", "disp -c", "NAME");
    ap.setArg("-d", false, true, "", "disp -c", "FLAG");

    OpenAPIxx::ArgMap out_args;
    std::string msg;
    int ret = 0;
    ret = ap.parseArg(9, (const char**)args1, out_args, msg);
    printResult(ret, out_args, msg);
    ret = ap.parseArg(5, (const char**)args2, out_args, msg);
    printResult(ret, out_args, msg);

    return 0;
}