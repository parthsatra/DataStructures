#include <iostream>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
    if(argc == 3) {
        char *index_file_name = argv[1];
        int order = atoi(argv[2]);
        cout << "Infex file name: " << index_file_name << endl;
        cout << "Order: " << order << endl;

        while(true) {
            string temp;
            getline(cin, temp);

            istringstream iss(temp);
            string command;
            string key;
            iss >> command;
            if(!strcmp("add", command.c_str())) {
                iss >> key;
                cout << "Add: " << key << endl;
            } else if(!strcmp("find", command.c_str())) {
                iss >> key;
                cout << "Find: " << key << endl;
            } else if(!strcmp("print", command.c_str())) {
                cout << "Print" << endl;
            } else if(!strcmp("end", command.c_str())) {
                cout << "End" << endl;
                break;
            }
        }
    } else {
        cout << "Invalid number of parameters passed" << endl;
        exit(1);
    }
    return 0;
}