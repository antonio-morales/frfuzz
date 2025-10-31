/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <map>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "elf.h"

typedef uint32_t Dwarf32_Off;
typedef uint64_t Dwarf64_Off;

enum DW_UT : uint8_t {
    DW_UT_compile = 0x01,
    DW_UT_type = 0x02,
    DW_UT_partial = 0x03,
    DW_UT_skeleton = 0x04,
    DW_UT_split_compile = 0x05,
    DW_UT_split_type = 0x06,
    DW_UT_lo_user = 0x80,
    DW_UT_hi_user = 0xff
};

inline const std::map<DW_UT, std::string> DW_UT_STR = {
    {DW_UT_compile, "DW_UT_compile"},
    {DW_UT_type, "DW_UT_type"},
    {DW_UT_partial, "DW_UT_partial"},
    {DW_UT_skeleton, "DW_UT_skeleton"},
    {DW_UT_split_compile, "DW_UT_split_compile"},
    {DW_UT_split_type, "DW_UT_split_type"},
    {DW_UT_lo_user, "DW_UT_lo_user"},
    {DW_UT_hi_user, "DW_UT_hi_user"},
};

// Tag encodings
enum DW_TAG {
    DW_NULL = 0x00,
    DW_TAG_array_type = 0x01,
    DW_TAG_class_type = 0x02,
    DW_TAG_entry_point = 0x03,
    DW_TAG_enumeration_type = 0x04,
    DW_TAG_formal_parameter = 0x05,
    DW_TAG_imported_declaration = 0x08,
    DW_TAG_label = 0x0a,
    DW_TAG_lexical_block = 0x0b,
    DW_TAG_member = 0x0d,
    DW_TAG_pointer_type = 0x0f,
    DW_TAG_reference_type = 0x10,
    DW_TAG_compile_unit = 0x11,
    DW_TAG_string_type = 0x12,
    DW_TAG_structure_type = 0x13,
    DW_TAG_subroutine_type = 0x15,
    DW_TAG_typedef = 0x16,
    DW_TAG_union_type = 0x17,
    DW_TAG_unspecified_parameters = 0x18,
    DW_TAG_variant = 0x19,
    DW_TAG_common_block = 0x1a,
    DW_TAG_common_inclusion = 0x1b,
    DW_TAG_inheritance = 0x1c,
    DW_TAG_inlined_subroutine = 0x1d,
    DW_TAG_module = 0x1e,
    DW_TAG_ptr_to_member_type = 0x1f,
    DW_TAG_set_type = 0x20,
    DW_TAG_subrange_type = 0x21,
    DW_TAG_with_stmt = 0x22,
    DW_TAG_access_declaration = 0x23,
    DW_TAG_base_type = 0x24,
    DW_TAG_catch_block = 0x25,
    DW_TAG_const_type = 0x26,
    DW_TAG_constant = 0x27,
    DW_TAG_enumerator = 0x28,
    DW_TAG_file_type = 0x29,
    DW_TAG_friend = 0x2a,
    DW_TAG_namelist = 0x2b,
    DW_TAG_namelist_item = 0x2c,
    DW_TAG_packed_type = 0x2d,
    DW_TAG_subprogram = 0x2e,
    DW_TAG_template_type_parameter = 0x2f,
    DW_TAG_template_value_parameter = 0x30,
    DW_TAG_thrown_type = 0x31,
    DW_TAG_try_block = 0x32,
    DW_TAG_variant_part = 0x33,
    DW_TAG_variable = 0x34,
    DW_TAG_volatile_type = 0x35,
    DW_TAG_dwarf_procedure = 0x36,
    DW_TAG_restrict_type = 0x37,
    DW_TAG_interface_type = 0x38,
    DW_TAG_namespace = 0x39,
    DW_TAG_imported_module = 0x3a,
    DW_TAG_unspecified_type = 0x3b,
    DW_TAG_partial_unit = 0x3c,
    DW_TAG_imported_unit = 0x3d,
    DW_TAG_condition = 0x3f,
    DW_TAG_shared_type = 0x40,
    DW_TAG_type_unit = 0x41,
    DW_TAG_rvalue_reference_type = 0x42,
    DW_TAG_template_alias = 0x43,
    DW_TAG_coarray_type = 0x44,
    DW_TAG_generic_subrange = 0x45,
    DW_TAG_dynamic_type = 0x46,
    DW_TAG_atomic_type = 0x47,
    DW_TAG_call_site = 0x48,
    DW_TAG_call_site_parameter = 0x49,
    DW_TAG_skeleton_unit = 0x4a,
    DW_TAG_immutable_type = 0x4b,
    DW_TAG_lo_user = 0x4080,
    DW_TAG_hi_user = 0xffff
};

inline const std::map<DW_TAG, std::string> DW_TAG_STR = {{DW_NULL, "DW_NULL"},
                                                         {DW_TAG_array_type, "DW_TAG_array_type"},
                                                         {DW_TAG_class_type, "DW_TAG_class_type"},
                                                         {DW_TAG_entry_point, "DW_TAG_entry_point"},
                                                         {DW_TAG_enumeration_type, "DW_TAG_enumeration_type"},
                                                         {DW_TAG_formal_parameter, "DW_TAG_formal_parameter"},
                                                         {DW_TAG_imported_declaration, "DW_TAG_imported_declaration"},
                                                         {DW_TAG_label, "DW_TAG_label"},
                                                         {DW_TAG_lexical_block, "DW_TAG_lexical_block"},
                                                         {DW_TAG_member, "DW_TAG_member"},
                                                         {DW_TAG_pointer_type, "DW_TAG_pointer_type"},
                                                         {DW_TAG_reference_type, "DW_TAG_reference_type"},
                                                         {DW_TAG_compile_unit, "DW_TAG_compile_unit"},
                                                         {DW_TAG_string_type, "DW_TAG_string_type"},
                                                         {DW_TAG_structure_type, "DW_TAG_structure_type"},
                                                         {DW_TAG_subroutine_type, "DW_TAG_subroutine_type"},
                                                         {DW_TAG_typedef, "DW_TAG_typedef"},
                                                         {DW_TAG_union_type, "DW_TAG_union_type"},
                                                         {DW_TAG_unspecified_parameters, "DW_TAG_unspecified_parameters"},
                                                         {DW_TAG_variant, "DW_TAG_variant"},
                                                         {DW_TAG_common_block, "DW_TAG_common_block"},
                                                         {DW_TAG_common_inclusion, "DW_TAG_common_inclusion"},
                                                         {DW_TAG_inheritance, "DW_TAG_inheritance"},
                                                         {DW_TAG_inlined_subroutine, "DW_TAG_inlined_subroutine"},
                                                         {DW_TAG_module, "DW_TAG_module"},
                                                         {DW_TAG_ptr_to_member_type, "DW_TAG_ptr_to_member_type"},
                                                         {DW_TAG_set_type, "DW_TAG_set_type"},
                                                         {DW_TAG_subrange_type, "DW_TAG_subrange_type"},
                                                         {DW_TAG_with_stmt, "DW_TAG_with_stmt"},
                                                         {DW_TAG_access_declaration, "DW_TAG_access_declaration"},
                                                         {DW_TAG_base_type, "DW_TAG_base_type"},
                                                         {DW_TAG_catch_block, "DW_TAG_catch_block"},
                                                         {DW_TAG_const_type, "DW_TAG_const_type"},
                                                         {DW_TAG_constant, "DW_TAG_constant"},
                                                         {DW_TAG_enumerator, "DW_TAG_enumerator"},
                                                         {DW_TAG_file_type, "DW_TAG_file_type"},
                                                         {DW_TAG_friend, "DW_TAG_friend"},
                                                         {DW_TAG_namelist, "DW_TAG_namelist"},
                                                         {DW_TAG_namelist_item, "DW_TAG_namelist_item"},
                                                         {DW_TAG_packed_type, "DW_TAG_packed_type"},
                                                         {DW_TAG_subprogram, "DW_TAG_subprogram"},
                                                         {DW_TAG_template_type_parameter, "DW_TAG_template_type_param"},
                                                         {DW_TAG_template_value_parameter, "DW_TAG_template_value_parameter"},
                                                         {DW_TAG_thrown_type, "DW_TAG_thrown_type"},
                                                         {DW_TAG_try_block, "DW_TAG_try_block"},
                                                         {DW_TAG_variant_part, "DW_TAG_variant_part"},
                                                         {DW_TAG_variable, "DW_TAG_variable"},
                                                         {DW_TAG_volatile_type, "DW_TAG_volatile_type"},
                                                         {DW_TAG_dwarf_procedure, "DW_TAG_dwarf_procedure"},
                                                         {DW_TAG_restrict_type, "DW_TAG_restrict_type"},
                                                         {DW_TAG_interface_type, "DW_TAG_interface_type"},
                                                         {DW_TAG_namespace, "DW_TAG_namespace"},
                                                         {DW_TAG_imported_module, "DW_TAG_imported_module"},
                                                         {DW_TAG_unspecified_type, "DW_TAG_unspecified_type"},
                                                         {DW_TAG_partial_unit, "DW_TAG_partial_unit"},
                                                         {DW_TAG_imported_unit, "DW_TAG_imported_unit"},
                                                         {DW_TAG_condition, "DW_TAG_condition"},
                                                         {DW_TAG_shared_type, "DW_TAG_shared_type"},
                                                         {DW_TAG_type_unit, "DW_TAG_type_unit"},
                                                         {DW_TAG_rvalue_reference_type, "DW_TAG_rvalue_reference_type"},
                                                         {DW_TAG_template_alias, "DW_TAG_template_alias"},
                                                         {DW_TAG_coarray_type, "DW_TAG_coarray_type"},
                                                         {DW_TAG_generic_subrange, "DW_TAG_generic_subrange"},
                                                         {DW_TAG_dynamic_type, "DW_TAG_dynamic_type"},
                                                         {DW_TAG_atomic_type, "DW_TAG_atomic_type"},
                                                         {DW_TAG_call_site, "DW_TAG_call_site"},
                                                         {DW_TAG_call_site_parameter, "DW_TAG_call_site_parameter"},
                                                         {DW_TAG_skeleton_unit, "DW_TAG_skeleton_unit"},
                                                         {DW_TAG_immutable_type, "DW_TAG_immutable_type"},
                                                         {DW_TAG_lo_user, "DW_TAG_lo_user"},
                                                         {DW_TAG_hi_user, "DW_TAG_hi_user"}};

enum DW_CHILDREN { DW_CHILDREN_no = 0x00, DW_CHILDREN_yes = 0x01 };

// Attribute encodings
enum DW_AT : uint16_t {
    DW_AT_sibling = 0x01,
    DW_AT_location = 0x02,
    DW_AT_name = 0x03,
    DW_AT_ordering = 0x09,
    DW_AT_byte_size = 0x0b,
    DW_AT_bit_offset = 0x0c,
    DW_AT_bit_size = 0x0d,
    DW_AT_stmt_list = 0x10,
    DW_AT_low_pc = 0x11,
    DW_AT_high_pc = 0x12,
    DW_AT_language = 0x13,
    DW_AT_discr = 0x15,
    DW_AT_discr_value = 0x16,
    DW_AT_visibility = 0x17,
    DW_AT_import = 0x18,
    DW_AT_string_length = 0x19,
    DW_AT_common_reference = 0x1a,
    DW_AT_comp_dir = 0x1b,
    DW_AT_const_value = 0x1c,
    DW_AT_containing_type = 0x1d,
    DW_AT_default_value = 0x1e,
    DW_AT_inline = 0x20,
    DW_AT_is_optional = 0x21,
    DW_AT_lower_bound = 0x22,
    DW_AT_producer = 0x25,
    DW_AT_prototyped = 0x27,
    DW_AT_return_addr = 0x2a,
    DW_AT_start_scope = 0x2c,
    DW_AT_bit_stride = 0x2e,
    DW_AT_upper_bound = 0x2f,
    DW_AT_abstract_origin = 0x31,
    DW_AT_accessibility = 0x32,
    DW_AT_address_class = 0x33,
    DW_AT_artificial = 0x34,
    DW_AT_base_types = 0x35,
    DW_AT_calling_convention = 0x36,
    DW_AT_count = 0x37,
    DW_AT_data_member_location = 0x38,
    DW_AT_decl_column = 0x39,
    DW_AT_decl_file = 0x3a,
    DW_AT_decl_line = 0x3b,
    DW_AT_declaration = 0x3c,
    DW_AT_discr_list = 0x3d,
    DW_AT_encoding = 0x3e,
    DW_AT_external = 0x3f,
    DW_AT_frame_base = 0x40,
    DW_AT_friend = 0x41,
    DW_AT_identifier_case = 0x42,
    DW_AT_macro_info = 0x43,
    DW_AT_namelist_item = 0x44,
    DW_AT_priority = 0x45,
    DW_AT_segment = 0x46,
    DW_AT_specification = 0x47,
    DW_AT_static_link = 0x48,
    DW_AT_type = 0x49,
    DW_AT_use_location = 0x4a,
    DW_AT_variable_parameter = 0x4b,
    DW_AT_virtuality = 0x4c,
    DW_AT_vtable_elem_location = 0x4d,
    DW_AT_allocated = 0x4e,
    DW_AT_associated = 0x4f,
    DW_AT_data_location = 0x50,
    DW_AT_byte_stride = 0x51,
    DW_AT_entry_pc = 0x52,
    DW_AT_use_UTF8 = 0x53,
    DW_AT_extension = 0x54,
    DW_AT_ranges = 0x55,
    DW_AT_trampoline = 0x56,
    DW_AT_call_column = 0x57,
    DW_AT_call_file = 0x58,
    DW_AT_call_line = 0x59,
    DW_AT_description = 0x5a,
    DW_AT_binary_scale = 0x5b,
    DW_AT_decimal_scale = 0x5c,
    DW_AT_small = 0x5d,
    DW_AT_decimal_sign = 0x5e,
    DW_AT_digit_count = 0x5f,
    DW_AT_picture_string = 0x60,
    DW_AT_mutable = 0x61,
    DW_AT_threads_scaled = 0x62,
    DW_AT_explicit = 0x63,
    DW_AT_object_pointer = 0x64,
    DW_AT_endianity = 0x65,
    DW_AT_elemental = 0x66,
    DW_AT_pure = 0x67,
    DW_AT_recursive = 0x68,
    DW_AT_signature = 0x69,
    DW_AT_main_subprogram = 0x6a,
    DW_AT_data_bit_offset = 0x6b,
    DW_AT_const_expr = 0x6c,
    DW_AT_enum_class = 0x6d,
    DW_AT_linkage_name = 0x6e,
    DW_AT_string_length_bit_size = 0x6f,
    DW_AT_string_length_byte_size = 0x70,
    DW_AT_rank = 0x71,
    DW_AT_str_offsets_base = 0x72,
    DW_AT_addr_base = 0x73,
    DW_AT_rnglists_base = 0x74,
    DW_AT_dwo_name = 0x76,
    DW_AT_reference = 0x77,
    DW_AT_rvalue_reference = 0x78,
    DW_AT_macros = 0x79,
    DW_AT_call_all_calls = 0x7a,
    DW_AT_call_all_source_calls = 0x7b,
    DW_AT_call_all_tail_calls = 0x7c,
    DW_AT_call_return_pc = 0x7d,
    DW_AT_call_value = 0x7e,
    DW_AT_call_origin = 0x7f,
    DW_AT_call_parameter = 0x80,
    DW_AT_call_pc = 0x81,
    DW_AT_call_tail_call = 0x82,
    DW_AT_call_target = 0x83,
    DW_AT_call_target_clobbered = 0x84,
    DW_AT_call_data_location = 0x85,
    DW_AT_call_data_value = 0x86,
    DW_AT_noreturn = 0x87,
    DW_AT_alignment = 0x88,
    DW_AT_export_symbols = 0x89,
    DW_AT_deleted = 0x8a,
    DW_AT_defaulted = 0x8b,
    DW_AT_loclists_base = 0x8c,
    DW_AT_lo_user = 0x2000,
    DW_AT_hi_user = 0x3fff
};

inline const std::map<DW_AT, std::string> DW_AT_STR = {{DW_AT_sibling, "DW_AT_sibling"},
                                                       {DW_AT_location, "DW_AT_location"},
                                                       {DW_AT_name, "DW_AT_name"},
                                                       {DW_AT_ordering, "DW_AT_ordering"},
                                                       {DW_AT_byte_size, "DW_AT_byte_size"},
                                                       {DW_AT_bit_offset, "DW_AT_bit_offset"},
                                                       {DW_AT_bit_size, "DW_AT_bit_size"},
                                                       {DW_AT_stmt_list, "DW_AT_stmt_list"},
                                                       {DW_AT_low_pc, "DW_AT_low_pc"},
                                                       {DW_AT_high_pc, "DW_AT_high_pc"},
                                                       {DW_AT_language, "DW_AT_language"},
                                                       {DW_AT_discr, "DW_AT_discr"},
                                                       {DW_AT_discr_value, "DW_AT_discr_value"},
                                                       {DW_AT_visibility, "DW_AT_visibility"},
                                                       {DW_AT_import, "DW_AT_import"},
                                                       {DW_AT_string_length, "DW_AT_string_length"},
                                                       {DW_AT_common_reference, "DW_AT_common_reference"},
                                                       {DW_AT_comp_dir, "DW_AT_comp_dir"},
                                                       {DW_AT_const_value, "DW_AT_const_value"},
                                                       {DW_AT_containing_type, "DW_AT_containing_type"},
                                                       {DW_AT_default_value, "DW_AT_default_value"},
                                                       {DW_AT_inline, "DW_AT_inline"},
                                                       {DW_AT_is_optional, "DW_AT_is_optional"},
                                                       {DW_AT_lower_bound, "DW_AT_lower_bound"},
                                                       {DW_AT_producer, "DW_AT_producer"},
                                                       {DW_AT_prototyped, "DW_AT_prototyped"},
                                                       {DW_AT_return_addr, "DW_AT_return_addr"},
                                                       {DW_AT_start_scope, "DW_AT_start_scope"},
                                                       {DW_AT_bit_stride, "DW_AT_bit_stride"},
                                                       {DW_AT_upper_bound, "DW_AT_upper_bound"},
                                                       {DW_AT_abstract_origin, "DW_AT_abstract_origin"},
                                                       {DW_AT_accessibility, "DW_AT_accessibility"},
                                                       {DW_AT_address_class, "DW_AT_address_class"},
                                                       {DW_AT_artificial, "DW_AT_artificial"},
                                                       {DW_AT_base_types, "DW_AT_base_types"},
                                                       {DW_AT_calling_convention, "DW_AT_calling_convention"},
                                                       {DW_AT_count, "DW_AT_count"},
                                                       {DW_AT_data_member_location, "DW_AT_data_member_location"},
                                                       {DW_AT_decl_column, "DW_AT_decl_column"},
                                                       {DW_AT_decl_file, "DW_AT_decl_file"},
                                                       {DW_AT_decl_line, "DW_AT_decl_line"},
                                                       {DW_AT_declaration, "DW_AT_declaration"},
                                                       {DW_AT_discr_list, "DW_AT_discr_list"},
                                                       {DW_AT_encoding, "DW_AT_encoding"},
                                                       {DW_AT_external, "DW_AT_external"},
                                                       {DW_AT_frame_base, "DW_AT_frame_base"},
                                                       {DW_AT_friend, "DW_AT_friend"},
                                                       {DW_AT_identifier_case, "DW_AT_identifier_case"},
                                                       {DW_AT_macro_info, "DW_AT_macro_info"},
                                                       {DW_AT_namelist_item, "DW_AT_namelist_item"},
                                                       {DW_AT_priority, "DW_AT_priority"},
                                                       {DW_AT_segment, "DW_AT_segment"},
                                                       {DW_AT_specification, "DW_AT_specification"},
                                                       {DW_AT_static_link, "DW_AT_static_link"},
                                                       {DW_AT_type, "DW_AT_type"},
                                                       {DW_AT_use_location, "DW_AT_use_location"},
                                                       {DW_AT_variable_parameter, "DW_AT_variable_parameter"},
                                                       {DW_AT_virtuality, "DW_AT_virtuality"},
                                                       {DW_AT_vtable_elem_location, "DW_AT_vtable_elem_location"},
                                                       {DW_AT_allocated, "DW_AT_allocated"},
                                                       {DW_AT_associated, "DW_AT_associated"},
                                                       {DW_AT_data_location, "DW_AT_data_location"},
                                                       {DW_AT_byte_stride, "DW_AT_byte_stride"},
                                                       {DW_AT_entry_pc, "DW_AT_entry_pc"},
                                                       {DW_AT_use_UTF8, "DW_AT_use_UTF8"},
                                                       {DW_AT_extension, "DW_AT_extension"},
                                                       {DW_AT_ranges, "DW_AT_ranges"},
                                                       {DW_AT_trampoline, "DW_AT_trampoline"},
                                                       {DW_AT_call_column, "DW_AT_call_column"},
                                                       {DW_AT_call_file, "DW_AT_call_file"},
                                                       {DW_AT_call_line, "DW_AT_call_line"},
                                                       {DW_AT_description, "DW_AT_description"},
                                                       {DW_AT_binary_scale, "DW_AT_binary_scale"},
                                                       {DW_AT_decimal_scale, "DW_AT_decimal_scale"},
                                                       {DW_AT_small, "DW_AT_small"},
                                                       {DW_AT_decimal_sign, "DW_AT_decimal_sign"},
                                                       {DW_AT_digit_count, "DW_AT_digit_count"},
                                                       {DW_AT_picture_string, "DW_AT_picture_string"},
                                                       {DW_AT_mutable, "DW_AT_mutable"},
                                                       {DW_AT_threads_scaled, "DW_AT_threads_scaled"},
                                                       {DW_AT_explicit, "DW_AT_explicit"},
                                                       {DW_AT_object_pointer, "DW_AT_object_pointer"},
                                                       {DW_AT_endianity, "DW_AT_endianity"},
                                                       {DW_AT_elemental, "DW_AT_elemental"},
                                                       {DW_AT_pure, "DW_AT_pure"},
                                                       {DW_AT_recursive, "DW_AT_recursive"},
                                                       {DW_AT_signature, "DW_AT_signature"},
                                                       {DW_AT_main_subprogram, "DW_AT_main_subprogram"},
                                                       {DW_AT_data_bit_offset, "DW_AT_data_bit_offset"},
                                                       {DW_AT_const_expr, "DW_AT_const_expr"},
                                                       {DW_AT_enum_class, "DW_AT_enum_class"},
                                                       {DW_AT_linkage_name, "DW_AT_linkage_name"},
                                                       {DW_AT_string_length_bit_size, "DW_AT_string_length_bit_size"},
                                                       {DW_AT_string_length_byte_size, "DW_AT_string_length_byte_size"},
                                                       {DW_AT_rank, "DW_AT_rank"},
                                                       {DW_AT_str_offsets_base, "DW_AT_str_offsets_base"},
                                                       {DW_AT_addr_base, "DW_AT_addr_base"},
                                                       {DW_AT_rnglists_base, "DW_AT_rnglists_base"},
                                                       {DW_AT_dwo_name, "DW_AT_dwo_name"},
                                                       {DW_AT_reference, "DW_AT_reference"},
                                                       {DW_AT_rvalue_reference, "DW_AT_rvalue_reference"},
                                                       {DW_AT_macros, "DW_AT_macros"},
                                                       {DW_AT_call_all_calls, "DW_AT_call_all_calls"},
                                                       {DW_AT_call_all_source_calls, "DW_AT_call_all_source_calls"},
                                                       {DW_AT_call_all_tail_calls, "DW_AT_call_all_tail_calls"},
                                                       {DW_AT_call_return_pc, "DW_AT_call_return_pc"},
                                                       {DW_AT_call_value, "DW_AT_call_value"},
                                                       {DW_AT_call_origin, "DW_AT_call_origin"},
                                                       {DW_AT_call_parameter, "DW_AT_call_parameter"},
                                                       {DW_AT_call_pc, "DW_AT_call_pc"},
                                                       {DW_AT_call_tail_call, "DW_AT_call_tail_call"},
                                                       {DW_AT_call_target, "DW_AT_call_target"},
                                                       {DW_AT_call_target_clobbered, "DW_AT_call_target_clobbered"},
                                                       {DW_AT_call_data_location, "DW_AT_call_data_location"},
                                                       {DW_AT_call_data_value, "DW_AT_call_data_value"},
                                                       {DW_AT_noreturn, "DW_AT_noreturn"},
                                                       {DW_AT_alignment, "DW_AT_alignment"},
                                                       {DW_AT_export_symbols, "DW_AT_export_symbols"},
                                                       {DW_AT_deleted, "DW_AT_deleted"},
                                                       {DW_AT_defaulted, "DW_AT_defaulted"},
                                                       {DW_AT_loclists_base, "DW_AT_loclists_base"},
                                                       {DW_AT_lo_user, "DW_AT_lo_user"},
                                                       {DW_AT_hi_user, "DW_AT_hi_user"}};

// Attribute form encodings
enum DW_FORM {
    DW_FORM_addr = 0x01,
    DW_FORM_block2 = 0x03,
    DW_FORM_block4 = 0x04,
    DW_FORM_data2 = 0x05,
    DW_FORM_data4 = 0x06,
    DW_FORM_data8 = 0x07,
    DW_FORM_string = 0x08,
    DW_FORM_block = 0x09,
    DW_FORM_block1 = 0x0a,
    DW_FORM_data1 = 0x0b,
    DW_FORM_flag = 0x0c,
    DW_FORM_sdata = 0x0d,
    DW_FORM_strp = 0x0e,
    DW_FORM_udata = 0x0f,
    DW_FORM_ref_addr = 0x10,
    DW_FORM_ref1 = 0x11,
    DW_FORM_ref2 = 0x12,
    DW_FORM_ref4 = 0x13,
    DW_FORM_ref8 = 0x14,
    DW_FORM_ref_udata = 0x15,
    DW_FORM_indirect = 0x16,
    DW_FORM_sec_offset = 0x17,
    DW_FORM_exprloc = 0x18,
    DW_FORM_flag_present = 0x19,
    DW_FORM_strx = 0x1a,
    DW_FORM_addrx = 0x1b,
    DW_FORM_ref_sup4 = 0x1c,
    DW_FORM_strp_sup = 0x1d,
    DW_FORM_data16 = 0x1e,
    DW_FORM_line_strp = 0x1f,
    DW_FORM_ref_sig8 = 0x20,
    DW_FORM_implicit_const = 0x21,
    DW_FORM_loclistx = 0x22,
    DW_FORM_rnglistx = 0x23,
    DW_FORM_ref_sup8 = 0x24,
    DW_FORM_strx1 = 0x25,
    DW_FORM_strx2 = 0x26,
    DW_FORM_strx3 = 0x27,
    DW_FORM_strx4 = 0x28,
    DW_FORM_addrx1 = 0x29,
    DW_FORM_addrx2 = 0x2a,
    DW_FORM_addrx3 = 0x2b,
    DW_FORM_addrx4 = 0x2c
};

enum DW_LANG : uint16_t {
    DW_LANG_C89 = 0x0001,
    DW_LANG_C = 0x0002,
    DW_LANG_Ada83 = 0x0003,
    DW_LANG_C_plus_plus = 0x0004,
    DW_LANG_Cobol74 = 0x0005,
    DW_LANG_Cobol85 = 0x0006,
    DW_LANG_Fortran77 = 0x0007,
    DW_LANG_Fortran90 = 0x0008,
    DW_LANG_Pascal83 = 0x0009,
    DW_LANG_Modula2 = 0x000a,
    DW_LANG_Java = 0x000b,
    DW_LANG_C99 = 0x000c,
    DW_LANG_Ada95 = 0x000d,
    DW_LANG_Fortran95 = 0x000e,
    DW_LANG_PLI = 0x000f,
    DW_LANG_ObjC = 0x0010,
    DW_LANG_ObjC_plus_plus = 0x0011,
    DW_LANG_UPC = 0x0012,
    DW_LANG_D = 0x0013,
    DW_LANG_Python = 0x0014,
    DW_LANG_OpenCL = 0x0015,
    DW_LANG_Go = 0x0016,
    DW_LANG_Modula3 = 0x0017,
    DW_LANG_Haskell = 0x0018,
    DW_LANG_C_plus_plus_03 = 0x0019,
    DW_LANG_C_plus_plus_11 = 0x001a,
    DW_LANG_OCaml = 0x001b,
    DW_LANG_Rust = 0x001c,
    DW_LANG_C11 = 0x001d,
    DW_LANG_Swift = 0x001e,
    DW_LANG_Julia = 0x001f,
    DW_LANG_Dylan = 0x0020,
    DW_LANG_C_plus_plus_14 = 0x0021,
    DW_LANG_Fortran03 = 0x0022,
    DW_LANG_Fortran08 = 0x0023,
    DW_LANG_RenderScript = 0x0024,
    DW_LANG_BLISS = 0x0025,
    DW_LANG_lo_user = 0x8000,
    DW_LANG_hi_user = 0xffff
};

inline const std::map<DW_LANG, std::string> DW_LANG_STR = {{DW_LANG_C89, "C89"},
                                                           {DW_LANG_C, "C"},
                                                           {DW_LANG_Ada83, "Ada83"},
                                                           {DW_LANG_C_plus_plus, "C++"},
                                                           {DW_LANG_Cobol74, "Cobol74"},
                                                           {DW_LANG_Cobol85, "Cobol85"},
                                                           {DW_LANG_Fortran77, "Fortran77"},
                                                           {DW_LANG_Fortran90, "Fortran90"},
                                                           {DW_LANG_Pascal83, "Pascal83"},
                                                           {DW_LANG_Modula2, "Modula2"},
                                                           {DW_LANG_Java, "Java"},
                                                           {DW_LANG_C99, "C99"},
                                                           {DW_LANG_Ada95, "Ada95"},
                                                           {DW_LANG_Fortran95, "Fortran95"},
                                                           {DW_LANG_PLI, "PLI"},
                                                           {DW_LANG_ObjC, "ObjC"},
                                                           {DW_LANG_ObjC_plus_plus, "ObjC++"},
                                                           {DW_LANG_UPC, "UPC"},
                                                           {DW_LANG_D, "D"},
                                                           {DW_LANG_Python, "Python"},
                                                           {DW_LANG_OpenCL, "OpenCL"},
                                                           {DW_LANG_Go, "Go"},
                                                           {DW_LANG_Modula3, "Modula3"},
                                                           {DW_LANG_Haskell, "Haskell"},
                                                           {DW_LANG_C_plus_plus_03, "C++03"},
                                                           {DW_LANG_C_plus_plus_11, "C++11"},
                                                           {DW_LANG_OCaml, "OCaml"},
                                                           {DW_LANG_Rust, "Rust"},
                                                           {DW_LANG_C11, "C11"},
                                                           {DW_LANG_Swift, "Swift"},
                                                           {DW_LANG_Julia, "Julia"},
                                                           {DW_LANG_Dylan, "Dylan"},
                                                           {DW_LANG_C_plus_plus_14, "C++14"},
                                                           {DW_LANG_Fortran03, "Fortran03"},
                                                           {DW_LANG_Fortran08, "Fortran08"},
                                                           {DW_LANG_RenderScript, "RenderScript"},
                                                           {DW_LANG_BLISS, "BLISS"},
                                                           {DW_LANG_lo_user, "lo_user"},
                                                           {DW_LANG_hi_user, "hi_user"}

};

inline const std::map<uint8_t, std::string> DW_ATE_STR = {
    {1, "address"},         {2, "boolean"},  {3, "complex float"}, {4, "float"},           {5, "signed"},
    {6, "signed char"},     {7, "unsigned"}, {8, "unsigned char"}, {9, "imaginary float"}, {10, "packed_decimal"},
    {11, "numeric string"}, {12, "edited"},  {13, "signed fixed"}, {14, "unsigned_fixed"}, {15, "decimal float"},
    {16, "unicode string"}, {17, "UCS"},     {18, "ASCII"},        {128, "lo_user"},       {255, "hi_user"}};

inline const std::map<uint8_t, std::string> DW_OP_STR = {
    {0x03, "DW_OP_addr"},
    {0x06, "DW_OP_deref"},
    {0x08, "DW_OP_const1u"},
    {0x09, "DW_OP_const1s"},
    {0x0a, "DW_OP_const2u"},
    {0x0b, "DW_OP_const2s"},
    {0x0c, "DW_OP_const4u"},
    {0x0d, "DW_OP_const4s"},
    {0x0e, "DW_OP_const8u"},
    {0x0f, "DW_OP_const8s"},
    {0x10, "DW_OP_constu"},
    {0x11, "DW_OP_consts"},
    {0x12, "DW_OP_dup"},
    {0x13, "DW_OP_drop"},
    {0x14, "DW_OP_over"},
    {0x15, "DW_OP_pick"},
    {0x16, "DW_OP_swap"},
    {0x17, "DW_OP_rot"},
    {0x18, "DW_OP_xderef"},
    {0x19, "DW_OP_abs"},
    {0x1a, "DW_OP_and"},
    {0x1b, "DW_OP_div"},
    {0x1c, "DW_OP_minus"},
    {0x1d, "DW_OP_mod"},
    {0x1e, "DW_OP_mul"},
    {0x1f, "DW_OP_neg"},
    {0x20, "DW_OP_not"},
    {0x21, "DW_OP_or"},
    {0x22, "DW_OP_plus"},
    {0x23, "DW_OP_plus_uconst"},
    {0x24, "DW_OP_shl"},
    {0x25, "DW_OP_shr"},
    {0x26, "DW_OP_shra"},
    {0x27, "DW_OP_xor"},
    {0x2f, "DW_OP_skip"},
    {0x28, "DW_OP_bra"},
    {0x29, "DW_OP_eq"},
    {0x2a, "DW_OP_ge"},
    {0x2b, "DW_OP_gt"},
    {0x2c, "DW_OP_le"},
    {0x2d, "DW_OP_lt"},
    {0x2e, "DW_OP_ne"},
    {0x2f, "DW_OP_skip"},
    {0x30, "DW_OP_lit0"},
    {0x31, "DW_OP_lit1"},
    {0x32, "DW_OP_lit2"},
    {0x33, "DW_OP_lit3"},
    {0x34, "DW_OP_lit4"},
    {0x35, "DW_OP_lit5"},
    {0x36, "DW_OP_lit6"},
    {0x37, "DW_OP_lit7"},
    {0x38, "DW_OP_lit8"},
    {0x39, "DW_OP_lit9"},
    {0x3a, "DW_OP_lit10"},
    {0x3b, "DW_OP_lit11"},
    {0x3c, "DW_OP_lit12"},
    {0x3d, "DW_OP_lit13"},
    {0x3e, "DW_OP_lit14"},
    {0x3f, "DW_OP_lit15"},
    {0x40, "DW_OP_lit16"},
    {0x41, "DW_OP_lit17"},
    {0x42, "DW_OP_lit18"},
    {0x43, "DW_OP_lit19"},
    {0x44, "DW_OP_lit20"},
    {0x45, "DW_OP_lit21"},
    {0x46, "DW_OP_lit22"},
    {0x47, "DW_OP_lit23"},
    {0x48, "DW_OP_lit24"},
    {0x49, "DW_OP_lit25"},
    {0x4a, "DW_OP_lit26"},
    {0x4b, "DW_OP_lit27"},
    {0x4c, "DW_OP_lit28"},
    {0x4d, "DW_OP_lit29"},
    {0x4e, "DW_OP_lit30"},
    {0x4f, "DW_OP_lit31"},
    {0x50, "DW_OP_reg0"},
    {0x51, "DW_OP_reg1"},
    {0x52, "DW_OP_reg2"},
    {0x53, "DW_OP_reg3"},
    {0x54, "DW_OP_reg4"},
    {0x55, "DW_OP_reg5"},
    {0x56, "DW_OP_reg6"},
    {0x57, "DW_OP_reg7"},
    {0x58, "DW_OP_reg8"},
    {0x59, "DW_OP_reg9"},
    {0x5a, "DW_OP_reg10"},
    {0x5b, "DW_OP_reg11"},
    {0x5c, "DW_OP_reg12"},
    {0x5d, "DW_OP_reg13"},
    {0x5e, "DW_OP_reg14"},
    {0x5f, "DW_OP_reg15"},
    {0x60, "DW_OP_reg16"},
    {0x61, "DW_OP_reg17"},
    {0x62, "DW_OP_reg18"},
    {0x63, "DW_OP_reg19"},
    {0x64, "DW_OP_reg20"},
    {0x65, "DW_OP_reg21"},
    {0x66, "DW_OP_reg22"},
    {0x67, "DW_OP_reg23"},
    {0x68, "DW_OP_reg24"},
    {0x69, "DW_OP_reg25"},
    {0x6a, "DW_OP_reg26"},
    {0x6b, "DW_OP_reg27"},
    {0x6c, "DW_OP_reg28"},
    {0x6d, "DW_OP_reg29"},
    {0x6e, "DW_OP_reg30"},
    {0x6f, "DW_OP_reg31"},
    {0x70, "DW_OP_breg0"},
    {0x71, "DW_OP_breg1"},
    {0x72, "DW_OP_breg2"},
    {0x73, "DW_OP_breg3"},
    {0x74, "DW_OP_breg4"},
    {0x75, "DW_OP_breg5"},
    {0x76, "DW_OP_breg6"},
    {0x77, "DW_OP_breg7"},
    {0x78, "DW_OP_breg8"},
    {0x79, "DW_OP_breg9"},
    {0x7a, "DW_OP_breg10"},
    {0x7b, "DW_OP_breg11"},
    {0x7c, "DW_OP_breg12"},
    {0x7d, "DW_OP_breg13"},
    {0x7e, "DW_OP_breg14"},
    {0x7f, "DW_OP_breg15"},
    {0x80, "DW_OP_breg16"},
    {0x81, "DW_OP_breg17"},
    {0x82, "DW_OP_breg18"},
    {0x83, "DW_OP_breg19"},
    {0x84, "DW_OP_breg20"},
    {0x85, "DW_OP_breg21"},
    {0x86, "DW_OP_breg22"},
    {0x87, "DW_OP_breg23"},
    {0x88, "DW_OP_breg24"},
    {0x89, "DW_OP_breg25"},
    {0x8a, "DW_OP_breg26"},
    {0x8b, "DW_OP_breg27"},
    {0x8c, "DW_OP_breg28"},
    {0x8d, "DW_OP_breg29"},
    {0x8e, "DW_OP_breg30"},
    {0x8f, "DW_OP_breg31"},
    {0x90, "DW_OP_regx"},
    {0x91, "DW_OP_fbreg"},
    {0x92, "DW_OP_bregx"},
    {0x93, "DW_OP_piece"},
    {0x94, "DW_OP_deref_size"},
    {0x95, "DW_OP_xderef_size"},
    {0x96, "DW_OP_nop"},
    {0x97, "DW_OP_push_object_address"},
    {0x98, "DW_OP_call2"},
    {0x99, "DW_OP_call4"},
    {0x9a, "DW_OP_call_ref"},
    {0x9b, "DW_OP_form_tls_address"},
    {0x9c, "DW_OP_call_frame_cfa"},
    {0x9d, "DW_OP_bit_piece"},
    {0x9e, "DW_OP_implicit_value"},
    {0x9f, "DW_OP_stack_value"},
    {0xa0, "DW_OP_implicit_pointer"},
    {0xa1, "DW_OP_addrx"},
    {0xa2, "DW_OP_constx"},
    {0xa3, "DW_OP_entry_value"},
    {0xa4, "DW_OP_const_type"},
    {0xa5, "DW_OP_regval_type"},
    {0xa6, "DW_OP_deref_type"},
    {0xa7, "DW_OP_xderef_type"},
    {0xa8, "DW_OP_convert"},
    {0xa9, "DW_OP_reinterpret"},
    {0xe0, "DW_OP_lo_user"},
    {0xff, "DW_OP_hi_user"},
};

/*
size_t DW_FORM_size(DW_FORM dw_form){

    switch(dw_form){

        case DW_FORM_addr:{
            return sizeof(Elf64_Addr);
        }

        case DW_FORM_block2:{
            return sizeof(uint16_t);
        }

        case DW_FORM_block4:{
            return sizeof(uint32_t);
        }

        case DW_FORM_data2:{
            return sizeof(uint16_t);
        }

        case DW_FORM_data4:{
            return sizeof(uint32_t);
        }

        case DW_FORM_data8:{
            return sizeof(uint64_t);
        }

        case DW_FORM_block:{
            return sizeof(uint64_t);
        }


    }

}

*/

// Page 200-202 Version 5 DWARF
struct DWARF32_CU_header {
    uint32_t length;
    uint16_t version;
    DW_UT unit_type;
    uint8_t pointer_size;
    Dwarf32_Off abbrev_offset;
};

struct debug_info_entry {
    uint64_t abbrev_code;
    uint16_t version;
    uint32_t abbrev_offset;
    uint8_t address_size;
};

struct DWARF_attr_spec {
    DW_AT DW_AT_at;
    DW_FORM DW_FORM_form;
    uint64_t implicit_const;
};

struct DWARF_abbrev_decl {

    uint64_t abbrev_code;

    DW_TAG dw_tag;

    // next physically succeeding entry of any debugging information entry using this abbreviation is a sibling of that entry
    DW_CHILDREN dw_children;

    std::vector<DWARF_attr_spec> attr_specs;
    size_t attr_specs_size;
};

typedef std::vector<DWARF_abbrev_decl> Dwarf_Abbrev_table;

// Forward declarations
class DWARF_attr;
class DWARF_DI_entry;
class DWARF_CU;
class DWARF;

class DWARF_shared_info {
  public:
    const unsigned char *debug_info;
    size_t debug_info_size;
    const unsigned char *debug_info_end;

    const unsigned char *debug_str;
    const char *debug_line_str;

    const unsigned char *debug_abbrev;
    size_t debug_abbrev_size;
    const unsigned char *debug_abbrev_end;

    std::vector<DWARF_CU> *compilation_units;

    std::vector<DWARF_DI_entry> *DI_entries;

    std::vector<DWARF_attr> *attributes;

    // low_pc_map addr, attribute index
    std::unordered_multimap<Elf64_Addr, uint32_t> low_pc_map;

    // Offset, DI_entry index
    std::unordered_map<Dwarf32_Off, uint32_t> offset_DI_map;

    Elf32_Off rel_offset(const unsigned char *ptr);

  private:
};

class DWARF_attr {

    friend class DWARF_DI_entry;

  public:
    DWARF_attr(DWARF_DI_entry &di, DWARF_shared_info &shared_info);

    // Cast operators
    operator uint64_t() { return num_value; }
    operator bool() { return num_value; }
    operator const unsigned char *() { return str_value; }
    operator std::string() {
        if (str_value == nullptr) {
            return std::string("");
        }
        return std::string((char *)str_value);
    }

    bool read(unsigned char *&ptr, const DWARF_attr_spec &spec, bool print = false);

    // Overload << operator for DWARF_attr
    friend std::ostream &operator<<(std::ostream &os, const DWARF_attr &attr) {

        if (attr.str_value != attr.nullStr) {
            os << attr.str_value;

        } else if (attr.bytes != nullptr) {
            for (size_t i = 0; i < attr.bytes_size; i++) {
                os << std::hex << (int)attr.bytes[i] << " ";
            }

        } else if (attr.is_signed) {

            int64_t signed_value = (int64_t)attr.num_value;

            if (attr.hex_format) {
                os << "0x" << std::hex << signed_value;
            } else {
                os << std::dec << signed_value;
            }

        } else {
            if (attr.hex_format && attr.num_value != 0) {
                os << "0x" << std::hex << attr.num_value;
            } else {
                os << std::dec << attr.num_value;
            }
        }
        // os << attr.print().str();
        return os;
    }

    std::string print();

    inline DWARF_DI_entry &parent() const { return sInfo.DI_entries->at(parent_DI); }

    inline DWARF_CU &grandparent() const;

    inline Elf32_Off Offset() const { return offset; }

  private:
    uint32_t index; // Compilation Unit index
    Elf32_Off offset;

    const DWARF_attr_spec *spec;

    const unsigned char *const nullStr = (const unsigned char *)"\0";

    const unsigned char *str_value = nullStr;
    Dwarf32_Off str_offset = 0;

    uint64_t num_value = 0;
    bool is_signed = false;
    bool hex_format = false;

    unsigned char *bytes = nullptr;
    size_t bytes_size = 0;

    // Parent Debugging Information Entry index
    uint32_t parent_DI;

    DWARF_shared_info &sInfo;
};

class DWARF_DI_entry {

    friend class DWARF_attr;

  public:
    DWARF_DI_entry(DWARF_CU &cu, DWARF_shared_info &shared_info);

    bool read(unsigned char *&ptr, bool print = false);

    inline DW_TAG Type() const { return dw_tag; }

    inline DWARF_attr &operator[](DW_AT dw_at) {
        uint32_t index = attr_map.at(dw_at);
        return sInfo.attributes->at(index);
    }

    inline DWARF_CU &parent() const { return sInfo.compilation_units->at(parent_CU); }

    friend std::ostream &print_DI_desc(std::ostream &os, const DWARF_DI_entry &DI_entry) {

        if (DI_entry.dw_tag == DW_NULL) {
            os << "Abbrev Number: " << std::dec << DI_entry.Abbrev_number;
        } else {
            os << "Abbrev Number: " << std::dec << DI_entry.Abbrev_number << " (" << DW_TAG_STR.at(DI_entry.dw_tag) << ")";
        }

        return os;
    }

    // Overload << operator for DWARF_DI_entry
    friend std::ostream &operator<<(std::ostream &os, const DWARF_DI_entry &DI_entry) {

        os << "\t<" << DI_entry.depth << "><" << std::hex << DI_entry.offset << ">: ";

        print_DI_desc(os, DI_entry);
        os << std::endl;

        for (auto &decl : DI_entry.abbrev_decl->attr_specs) {

            uint32_t attr_index = DI_entry.attr_map.at(decl.DW_AT_at);

            DWARF_attr &attr = DI_entry.sInfo.attributes->at(attr_index);

            os << "\t\t" << attr.print() << std::endl;

            /*
            if(DI_entry.offset > 0x49618){
                std::cout << "let's gonna break" << std::endl;
            }
            */
        }

        return os;
    }

    // DW_TAG dw_tag() const { return dw_tag_prv; }

  private:
    uint32_t index; // Debugging Information Entry index
    Dwarf32_Off offset;

    DW_TAG dw_tag;

    size_t depth = 0;

    // DW_AT, attribute index
    std::map<DW_AT, uint32_t> attr_map;

    DWARF_abbrev_decl *abbrev_decl;
    uint64_t Abbrev_number;

    // Parent Compilation Unit index
    const uint32_t parent_CU;

    DWARF_shared_info &sInfo;
};

// DWARF Compilation Unit
class DWARF_CU {

    friend class DWARF_DI_entry;
    friend class DWARF_attr;

  public:
    DWARF_CU(DWARF &dwarf, DWARF_shared_info &shared_info);

    bool read(unsigned char *&ptr, bool print = false);

    inline DWARF &parent() const { return parent_dwarf; }

    // Overload << operator for DWARF_CU
    friend std::ostream &operator<<(std::ostream &os, const DWARF_CU &cu) {

        os << "Compilation Unit @ offset ";

        if (cu.offset != 0) {
            os << "0x";
        }
        os << std::hex << cu.offset << ":" << std::endl;

        os << "  Length: " << std::hex << "0x" << cu.cu_header->length;
        if (typeid(cu.cu_header) == typeid(DWARF32_CU_header *)) {
            os << " (32-bit)";
        } else {
            os << " (64-bit)";
        }
        os << std::endl;

        os << "  Version: " << cu.cu_header->version << std::endl;

        os << "  Unit Type: " << DW_UT_STR.at(cu.cu_header->unit_type) << " (" << std::to_string(cu.cu_header->unit_type) << ")" << std::endl;

        os << "  Abbrev Offset: ";
        if (cu.cu_header->abbrev_offset != 0) {
            os << "0x";
        }
        os << cu.cu_header->abbrev_offset << std::endl;

        os << "  Pointer Size: " << +cu.cu_header->pointer_size << std::endl;

        for (auto i : cu.DI_children) {

            os << cu.sInfo.DI_entries->at(i);
        }

        return os;
    }

  private:
    uint32_t index; // Compilation Unit index
    Elf32_Off offset;

    DWARF32_CU_header *cu_header = nullptr;
    const unsigned char *cu_end;

    Dwarf_Abbrev_table *abbrev_table;

    // DWARF_DI_entry children (index)
    std::vector<uint32_t> DI_children;

    // Parent DWARF ptr
    DWARF &parent_dwarf;

    DWARF_shared_info &sInfo;
};

class ELF; // Forward declaration

class DWARF {

    friend class DWARF_CU;

  public:
    DWARF();

    DWARF(const ELF *elf_instance);

    bool load_abbrev_tables(bool print = false);

    bool load_compilation_units(bool print = false);

    std::vector<DWARF_DI_entry> search_by_address(Elf64_Addr);

    // Overload << operator for DWARF
    friend std::ostream &operator<<(std::ostream &os, const DWARF &dwarf) {

        for (auto &cu : dwarf.compilation_units) {

            os << cu << " ";
        }

        return os;
    }

  private:
    const ELF *const elf;

    DWARF_shared_info sInfo;

    bool abbrev_tables_loaded = false;
    bool compilation_units_loaded = false;

    std::map<Dwarf32_Off, Dwarf_Abbrev_table *> abbrev_tables;

    std::vector<DWARF_CU> compilation_units;

    std::vector<DWARF_DI_entry> DI_entries; // Debugging Information entries

    std::vector<DWARF_attr> attributes;

    // std::map<DW_AT, DWARF_attr> attributes;

    //<offset, abbrev_table>

    // DI_entry address, DWARF_abbrev_decl *
    // std::unordered_map<const unsigned char *, DWARF_abbrev_decl *> abbrev_map;

    // Elf64_Addr, DI_entry address
    // std::unordered_multimap<Elf64_Addr, const unsigned char *> low_pc_map;

    // std::vector<DWARF_DI_entry> DI_entries;

    // bool read_CU(unsigned char *&ptr, bool print = false);

    DWARF_attr read_attr(unsigned char *&ptr, const DWARF_attr_spec &spec, bool print = false);

    // DWARF_DI_entry addr_to__DWARF_DI_entry(const unsigned char *ptr);
};