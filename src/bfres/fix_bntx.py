with open('bfres.cpp', 'r') as f:
    content = f.read()

# Find the start of the duplicate debug blocks
start_marker = "// Check known offsets from manual inspection"
end_marker = "// BNTX uses ResFile format with internal sections (BRTI, _STR, texture data)"

idx1 = content.find(start_marker)
idx2 = content.find(end_marker, content.find(start_marker))

if idx1 != -1 and idx2 != -1:
    # Find the exact end of the duplicate block
    # The duplicate block ends right before "BNTX uses ResFile format"
    # But there's another occurrence of "BNTX uses ResFile format" earlier
    # Let's find the second occurrence
    
    # Actually, let's find the SectionCandidate struct which comes right after the duplicate blocks
    marker = "struct SectionCandidate {"
    idx3 = content.find(marker, content.find("BNTX uses ResFile format with internal sections"))
    
    if idx3 != -1:
        # The duplicate block is between the first "Check known offsets" and the struct
        # But there are TWO "Check known offsets" sections - we want to remove the second one
        # which is right before the struct
        
        # Find all occurrences of "Check known offsets"
        indices = []
        start = 0
        while True:
            idx = content.find("// Check known offsets from manual inspection", start)
            if idx == -1:
                break
            indices.append(idx)
            start = idx + 1
        
        print(f"Found {len(indices)} occurrences")
        for i, idx in enumerate(indices):
            print(f"  {i}: at {idx}")
        
        # The last one before the struct is the one to remove
        if len(indices) >= 2:
            last_idx = indices[-1]
            # Find the end of this block - it ends before "BNTX uses ResFile format" 
            # but there are multiple "BNTX uses ResFile format" comments
            # Let's find the one right before the struct
            struct_idx = content.find("struct SectionCandidate {", indices[-1])
            if struct_idx != -1:
                # The block to remove is from the second occurrence to the struct
                block_start = indices[-1]
                block_end = struct_idx
                print(f"Block to remove: {block_start} to {struct_idx}")
                
                # Also need to handle the std::fprintf line before the duplicate block
                fprintf_idx = content.rfind("std::fprintf(stderr, \"BNTX at offset", 0, indices[-1])
                if fprintf_idx != -1:
                    print(f"Found fprintf at {fprintf_idx}")
                    
                    # Replace from fprintf_idx to struct_idx with new content
                    old_content = content[fprintf_idx:struct_idx]
                    new_content = '''    std::fprintf(stderr, "BNTX at offset %zu, size %zu, magic=0x%08x\n",
                 bntx_section.offset, bntx_size, magic);

    // Scan for BRTI magic "BRTI" (0x49545242) and _STR magic "_STR" (0x5254535F)
    uint32_t brti_offset = 0, brti_size = 0;
    for (size_t offset = 64; offset + 4 <= bntx_size; ++offset) {
        uint32_t sig = *reinterpret_cast<const uint32_t*>(bntx_data + offset);
        if (sig == 0x49545242) {  // "BRTI"
            brti_offset = static_cast<uint32_t>(offset);
            if (offset + 8 <= bntx_size) {
                brti_size = *reinterpret_cast<const uint32_t*>(bntx_data + offset + 4);
            }
            std::fprintf(stderr, "[DEBUG] Found BRTI magic at offset %u, size %u\n", brti_offset, brti_size);
            break;
        }
    }

    uint32_t str_offset = 0, str_size = 0;
    for (size_t offset = 64; offset + 4 <= bntx_size; ++offset) {
        uint32_t sig = *reinterpret_cast<const uint32_t*>(bntx_data + offset);
        if (sig == 0x5254535F) {  // "_STR"
            str_offset = static_cast<uint32_t>(offset);
            if (offset + 8 <= bntx_size) {
                str_size = *reinterpret_cast<const uint32_t*>(bntx_data + offset + 4);
            }
            std::fprintf(stderr, "[DEBUG] Found _STR magic at offset %u, size %u\n", str_offset, str_size);
            break;
        }
    }
'''
                    content = content[:fprintf_idx] + new_content + content[struct_idx:]
                    with open('bfres.cpp', 'w') as f:
                        f.write(content)
                    print("Replaced successfully!")
                else:
                    print("Could not find fprintf line")
            else:
                print("Could not find struct")
        else:
            print("Less than 2 occurrences found")
    else:
        print("Could not find struct SectionCandidate")

