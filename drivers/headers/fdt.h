/*
    Documentation for the device tree specification:
    https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html
*/

#ifndef FDT_H
#define FDT_H

#include <stdint.h>

#define FDT_MAGIC_NUMBER 0xd00dfeed


/***************************************************************/
/*                  In Memory Data Structures                  */
/***************************************************************/
// These structs are intended to be superimposed directly into memory for reading.
// All variables are in big-endian format, must be converted for use in RISC-V
typedef struct {
    uint32_t magic;                 // Magic number
    uint32_t totalsize;             // Total size in bytes of the devicetree data structure, including the header, mem reservation block, structure block, and strings block, including all free space in between.
    uint32_t off_dt_struct;         // Offset in bytes of the structure block from the beginning of the header
    uint32_t off_dt_strings;        // Offset in bytes of the string block from the beginning of the header
    uint32_t off_mem_rsvmap;        // Offset in bytes of the memory reservation block from the beginning of the header
    uint32_t version;               // Version of the devicetree structure
    uint32_t last_comp_version;     // The lowest version of the devicetree data structure that is backwards compatible
    uint32_t boot_cpuid_phys;       // Physical ID of the systems boot CPU
    uint32_t size_dt_strings;       // Length in bytes of the strings block
    uint32_t size_dt_struct;        // Length in bytes of the structure block
} fdt_header; 

#define FDT_BEGIN_NODE  0x00000001  // Marks the beginning of a node's representation
#define FDT_END_NODE    0x00000002  // Marks the end of a node's representation
#define FDT_PROP        0x00000003  // Marks the beginning of one property in a devicetree
#define FDT_NOP         0x00000004  // Marks that the program should ignore this token
#define FDT_END         0x00000009  // Marks the end of the structure block

typedef struct {
    uint64_t address;               // Physical address of the memory reservation block
    uint64_t size;                  // Size of the memory reserved block
} fdt_reserve_entry;

typedef struct {
    uint32_t tag;
    char name[];
} fdt_node_header;

typedef struct {
    uint32_t len;                   // Length of the property in bytes, including zero if it's an empty property
    uint32_t nameoff;               // Offset into the strings block at which the property name is stored
} fdt_property;

/***************************************************************/
/*                   Parsed Data Structures                    */
/***************************************************************/
// All structs that have been parsed for use belong here.

typedef struct {
    uint32_t length;
    char** name;
    void** data;
} device_tree_property;

typedef struct {
    char** name;
    uint32_t child_count;
    void** children;
    uint32_t property_count;
    device_tree_property** properties;
} device_tree_node;




uint32_t init_fdt(uint32_t address);


#endif