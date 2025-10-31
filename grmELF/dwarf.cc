/* SPDX-License-Identifier: AGPL-3.0-only */
#include "dwarf.h"

// Return the relative offset of the given ptr
Elf32_Off DWARF_shared_info::rel_offset(const unsigned char *ptr) {

    if (ptr > debug_info && ptr < debug_info_end) {
        return ptr - debug_info;

    } else if (ptr > debug_abbrev && ptr < debug_abbrev_end) {
        return ptr - debug_abbrev;

    } else {
        return 0;
    }
}

DWARF::DWARF(const ELF *elf_instance) : elf(elf_instance) {

    sInfo.debug_info = elf_instance->debug_info;
    sInfo.debug_info_size = elf_instance->debug_info_size;
    sInfo.debug_info_end = elf_instance->debug_info_end;

    sInfo.debug_str = elf_instance->debug_str;
    sInfo.debug_line_str = elf_instance->debug_line_str;

    sInfo.debug_abbrev = elf_instance->debug_abbrev;
    sInfo.debug_abbrev_size = elf_instance->debug_abbrev_size;
    sInfo.debug_abbrev_end = elf_instance->debug_abbrev_end;

    sInfo.compilation_units = &compilation_units;

    sInfo.DI_entries = &DI_entries;

    sInfo.attributes = &attributes;

    load_abbrev_tables();

    load_compilation_units();
}

std::vector<DWARF_DI_entry> DWARF::search_by_address(Elf64_Addr address) {

    if (!abbrev_tables_loaded) {
        load_abbrev_tables();
    }

    if (!compilation_units_loaded) {
        load_compilation_units();
    }

    std::vector<DWARF_DI_entry> entries;

    size_t num_values = sInfo.low_pc_map.count(address);

    auto it = sInfo.low_pc_map.find(address);

    for (int i = 0; i < num_values; i++) {

        // DWARF_DI_entry entry = addr_to__DWARF_DI_entry(it->second);

        DWARF_attr attr = attributes[it->second];

        DWARF_DI_entry entry = attr.parent();

        // entries.push_back(DI_entries[it->second]);

        entries.push_back(entry);

        it++;
    }

    // Look for DW_AT_low_pc

    return entries;
}

bool DWARF::load_abbrev_tables(bool print) {

    const unsigned char *abbrev_tables_end = sInfo.debug_abbrev + sInfo.debug_abbrev_size;

    unsigned char *ptr = (unsigned char *)sInfo.debug_abbrev;

    while (ptr < abbrev_tables_end) {

        Dwarf32_Off offset = ptr - sInfo.debug_abbrev;

        Dwarf_Abbrev_table *abbrev_table = new Dwarf_Abbrev_table();

        abbrev_table->push_back({0, DW_NULL, DW_CHILDREN::DW_CHILDREN_no, {}, 0}); // NULL decl

        while (1) {

            DWARF_abbrev_decl decl;

            unsigned n;

            decl.abbrev_code = decodeULEB128(ptr, &n);

            if (decl.abbrev_code == 0) { // End of table

                if (print) {
                    std::cout << "table size = " << abbrev_table->size() << std::endl;
                }

                ptr++;
                break;
            }

            ptr += n;

            decl.dw_tag = static_cast<DW_TAG>(decodeULEB128(ptr, &n));

            ptr += n;

            decl.dw_children = static_cast<DW_CHILDREN>(*ptr);

            ptr++;

            // std::cout << "code: " << std::dec << code << std::endl;

            DWARF_attr_spec spec;

            while (1) { // TODO: Check memory bounds

                spec.DW_AT_at = static_cast<DW_AT>(decodeULEB128(ptr, &n));

                ptr += n;

                spec.DW_FORM_form = static_cast<DW_FORM>(decodeULEB128(ptr, &n));

                decl.attr_specs_size += 1;

                ptr += n;

                if (spec.DW_FORM_form == DW_FORM_implicit_const) {

                    spec.implicit_const = decodeULEB128(ptr, &n);

                    ptr += n;
                }

                if (spec.DW_AT_at == 0 && spec.DW_FORM_form == 0) {
                    break;
                } else {
                    decl.attr_specs.push_back(spec);
                }
            }

            abbrev_table->push_back(decl);
        }

        abbrev_tables.insert({offset, abbrev_table});
    }

    abbrev_tables_loaded = true;
    return true;
}

bool DWARF::load_compilation_units(bool print) {

    // this->debug_info_end = debug_info + debug_info_size;

    unsigned char *ptr = (unsigned char *)sInfo.debug_info;

    while (ptr < sInfo.debug_info_end) { // For each DWARF Compilation Unit

        if (print) {
            std::cout << "Compilation Unit @ offset " << std::hex << sInfo.rel_offset(ptr) << std::endl;
        }

        compilation_units.emplace_back(*this, sInfo);

        compilation_units.back().read(ptr, print);

        // std::cout << memory_dump(ptr, 64) << std::endl;

        if (print) {
            std::cout << "END OF COMPILATION UNIT" << std::endl;
        }
    }

    if (print) {
        std::cout << "END OF .debug_info" << std::endl;
    }

    compilation_units_loaded = true;

    std::cout << "------- Stats -------" << std::endl;
    std::cout << "Total Compilation Units: " << compilation_units.size() << std::endl;
    std::cout << "Total Debugging Information entries: " << sInfo.DI_entries->size() << std::endl;
    std::cout << "Total Attributes: " << sInfo.attributes->size() << std::endl;

    std::cout << "low_pc_map size = " << sInfo.low_pc_map.size() << std::endl;

    return true;
}

DWARF_CU::DWARF_CU(DWARF &dwarf, DWARF_shared_info &shared_info) : parent_dwarf(dwarf), sInfo(shared_info) {
    index = sInfo.compilation_units->size();
}

bool DWARF_CU::read(unsigned char *&ptr, bool print) {

    offset = sInfo.rel_offset(ptr);

    uint32_t *check_length = (uint32_t *)ptr;

    if (*check_length == 0xffffffff) {

        // 64 bit
        uint64_t length = *(uint64_t *)ptr + 4;

    } else {

        // 32 bit
        cu_header = (DWARF32_CU_header *)ptr;
    }

    ptr += sizeof(DWARF32_CU_header);

    cu_end = (unsigned char *)cu_header + cu_header->length + sizeof(cu_header->length);

    if (print) {
        std::cout << "\t Length: " << std::hex << cu_header->length << std::endl;
        std::cout << "\t Version: " << cu_header->version << std::endl;
        std::cout << "\t Unit Type: " << std::hex << cu_header->unit_type << std::endl;
        std::cout << "\t Abbrev Offset: " << std::hex << cu_header->abbrev_offset << std::endl;
        std::cout << "\t Pointer Size: " << std::hex << cu_header->pointer_size << std::endl;
    }

    if (parent_dwarf.abbrev_tables.contains(cu_header->abbrev_offset) == false) {

        // std::cout << memory_dump(ptr, 64) << std::endl;

        std::cout << "Error: No abbrev table for this CU" << std::endl;
        return false;
    }

    abbrev_table = parent_dwarf.abbrev_tables.at(cu_header->abbrev_offset);

    uint32_t CU_index = sInfo.compilation_units->size() - 1;

    while (ptr < cu_end) {

        DI_children.push_back(sInfo.DI_entries->size());

        sInfo.DI_entries->emplace_back(*this, sInfo);

        sInfo.DI_entries->back().read(ptr, print);
    }
}

DWARF_DI_entry::DWARF_DI_entry(DWARF_CU &cu, DWARF_shared_info &shared_info) : parent_CU(cu.index), sInfo(shared_info) {
    index = sInfo.DI_entries->size();
}

bool DWARF_DI_entry::read(unsigned char *&ptr, bool print) {

    offset = sInfo.rel_offset(ptr);
    sInfo.offset_DI_map.insert({offset, index});

    unsigned n;

    Abbrev_number = decodeULEB128(ptr, &n);

    if (print) {
        std::cout << "Abbrev Number: " << Abbrev_number << std::endl;
    }

    ptr += n;

    // DWARF_CU cu = parent();

    if (parent().abbrev_table->size() <= Abbrev_number) {
        std::cout << "Error: Abbrev number out of range" << std::endl;
        return false;
    }

    abbrev_decl = &parent().abbrev_table->at(Abbrev_number);

    dw_tag = abbrev_decl->dw_tag;

    static size_t current_depth = 0;

    depth = current_depth;

    if (dw_tag == DW_NULL) {
        current_depth -= 1;

    } else if (abbrev_decl->dw_children == DW_CHILDREN_yes) {
        current_depth += 1;
    }

    uint32_t DI_index = sInfo.DI_entries->size() - 1;

    for (const DWARF_attr_spec &spec : abbrev_decl->attr_specs) {

        sInfo.attributes->emplace_back(*this, sInfo);

        sInfo.attributes->back().read(ptr, spec, print);

        attr_map.insert({spec.DW_AT_at, sInfo.attributes->back().index});

        // DWARF_attr attribute = read_attr(ptr, spec, print);

        // add_attribute(attribute);
    }
}

// DWARF_DI_entry::DWARF_DI_entry(DWARF_CU &cu, DWARF_shared_info &shared_info) : parent_CU(cu.index), sInfo(shared_info) {

DWARF_attr::DWARF_attr(DWARF_DI_entry &di, DWARF_shared_info &shared_info) : parent_DI(di.index), sInfo(shared_info) {
    index = sInfo.attributes->size();
}

bool DWARF_attr::read(unsigned char *&ptr, const DWARF_attr_spec &spec, bool print) {

    offset = sInfo.rel_offset(ptr);

    if (offset == 0x392b) {
        std::cout << "here" << std::endl;
        std::cout << memory_dump(ptr, 16) << std::endl;
    }

    this->spec = &spec;
    // this->value = ptr;

    if (print) {
        std::cout << "<" << sInfo.rel_offset(ptr) << "> ";
    }

    switch (spec.DW_FORM_form) {

        //------ address ------

    case DW_FORM_addr: {

        num_value = *(Elf64_Addr *)(ptr);
        hex_format = true;

        // TODO: TO solve
        if (spec.DW_AT_at == DW_AT_low_pc) {
            sInfo.low_pc_map.insert({num_value, this->index});
        }

        ptr += sizeof(Elf64_Addr);

        break;
    }

        //------ constant ------

    case DW_FORM_data1: {

        num_value = *(uint8_t *)(ptr);

        ptr += sizeof(uint8_t);

        break;
    }

    case DW_FORM_data2: {

        num_value = *(uint16_t *)(ptr);

        ptr += sizeof(uint16_t);

        break;
    }

    case DW_FORM_data4: {

        num_value = *(uint32_t *)(ptr);

        ptr += sizeof(uint32_t);

        break;
    }

    case DW_FORM_data8: {

        num_value = *(uint64_t *)(ptr);

        ptr += sizeof(uint64_t);

        break;
    }

    case DW_FORM_data16: {

        unsigned __int128 *data16 = (unsigned __int128 *)(ptr);

        ptr += sizeof(unsigned __int128);

        break;
    }

    case DW_FORM_implicit_const: {

        // No value stored in the .debug_info section
        num_value = spec.implicit_const;

        break;
    }

    case DW_FORM_sdata: {

        std::cout << std::hex << sInfo.rel_offset(ptr) << std::endl;

        unsigned n;

        num_value = decodeSLEB128(ptr, &n);

        is_signed = true;

        ptr += n;

        break;
    }

    case DW_FORM_udata: {

        unsigned n;

        num_value = decodeULEB128(ptr, &n);

        ptr += n;

        break;
    }

        //------ exprloc ------

    case DW_FORM_exprloc: {

        unsigned n;

        bytes_size = decodeULEB128(ptr, &n);

        ptr += n;

        bytes = new unsigned char[bytes_size];

        memcpy(bytes, ptr, bytes_size);

        ptr += bytes_size;

        break;
    }

        //------ flag ------

    case DW_FORM_flag: {

        uint8_t flag = *(uint8_t *)(ptr);

        if (flag != 0) {
            num_value = 1;
        }

        ptr += sizeof(uint8_t);

        break;
    }

    case DW_FORM_flag_present: {

        num_value = 1;

        break;
    }

        //------ reference ------

    case DW_FORM_ref1: {

        uint8_t *ref1 = (uint8_t *)(ptr);

        if (print) {
            std::cout << "DW_FORM_ref1: " << *ref1 << std::endl;
        }

        ptr += sizeof(uint8_t);

        break;
    }

    case DW_FORM_ref2: {

        uint16_t *ref2 = (uint16_t *)(ptr);

        if (print) {
            std::cout << "DW_FORM_ref2: " << *ref2 << std::endl;
        }

        ptr += sizeof(uint16_t);

        break;
    }

    case DW_FORM_ref4: {

        num_value = *(uint32_t *)(ptr);
        hex_format = true;

        if (print) {
            // std::cout << "DW_FORM_ref4: " << *ref4 << std::endl;
        }

        ptr += sizeof(uint32_t);

        break;
    }

    case DW_FORM_ref8: {

        uint64_t *ref8 = (uint64_t *)(ptr);

        if (print) {
            std::cout << "DW_FORM_ref8: " << *ref8 << std::endl;
        }

        ptr += sizeof(uint64_t);

        break;
    }

        //------ rnglist ------

    case DW_FORM_sec_offset: {

        num_value = *(Dwarf32_Off *)(ptr);

        ptr += sizeof(Dwarf32_Off);

        break;
    }

        //------ string ------

    case DW_FORM_string: {

        const char *str = (const char *)(ptr);

        str_value = (const unsigned char *)(str);

        ptr += strlen(str) + 1;

        break;
    }

    case DW_FORM_strp: {

        str_offset = *(Dwarf32_Off *)(ptr);

        str_value = (const unsigned char *)(sInfo.debug_str + str_offset);

        ptr += sizeof(Dwarf32_Off);

        break;
    }

    case DW_FORM_line_strp: {

        str_offset = *(Dwarf32_Off *)(ptr);

        str_value = (const unsigned char *)(sInfo.debug_line_str + str_offset);

        ptr += sizeof(Dwarf32_Off);

        break;
    }

    default: {

        // std::cout << memory_dump(ptr, 64) << std::endl;
        std::cout << "Non implemented: " << spec.DW_FORM_form << std::endl;
        break;
    }
    }

    return true;
}

DWARF_CU &DWARF_attr::grandparent() const {

    DWARF_DI_entry &p = this->parent();

    DWARF_CU &cu = p.parent();

    return cu;
}

std::string DWARF_attr::print() {

    std::stringstream ss;

    ss << "<" << std::hex << offset << ">\t" << DW_AT_STR.at(spec->DW_AT_at);

    if (spec->DW_AT_at == DW_AT_data_member_location || spec->DW_AT_at == DW_AT_linkage_name || spec->DW_AT_at == DW_AT_accessibility ||
        spec->DW_AT_at == DW_AT_object_pointer || spec->DW_AT_at == DW_AT_abstract_origin || spec->DW_AT_at == DW_AT_call_all_tail_calls ||
        spec->DW_AT_at == DW_AT_specification || spec->DW_AT_at == DW_AT_containing_type || spec->DW_AT_at == DW_AT_vtable_elem_location ||
        spec->DW_AT_at == DW_AT_call_all_calls) {
        ss << ": ";
    } else {
        ss << " : ";
    }

    if (spec->DW_FORM_form == DW_FORM_strp) {
        ss << "(indirect string, offset: ";
        if (str_offset != 0) {
            ss << "0x";
        }
        ss << str_offset << "): ";
    }

    if (spec->DW_FORM_form == DW_FORM_line_strp) {
        ss << "(indirect line string, offset: ";
        if (str_offset != 0) {
            ss << "0x";
        }
        ss << str_offset << "): ";
    }

    if (spec->DW_FORM_form == DW_FORM_exprloc) {
        ss << bytes_size;
        ss << " byte block: ";
    }

    if (spec->DW_AT_at == DW_AT_high_pc || spec->DW_AT_at == DW_AT_low_pc || spec->DW_AT_at == DW_AT_ranges || spec->DW_AT_at == DW_AT_stmt_list) {
        this->hex_format = true;
    }

    bool angle_brackets = false;

    if (spec->DW_AT_at == DW_AT_type || spec->DW_AT_at == DW_AT_sibling || spec->DW_AT_at == DW_AT_import || spec->DW_AT_at == DW_AT_object_pointer ||
        spec->DW_AT_at == DW_AT_containing_type || spec->DW_AT_at == DW_AT_abstract_origin || spec->DW_AT_at == DW_AT_specification) {

        DWARF_CU &cu = grandparent();
        num_value += cu.offset;

        angle_brackets = true;
    }

    if (offset == 0x3960) {
        std::cout << "debug stop" << std::endl;
    }

    if (angle_brackets) {
        ss << "<" << *this << ">";
    } else {
        ss << *this;
    }

    if (spec->DW_AT_at == DW_AT_encoding) {
        ss << " (" << DW_ATE_STR.at(num_value) << ")";

    } else if (spec->DW_AT_at == DW_AT_import) {

        if (sInfo.offset_DI_map.find(num_value) == sInfo.offset_DI_map.end()) {
            // std::cout << this->offset << std::endl;
            ss << " (not found)";

        } else {
            uint32_t index = sInfo.offset_DI_map.at(num_value);
            DWARF_DI_entry &di = sInfo.DI_entries->at(index);

            ss << " [";
            print_DI_desc(ss, di);
            ss << "]";
        }

    } else if (spec->DW_AT_at == DW_AT_language) {

        DW_LANG dw_lang = (DW_LANG)num_value;

        ss << " (" << DW_LANG_STR.at(dw_lang) << ")";

        // std::cout << " (" << DW_LANG_STR.at << ")";

    } else if (spec->DW_AT_at == DW_AT_accessibility) {
        if (num_value == 1) {
            ss << " (public)";
        } else if (num_value == 2) {
            ss << " (protected)";
        }

    } else if (spec->DW_AT_at == DW_AT_inline) {
        if (num_value == 0) {
            ss << " (not inlined)";
        } else if (num_value == 2) {
            ss << " (declared as inline but ignored)";
        }

    } else if (spec->DW_AT_at == DW_AT_virtuality) {
        if (num_value == 1) {
            ss << " (virtual)";
        }

    } else if (spec->DW_AT_at == DW_AT_location || spec->DW_AT_at == DW_AT_vtable_elem_location) {

        if (bytes != nullptr) {

            std::string dw_op = DW_OP_STR.at(bytes[0]);
            ss << "(";
            ss << dw_op;

            if (dw_op == "DW_OP_addr" && bytes_size > 8) {

                uint64_t addr = *(uint64_t *)(&bytes[1]);
                ss << ": " << std::hex << addr;

            } else if ((dw_op == "DW_OP_fbreg" || dw_op == "DW_OP_constu") && bytes_size > 1) {

                unsigned n;
                int64_t offset = decodeSLEB128(&bytes[1], &n);

                if (n > 1) {
                }

                ss << ": " << std::dec << offset;
            }

            ss << ")";
        }

    } else if (spec->DW_AT_at == DW_AT_frame_base) {
        if (bytes != nullptr && bytes_size > 0) {
            std::string dw_op = DW_OP_STR.at(bytes[0]);
            ss << "(" << dw_op << ")";
        }
    }

    return ss.str();
}
