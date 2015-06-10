#include <iostream>
#include <elfio/elfio.hpp>
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
    map<string, int> modSegStr;
    for (int i = 0; i < modSecNum; ++i) {
        section* psec = modReader.sections[i];
        modSegStr[string(psec->get_name())] = i;
        if (origSegStr.count(string(psec->get_name())) &&
            strstr(psec->get_name().c_str(), ".text") == psec->get_name().c_str()) {
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
    patchWriter.set_machine( EM_386 );

    for (int i = 0; i < modSecNum; ++i) {
        if (modSegsMark[i] == true) {
            section* modPsec = modReader.sections[i];
            section* patchPsec = patchWriter.sections.add(modPsec->get_name());
            patchPsec->set_type(modPsec->get_type());
            patchPsec->set_flags(modPsec->get_flags());
            patchPsec->set_info(modPsec->get_info());
            patchPsec->set_link(modPsec->get_link());
            patchPsec->set_addr_align(modPsec->get_addr_align());
            patchPsec->set_entry_size(modPsec->get_entry_size());
            patchPsec->set_name_string_offset(modPsec->get_name_string_offset());
            patchPsec->set_data(modPsec->get_data(), modPsec->get_size());
        }
    }
//
//    section *sym_sec = patchWriter.sections.add(".symtab");
//    section *mod_sym_sec = modReader.sections[modSegStr[".symtab"]];
//    sym_sec->set_type(mod_sym_sec->get_type());
//    sym_sec->set_info(mod_sym_sec->get_info());
//    sym_sec->set_addr_align(mod_sym_sec->get_addr_align());
//    sym_sec->set_entry_size(mod_sym_sec->get_entry_size());
//    sym_sec->set_link(mod_sym_sec->get_link());
//    sym_sec->set_data(mod_sym_sec->get_data(), mod_sym_sec->get_size());
//
//    section *str_sec = patchWriter.sections.add(".strtab");
//    section *mod_str_sec = modReader.sections[modSegStr[".strtab"]];
//    str_sec->set_type(mod_str_sec->get_type());
//    str_sec->set_info(mod_str_sec->get_info());
//    str_sec->set_addr_align(mod_str_sec->get_addr_align());
//    str_sec->set_entry_size(mod_str_sec->get_entry_size());
//    str_sec->set_link(mod_str_sec->get_link());
//    str_sec->set_data(mod_str_sec->get_data(), mod_str_sec->get_size());
//
//    section *shstr_sec = patchWriter.sections.add(".shstrtab");
//    section *mod_shstr_sec = modReader.sections[modSegStr[".shstrtab"]];
//    shstr_sec->set_type(mod_shstr_sec->get_type());
//    shstr_sec->set_info(mod_shstr_sec->get_info());
//    shstr_sec->set_addr_align(mod_shstr_sec->get_addr_align());
//    shstr_sec->set_entry_size(mod_shstr_sec->get_entry_size());
//    shstr_sec->set_link(mod_shstr_sec->get_link());
//    shstr_sec->set_data(mod_shstr_sec->get_data(), mod_shstr_sec->get_size());

    patchWriter.save(argv[3]);
}