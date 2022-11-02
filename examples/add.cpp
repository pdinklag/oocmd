#include <oocmd.hpp>

using namespace oocmd;

struct Addition : public ConfigObject {
   	int a = 0;
    int b = 0;
    
    Addition() : ConfigObject("Addition", "Adds two numbers and prints the result") {
        param('a', "first",  a, "The first summand.");
        param('b', "second", b, "The second summand.");
    }

    int run(Application const& app) {
        int sum = a + b;
        std::cout << sum << std::endl;
        return 0;
    }
};

int main(int argc, char** argv) {
    Addition add;
    return Application::run(add, argc, argv);
}
