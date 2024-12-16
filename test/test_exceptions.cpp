#include "../src/exceptions/exceptions.h"

#include <iostream>

using namespace std;
using namespace soapstab;

int main() {
    try {
        throw ibb_exception(0, "Hi there");
    } catch(ibb_exception &e) {
        cout << "Caught: " << e.what() << endl;
    }
    cout << "Out" << endl;
}
