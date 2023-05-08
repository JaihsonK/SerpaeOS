#include "iso_9660.h"
#include "../file.h"
#include "string/string.h"

//Experimental ISO 9660 filesystem

#define uint8_t int8

struct directory_entry
{
    int8 directory_record_size;
    int8 EAR_length; //length of Extended Attribute Record
    int32_LSB_MSB sector_location;
    int32_LSB_MSB data_length;
    datetime_t datetime;
    struct
    {
        uint8_t hidden:1;
        uint8_t subdir:1;
        uint8_t associated_file:1;
        uint8_t format_in_ear:1; //if set, EAR contains info about file format
        uint8_t permissions_in_ear:1;
        uint8_t reserved:2;
        uint8_t not_final_record:1;
    }__attribute__((packed)) flags;
    int8 file_unit_size;
    int8 interleave_gap_size;
    int16_LSB_MSB volume_seq_num;
    int8 filename_length;
    //filename of variable length, than 1byte padding if filename length is odd
}__attribute__((packed));

struct volume_descriptor
{
    int8 desc_type;
    char id[5]; //always CD001
    int8 version; //always 1
    union
    {
        struct
        {
            char master_system[32];
            char boot_sys_id[32];
            int8 boot_sys[1977];
        }__attribute__((packed)) boot_record;
        struct
        {
            int8 unused; //always 0
            char master_system[32];
            char boot_sys_id[32];
            int8 unused2[8];
            int32_LSB_MSB volume_space_size;
            int8 unused3[32];
            int16_LSB_MSB volume_set_size;
            int16_LSB_MSB volume_sequence_num;
            int16_LSB_MSB sector_size;
            int32_LSB_MSB path_table_size;
            int32_LSB typeL_path_table_loc;
            int32_LSB opt_typeL_path_table_loc;
            int32_MSB typeM_path_table_loc;
            int32_MSB opt_typeM_path_table_loc;

            struct directory_entry root_dir;
            int8 padding;

            char volume_set_id[128];
            char publisher_id[128];
            char data_prep_id[128];
            char application_id[128];
            char copyright_file_id[37];
            char abstract_file_id[37];
            char bibliographic_file_id[37];
            datetime_f volume_creation;
            datetime_f volume_modification;
            datetime_f expiry_date; //if applicable
            datetime_f effective_date;
            int8 file_struct_version; //always 0x01
            int8 unused4[1 + 512 + 653];
        }__attribute__((packed)) primary_volume_desc;
    };
}__attribute__((packed));

struct filesystem iso9660_fs = 
{
    
};

struct filesystem *iso9660_init()
{
    strcpy(iso9660_fs.name, "ISO 9660");
    return &iso9660_fs;
}
