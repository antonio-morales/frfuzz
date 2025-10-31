/* SPDX-License-Identifier: AGPL-3.0-only */
#include "elf.h"

bool ELF::open(std::filesystem::path input_file) {

    this->file_content = read_file(input_file);

    this->elf_addr = (void *)this->file_content.data(); // 0xd64800

    this->header = (ElfW(Ehdr) *)elf_addr;

    // ELF magic bytes = 0x7f, 'E', 'L', 'F'
    if (memcmp(header->e_ident, ELFMAG, SELFMAG) == 0) {

        // This is a valid elf file

        // https://ics.uci.edu/~aburtsev/238P/hw/hw3-elf/img/typical_elf.jpg

        sh = (Elf64_Shdr *)(elf_addr + header->e_shoff);
        this->num_sections = header->e_shnum;

        Elf64_Shdr *sh_str = &sh[header->e_shstrndx];
        this->section_names = (char *)(elf_addr + sh_str->sh_offset);

        for (int i = 0; i < num_sections; i++) {

            const char *section_name = section_names + sh[i].sh_name;

            sh_map[section_name] = (Elf64_Addr *)(&sh[i]);
        }

        Elf64_Shdr *sh_strings = (Elf64_Shdr *)(sh_map[".strtab"]);
        this->string_table = (char *)(elf_addr + sh_strings->sh_offset);

        Elf64_Shdr *sh_symbols = (Elf64_Shdr *)(sh_map[".symtab"]);
        this->symbols = (Elf64_Sym *)(elf_addr + sh_symbols->sh_offset);

        Elf64_Xword num_symbols = sh_symbols->sh_size / sh_symbols->sh_entsize;

        for (int i = 0; i < num_symbols; i++) {

            /*
            Elf64_Word	st_name;		// Symbol name (string tbl index)
            unsigned char	st_info;		// Symbol type and binding
            unsigned char st_other;		// Symbol visibility
            Elf64_Section	st_shndx;		// Section index
            Elf64_Addr	st_value;		// Symbol value
            Elf64_Xword	st_size;		// Symbol size
            */

            const char *symbol_name = string_table + symbols[i].st_name;

            symbols_map[symbol_name] = (Elf64_Addr *)(&symbols[i]);
        }

        // print_symbol("main");

        Elf64_Addr addr = get_symbol_addr("main");

        if (!sh_map.contains(".debug_info")) {
            std::cout << "No debug_info section" << std::endl;
            return false;
        }
        Elf64_Shdr *sh_debug_info = (Elf64_Shdr *)(sh_map[".debug_info"]);
        this->debug_info = (unsigned char *)(elf_addr + sh_debug_info->sh_offset);
        this->debug_info_size = sh_debug_info->sh_size;
        this->debug_info_end = debug_info + debug_info_size;

        if (!sh_map.contains(".debug_str")) {
            std::cout << "No debug_str section" << std::endl;
            return false;
        }
        Elf64_Shdr *sh_debug_str = (Elf64_Shdr *)(sh_map[".debug_str"]);
        this->debug_str = (unsigned char *)(elf_addr + sh_debug_str->sh_offset);

        if (!sh_map.contains(".debug_line_str")) {
            std::cout << "No debug_line_str section" << std::endl;
            return false;
        }
        Elf64_Shdr *sh_debug_line_str = (Elf64_Shdr *)(sh_map[".debug_line_str"]);
        this->debug_line_str = (char *)(elf_addr + sh_debug_line_str->sh_offset);

        if (!sh_map.contains(".debug_abbrev")) {
            std::cout << "No debug_abbrev section" << std::endl;
            return false;
        }
        Elf64_Shdr *sh_debug_abbrev = (Elf64_Shdr *)(sh_map[".debug_abbrev"]);
        this->debug_abbrev = (unsigned char *)(elf_addr + sh_debug_abbrev->sh_offset);
        this->debug_abbrev_size = sh_debug_abbrev->sh_size;
        this->debug_abbrev_end = debug_abbrev + debug_abbrev_size;

        /*
        for (const auto &section : sections) {

            if (section.type() == SHT_SYMTAB) {

                std::cout << "Symbol table" << std::endl;

                Elf64_Xword entry_size = section->sh_entsize;

                int num_symbols = section->sh_size / entry_size;

                std::cout << "Num:    Value          Size Type    Bind   Vis      Ndx Name" << std::endl;

                Elf64_Off string_table_offset = section_header_string->sh_offset;
                const char *string_table = (char *)(elf_addr + string_table_offset);

                for (int s = 0; s < num_symbols; s++) {

                    // Elf64_Off string_table_offset = section_header_string->sh_offset;
                    // const char *string_table = (char *)(elf_addr + string_table_offset);

                    Elf64_Sym *symPtr = (Elf64_Sym *)((char *)(elf_addr + section->sh_offset + s * entry_size));


                    //unsigned char	st_other;	// No defined meaning, 0
                    //Elf64_Half st_shndx;		// Associated section index


                    const char *sym_name = (char *)(string_table + symPtr->st_name);

                    unsigned char bind = ELF64_ST_BIND(symPtr->st_info); // Symbol's binding
                    unsigned char type = ELF64_ST_TYPE(symPtr->st_info); // Symbol's type

                    Elf64_Xword sym_size = symPtr->st_size; // Size of object (e.g., common)

                    Elf64_Addr sym_value = symPtr->st_value; // Value of the symbol

                    Symbol symbol(
                        sym_name,
                        sym_value,
                        sym_size,
                        bind,
                        type);

                    std::cout << symbol << std::endl;
                }

                std::cout << "Debug" << std::endl;
            }
        }

        */

        // sections = (Elf32_Shdr *)((char *)map_start + header.e_shoff);

        Elf64_Off sections = header->e_shoff;
    }
}

// String table

/*
int32_t read_elf_header(const char *elfFile, ElfW(Ehdr) & header) {

    Elf64_Ehdr *map_start = nullptr;

}
*/

Elf64_Addr ELF::get_symbol_addr(std::string symbol_name) {

    if (!symbols_map.contains(symbol_name)) {
        std::cout << "Symbol not found" << std::endl;
        return 0;
    }

    Elf64_Sym *symbol = (Elf64_Sym *)symbols_map[symbol_name];

    return symbol->st_value;
}

void ELF::print_symbol(std::string symbol_name) {

    if (!symbols_map.contains(symbol_name)) {
        std::cout << "Symbol not found" << std::endl;
        return;
    }

    Elf64_Sym *symbol = (Elf64_Sym *)symbols_map[symbol_name];

    std::cout << std::hex << std::setw(16) << std::setfill('0') << symbol->st_value << " " << std::dec << symbol->st_size;
    std::cout << " " << (int)symbol->st_info << " " << (int)symbol->st_other << " " << symbol->st_shndx << " ";
    std::cout << symbol_name << std::endl;

    // symTable[i].st_name;
}
