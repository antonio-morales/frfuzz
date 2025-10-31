/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <elf.h>

#include "utils/filesys.h"
#include "utils/utils.h"

#if defined(__LP64__)
#define ElfW(type) Elf64_##type
#else
#define ElfW(type) Elf32_##type
#endif

inline const std::unordered_map<int, std::string> section_header_types = {{SHT_NULL, "SHT_NULL (Section header table entry unused)"},
                                                                          {SHT_PROGBITS, "SHT_PROGBITS (Program data)"},
                                                                          {SHT_SYMTAB, "SHT_SYMTAB (Symbol table)"},
                                                                          {SHT_STRTAB, "SHT_STRTAB (String table)"},
                                                                          {SHT_RELA, "SHT_RELA (Relocation entries with addends)"},
                                                                          {SHT_HASH, "SHT_HASH (Symbol hash table)"},
                                                                          {SHT_DYNAMIC, "SHT_DYNAMIC (Dynamic linking information)"},
                                                                          {SHT_NOTE, "SHT_NOTE (Notes)"},
                                                                          {SHT_NOBITS, "SHT_NOBITS (Program space with no data (bss))"},
                                                                          {SHT_REL, "SHT_REL (Relocation entries, no addends)"},
                                                                          {SHT_SHLIB, "SHT_SHLIB (Reserved)"},
                                                                          {SHT_DYNSYM, "SHT_DYNSYM (Dynamic linker symbol table)"},
                                                                          {SHT_INIT_ARRAY, "SHT_INIT_ARRAY (Array of constructors)"},
                                                                          {SHT_FINI_ARRAY, "SHT_FINI_ARRAY (Array of destructors)"},
                                                                          {SHT_PREINIT_ARRAY, "SHT_PREINIT_ARRAY (Array of pre-constructors)"},
                                                                          {SHT_GROUP, "SHT_GROUP (Section group)"},
                                                                          {SHT_SYMTAB_SHNDX, "SHT_SYMTAB_SHNDX (Extended section indeces)"},
                                                                          {SHT_NUM, "SHT_NUM (Number of defined types.)"},
                                                                          {SHT_LOOS, "SHT_LOOS (Start OS-specific.)"},
                                                                          {SHT_GNU_ATTRIBUTES, "SHT_GNU_ATTRIBUTES (Object attributes)"},
                                                                          {SHT_GNU_HASH, "SHT_GNU_HASH (GNU-style hash table)"},
                                                                          {SHT_GNU_LIBLIST, "SHT_GNU_LIBLIST (Prelink library list)"},
                                                                          {SHT_CHECKSUM, "SHT_CHECKSUM (Checksum for DSO content.)"},
                                                                          {SHT_LOSUNW, "SHT_LOSUNW (Sun-specific low bound.)"},
                                                                          {SHT_SUNW_move, "SHT_SUNW_move (Sun-specific low bound.)"},
                                                                          {SHT_SUNW_COMDAT, "SHT_SUNW_COMDAT (Sun-specific low bound.)"},
                                                                          {SHT_SUNW_syminfo, "SHT_SUNW_syminfo (Sun-specific low bound.)"},
                                                                          {SHT_GNU_verdef, "SHT_GNU_verdef (Version definition section.)"},
                                                                          {SHT_GNU_verneed, "SHT_GNU_verneed (Version needs section.)"},
                                                                          {SHT_GNU_versym, "SHT_GNU_versym (Version symbol table.)"},
                                                                          {SHT_LOPROC, "SHT_LOPROC (Start of processor-specific.)"},
                                                                          {SHT_HIPROC, "SHT_HIPROC (End of processor-specific.)"},
                                                                          {SHT_LOUSER, "SHT_LOUSER (Start of application-specific.)"},
                                                                          {SHT_HIUSER, "SHT_HIUSER (End of application-specific.)"}};

inline const std::unordered_map<unsigned char, std::string> symbol_binding = {{STB_LOCAL, "LOCAL (Local symbol)"},
                                                                              {STB_GLOBAL, "GLOBAL (Global symbol)"},
                                                                              {STB_WEAK, "WEAK (Weak symbol)"},
                                                                              {STB_NUM, "NUM (Number of defined types.)"},
                                                                              {STB_LOOS, "LOOS (Start OS-specific.)"},
                                                                              {STB_GNU_UNIQUE, "GNU_UNIQUE (Unique symbol.)"},
                                                                              {STB_HIOS, "HIOS (End of OS-specific.)"},
                                                                              {STB_LOPROC, "LOPROC (Start of processor-specific.)"},
                                                                              {STB_HIPROC, "HIPROC (End of processor-specific.)"}};

inline const std::unordered_map<unsigned char, std::string> symbol_type = {{STT_NOTYPE, "NOTYPE (Symbol type is unspecified)"},
                                                                           {STT_OBJECT, "OBJECT (Symbol is a data object)"},
                                                                           {STT_FUNC, "FUNC (Symbol is a code object)"},
                                                                           {STT_SECTION, "SECTION (Symbol associated with a section)"},
                                                                           {STT_FILE, "FILE (Symbol's name is file name)"},
                                                                           {STT_COMMON, "COMMON (Symbol is a common data object)"},
                                                                           {STT_TLS, "TLS (Symbol is thread-local data object)"},
                                                                           {STT_NUM, "NUM (Number of defined types.)"},
                                                                           {STT_LOOS, "LOOS (Start OS-specific.)"},
                                                                           {STT_GNU_IFUNC, "GNU_IFUNC (Symbol is indirect code object)"},
                                                                           {STT_HIOS, "HIOS (End OS-specific.)"},
                                                                           {STT_LOPROC, "LOPROC (Start of processor-specific.)"},
                                                                           {STT_HIPROC, "HIPROC (End of processor-specific.)"}};

class ELF {

    friend class DWARF;

  public:
    ELF() {}

    bool open(std::filesystem::path input_file);

    // bool close();

    // std::string symbol_table(std::string symbol);

    Elf64_Addr get_symbol_addr(std::string symbol_name);

    void print_symbol(std::string symbol_name);

  private:
    std::filesystem::path file_path;

    std::string file_content;

    void *elf_addr;

    ElfW(Ehdr) * header;

    Elf64_Shdr *sh;
    Elf64_Half num_sections;

    const char *section_names;
    const char *string_table;

    const unsigned char *debug_info;
    size_t debug_info_size;
    const unsigned char *debug_info_end;

    const unsigned char *debug_str;
    const char *debug_line_str;

    const unsigned char *debug_abbrev;
    size_t debug_abbrev_size;
    const unsigned char *debug_abbrev_end;

    // Section name, section address
    std::map<std::string, Elf64_Addr *> sh_map;

    Elf64_Sym *symbols;

    // Symbol name, symbol address
    std::map<std::string, Elf64_Addr *> symbols_map;
};

int32_t read_elf_header(const char *elfFile, ElfW(Ehdr) & header);
