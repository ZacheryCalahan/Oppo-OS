/*
    Device specifications at:
    https://krinkinmu.github.io/2021/01/17/devicetree.html
*/

#include "headers/fdt.h"
#include "../kernel/stdlib/headers/intutil.h"
#include "../kernel/stdlib/headers/stdio.h"
#include "../kernel/stdlib/headers/stdlib.h"
#include "../kernel/stdlib/headers/scanner.h"

uint32_t parse_node(device_tree_node* node_ptr, scanner *s);


void** node_get_children(device_tree_node* node) {
    return node->children;
}

device_tree_property** node_get_properties(device_tree_node* node) {
    return node->properties;
}

device_tree_property* node_get_property(device_tree_node* node, char* name) {
    for (int i = 0; node[i].properties != NULL; i++) {
        if (strcmp(name, *node->properties[i]->name) == 0) {
            return node->properties[i];
        }
    }

    return NULL;
}

device_tree_node root_node; // Decompiled DTB, requires successful call to init_fdt to use.
char* fdt_string_block;
uint32_t fdt_string_block_size;

size_t fdt_size;
fdt_header fdt_map;         // Yes this being global feels wrong, yes i'm a terrible programmer.

uint32_t init_fdt(uint32_t address) {
    fdt_header *fdt_header_ptr = (fdt_header*) address; // Structure with FDT header
    
    // Initilize the header to little endian for reading
    fdt_map.magic               = uint32_be_to_le(fdt_header_ptr->magic);
    fdt_map.totalsize           = uint32_be_to_le(fdt_header_ptr->totalsize);
    fdt_map.off_dt_struct       = uint32_be_to_le(fdt_header_ptr->off_dt_struct);
    fdt_map.off_dt_strings      = uint32_be_to_le(fdt_header_ptr->off_dt_strings);
    fdt_map.off_mem_rsvmap      = uint32_be_to_le(fdt_header_ptr->off_mem_rsvmap);
    fdt_map.version             = uint32_be_to_le(fdt_header_ptr->version);
    fdt_map.last_comp_version   = uint32_be_to_le(fdt_header_ptr->last_comp_version);
    fdt_map.boot_cpuid_phys     = uint32_be_to_le(fdt_header_ptr->boot_cpuid_phys);
    fdt_map.size_dt_strings     = uint32_be_to_le(fdt_header_ptr->size_dt_strings);
    fdt_map.size_dt_struct      = uint32_be_to_le(fdt_header_ptr->magic);

    // Verify the magic number
    if (fdt_map.magic != FDT_MAGIC_NUMBER) {
        printf("Magic number not found for the FDT. Value: 0x%x", fdt_map.magic);
        return 1;
    }

    // Verify the version
    if (fdt_map.version != 17) {
        printf("FDT Version: %d not compatible with the parser.\n", fdt_map.version);
        return 2;
    }

    fdt_size = (size_t) fdt_map.totalsize;

    printf("Total size of the FDT: %u Bytes\n", fdt_size);

    // Load in the string block
    fdt_string_block = (char*) address + fdt_map.off_dt_strings;
    fdt_string_block_size = fdt_map.size_dt_struct;

    // Parse the memory reservation block
        // TODO
    
    // Parse the structure block (4 byte alignment)
    uint64_t* fdt_structure_block_ptr = (uint64_t *) (address + fdt_map.off_dt_struct); // Address of structure aligned to 4bytes.
    printf("Structure address: 0x%x\n", fdt_structure_block_ptr);

    scanner s;
    s.is_big_endian = true;
    s.scanner_address = fdt_structure_block_ptr;
    s.start_address = s.scanner_address;
    s.length = uint32_be_to_le(fdt_map.size_dt_struct);

    int result = parse_node(&root_node, &s);

    if (result != -1) { // Check that program returned on an end node.
        return result; // Success
    }
    
    return -1; // No end node found. Invalid structure
}

uint32_t parse_node(device_tree_node* node_ptr, scanner *s) {
    printf("Reading node!\n");

    // Loop through the stucture block until the end is reached.
    uint32_t current_token = 0;

    // Continue until token type is of FDT_END
    for (; current_token != FDT_END; ) {
        // Get info
        current_token = next_int(s);
        printf("Token found: %d\n", current_token);
        

        switch (current_token) {
            case FDT_BEGIN_NODE:
                printf("Begin node!\n");
                char* node_name = next_string(s);
                printf("Node name: \"%s\"\n", node_name);
                node_ptr->name = &node_name;
                align_scanner(s, 4);

                // Recursive call here!
                device_tree_node* child_node = kmalloc(sizeof(device_tree_node));
                int result = parse_node(child_node, s);
                if (result != 0) {
                    return result;
                }
                // Somehow, logic to implement the array of children should REALLY be handled here. 
                break;

            case FDT_END_NODE:
                printf("End node!\n");
                return 0;

            case FDT_PROP: // This should handle all prop tokens sequentially to correctly keep the array contigious
                printf("Prop node!\n");
                
                // Lookahead function
                uint32_t propCount = 0; // This is for kmalloc, to get a pointer to the array of properties to satisfy the device_tree_node data struct.
                uint32_t tag = current_token;
                void* tmp = get_current_pointer(s);
                for (int i = 0; ; i++) {
                    printf("Tag found: %x\n\n", tag);
                    if (tag != FDT_PROP) { // Reached the end of the chain
                        printf("Tag found: %x\n\n", tag);
                        break;
                    }

                    // Skip the whole property, and set up to read next token
                    uint32_t property_len = next_int(s);    // Read the length of the prop
                    skip_bytes(s, 4);                       // Skip the name offset for now
                    skip_bytes(s, property_len);            // Skip the data following the header
                    align_scanner(s, 4);                    // Realign the scanner to 4 bytes
                    propCount++;
                    tag = next_int(s);
                }
                printf("Tags found: %d\n", propCount);
                
                s->scanner_address = tmp; // Restore the properties of the scanner after the lookahead.
                device_tree_property* properties = kmalloc(propCount * sizeof(device_tree_property));

                // Insert each property in the array created earlier.
                for (int i = 0; i < propCount; i++) {
                    skip_bytes(s, 4); // Skip the tag, we already know what it is.
                    properties[i].length = next_int(s); // Assign the length of the data
                    char* str = next_string(s); // Assign the name to the properties
                    properties[i].name = &str;
                    printf("Name: %s\n", properties[i].name);
                }
                node_ptr->properties = &properties; // Assign the properties to the node.
                break;

            case FDT_NOP:
                printf("Nop node!\n");
                break;

            case FDT_END:
                printf("End!\n");
                return -1; // This tells the program to stop all recursive calls, as we've reached the end of the FDT.

            default:
                printf("Error traversing structure node. Illegal structure token: %d\n", current_token);
                return -2;
        }
    }
    return -16;

}