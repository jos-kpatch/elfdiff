#include <iostream>
#include <elfio/elfio.hpp>
#include <string>
#include <map>

using namespace std;
using namespace ELFIO;

int main(int argc, char** argv) {
    if (argc != 4) {
        cerr << "Usage: elfdiff original.o modified.o patch.o" << endl;
    }

    elfio origReader, modReader;
    if (!origReader.load(argv[1])) {
        cerr << "cannot load " << argv[1] << endl;
        return -1;
    }
    if (!modReader.load(argv[2])) {
        cerr << "cannot load " << argv[2] << endl;
        return -1;
    }

    if (!(origReader.get_class() == ELFCLASS32 && modReader.get_class() == ELFCLASS32)) {
        cerr << "not 32-bit elf file" << endl;
        return -1;
    }

    Elf_Half origSecNum = origReader.sections.size();
    Elf_Half modSecNum = modReader.sections.size();

    map<string, int> origSegStr;
    for (int i = 0; i < origSecNum; ++i) {
        section* psec = origReader.sections[i];
        origSegStr[string(psec->get_name())] = i;
    }

    map<int, int> origModMap;
    for (int i = 0; i < modSecNum; ++i) {
        section* psec = modReader.sections[i];
        if (origSegStr.count(string(psec->get_name()))) {
            origModMap[i] = origSegStr[string(psec->get_name())];
        } else {
            origModMap[i] = -1;
        }
    }

    vector<bool> modSegsMark(modSecNum);
    for (int i = 0; i < modSecNum; ++i) {
        if (origModMap[i] == -1) {
            modSegsMark[i] = true;
        } else {
            section* modPsec = modReader.sections[i];
            section* origPsec = origReader.sections[origModMap[i]];
            if (modPsec->get_size() != origPsec->get_size()) {
                modSegsMark[i] = true;
            } else if (memcmp(modPsec->get_data(), origPsec->get_data(), modPsec->get_size())) {
                modSegsMark[i] = true;
            } else {
                modSegsMark[i] = false;
            }
        }
    }

    elfio patchWriter;

    patchWriter.create( ELFCLASS32, ELFDATA2LSB );

    for (int i = 0; i < modSecNum; ++i) {
        if (modSegsMark[i] == true) {
            section* modPsec = modReader.sections[i];
            section* patchPsec = patchWriter.sections.add(modPsec->get_name());
            patchPsec->set_type(modPsec->get_type());
            patchPsec->set_flags(modPsec->get_flags());
            patchPsec->set_addr_align(modPsec->get_addr_align());
            patchPsec->set_data(modPsec->get_data(), modPsec->get_size());
        }
    }

    patchWriter.save(argv[3]);
}