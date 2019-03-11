#include "ncmcrypt.h"

#include <stdexcept>

int main(int argc, char *argv[]) {

	if (argc <= 1) {
		std::cout << "please input file path!" << std::endl;
		return 1;
	}

    try {
        int i;
        for (i = 1; i < argc; i++) {
            ncmdump::NeteaseCrypt crypt(argv[i]);
            crypt.Dump();
            crypt.FixMetadata();
            std::cout << crypt.dumpFilepath() << std::endl;
        }
    } catch (const std::exception &e) {
        std::cout << "exception: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "unexpected exception!" << std::endl;
    }

    return 0;
}