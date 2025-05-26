#include <fat32.h>
#include <printk.h>
#include <virtio.h>
#include <string.h>
#include <mbr.h>
#include <mm.h>

struct fat32_bpb fat32_header;
struct fat32_volume fat32_volume;

uint8_t fat32_buf[VIRTIO_BLK_SECTOR_SIZE];
uint8_t fat32_table_buf[VIRTIO_BLK_SECTOR_SIZE];

uint64_t cluster_to_sector(uint64_t cluster) {
    return (cluster - 2) * fat32_volume.sec_per_cluster + fat32_volume.first_data_sec;
}

uint64_t sector_to_cluster(uint64_t sector) {
    return (sector - fat32_volume.first_data_sec) / fat32_volume.sec_per_cluster + 2;
}

uint32_t next_cluster(uint64_t cluster) {
    uint64_t fat_offset = cluster * 4;
    uint64_t fat_sector = fat32_volume.first_fat_sec + fat_offset / VIRTIO_BLK_SECTOR_SIZE;
    virtio_blk_read_sector(fat_sector, fat32_table_buf);
    int index_in_sector = fat_offset % (VIRTIO_BLK_SECTOR_SIZE / sizeof(uint32_t));
    return *(uint32_t*)(fat32_table_buf + index_in_sector);
}

void fat32_init(uint64_t lba, uint64_t size) {
    virtio_blk_read_sector(lba, (void*)&fat32_header);
    fat32_volume.first_fat_sec = lba + fat32_header.rsvd_sec_cnt;
    fat32_volume.sec_per_cluster = fat32_header.sec_per_clus;
    fat32_volume.first_data_sec = fat32_volume.first_fat_sec + fat32_header.num_fats * fat32_header.fat_sz32;
    fat32_volume.fat_sz = fat32_header.fat_sz32;
    virtio_blk_read_sector(fat32_volume.first_fat_sec, fat32_buf);
    // if (*(uint32_t *)fat32_buf != 0x0fFFFFf8)
    // {
    //     printk("%lx %lx\n", fat32_volume.first_fat_sec, *(uint32_t *)fat32_buf);
    //     printk("fat32_init: FAT volume is invalid\n");
    // }
    // else {
    //     printk("fat32_init: FAT volume is valid\n");
    // }
}

int is_fat32(uint64_t lba) {
    virtio_blk_read_sector(lba, (void*)&fat32_header);
    if (fat32_header.boot_sector_signature != 0xaa55) {
        return 0;
    }
    return 1;
}

int next_slash(const char* path) {  // util function to be used in fat32_open_file
    int i = 0;
    while (path[i] != '\0' && path[i] != '/') {
        i++;
    }
    if (path[i] == '\0') {
        return -1;
    }
    return i;
}

void to_upper_case(char *str) {     // util function to be used in fat32_open_file
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

struct fat32_file fat32_open_file(const char *path) {
    struct fat32_file file = {0};
    char filename[12];
    memset(filename, 0, 12);
    strcpy(filename, path + 1 + next_slash(path + 1) + 1);
    to_upper_case(filename);
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
    uint64_t root_dir_sec = fat32_volume.first_data_sec;
    uint32_t cluster = fat32_header.root_clus;
    while (1) {
        // printk("sector: %lx\n", cluster_to_sector(cluster));
        virtio_blk_read_sector(cluster_to_sector(cluster), fat32_buf);
        for (int i = 0; i < FAT32_ENTRY_PER_SECTOR; i++) {
            // printk("%d %s\n", i, entry[i].name);
            if (entry[i].name[0] == 0x00) {
                // continue;
                return file;  // not found
            }
            if (entry[i].name[0] == 0xe5) {
                continue;  // deleted
            }
            if (entry[i].attr & 0x08) {
                continue;  // long name
            }
            if (entry[i].attr & 0x10) {
                continue;  // directory
            }
            char exixt_filename[12];
            memset(&exixt_filename, 0, 12);
            for (int j = 0; j < 8; j++) {
                exixt_filename[j] = entry[i].name[j] == ' ' ? '\0' : entry[i].name[j];
            }
            if (memcmp(exixt_filename, filename, 8) == 0) {
                // printk("file found\n");
                file.cluster = entry[i].startlow | ((uint32_t)entry[i].starthi << 16);
                file.dir.cluster = cluster;
                file.dir.index = i;
                return file;
            }
        }
        cluster = next_cluster(cluster);
    }
    return file;
}

int64_t fat32_lseek(struct file* file, int64_t offset, uint64_t whence) {
    if (whence == SEEK_SET) {
        file->cfo = offset;
    } else if (whence == SEEK_CUR) {
        file->cfo = file->cfo + offset;
    } else if (whence == SEEK_END) {
        /* Calculate file length */
        struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
        virtio_blk_read_sector(cluster_to_sector(file->fat32_file.cluster), fat32_buf);
        uint32_t file_sz = entry[file->fat32_file.dir.index].size;
        file->cfo = entry[file_sz].size + offset;
    } else {
        printk("fat32_lseek: whence not implemented\n");
        while (1);
    }
    return file->cfo;
}

uint64_t fat32_table_sector_of_cluster(uint32_t cluster) {
    return fat32_volume.first_fat_sec + cluster / (VIRTIO_BLK_SECTOR_SIZE / sizeof(uint32_t));
}

int64_t fat32_read(struct file* file, void* buf, uint64_t len) {
    // printk("begin read\n");
    uint32_t cluster = file->fat32_file.cluster;
    uint64_t sector = cluster_to_sector(cluster);
    uint32_t dir_index = file->fat32_file.dir.index;
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
    virtio_blk_read_sector(cluster_to_sector(file->fat32_file.dir.cluster), fat32_buf);
    uint32_t file_sz = entry[dir_index].size;
    // printk("The size of the file is %ld\n", file_sz);
    if (file->cfo + len > file_sz) {
        len = file_sz - file->cfo;
    }
    if (len <= 0) {
        return 0;
    }
    int64_t cfo = file->cfo;
    while (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
        cluster = next_cluster(cluster);
        cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
    }
    uint64_t read_len = 0;
    while (read_len < len) {
        sector = cluster_to_sector(cluster);
        // uint64_t offset = cfo % (fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE);
        uint64_t new_read_len = fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE - cfo;
        new_read_len = read_len + new_read_len > len ? len - read_len : new_read_len;
        virtio_blk_read_sector(sector, fat32_buf);
        memcpy(buf + read_len, fat32_buf + cfo, new_read_len);
        cfo += new_read_len;
        read_len += new_read_len;
        // printk("read_len: %ld, new_read_len: %ld, cfo: %ld, len: %d\n", read_len, new_read_len, cfo, len);
        if (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
            cluster = next_cluster(cluster);
            cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
        }
    }
    // printk("Completed read\n");
    file->cfo += read_len;
    return read_len;
}

int64_t fat32_write(struct file* file, const void* buf, uint64_t len) {
    uint32_t cluster = file->fat32_file.cluster;
    uint64_t sector = cluster_to_sector(cluster);
    uint32_t dir_index = file->fat32_file.dir.index;
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf; 
    virtio_blk_read_sector(cluster_to_sector(file->fat32_file.dir.cluster), fat32_buf);
    uint32_t file_sz = entry[dir_index].size;
    if (len <= 0) {
        return 0;
    }
    int64_t cfo = file->cfo;
    while (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
        cluster = next_cluster(cluster);
        cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
    }
    uint64_t write_len = 0;
    while (write_len < len) {
        sector = cluster_to_sector(cluster);
        virtio_blk_read_sector(sector, fat32_buf);
        uint64_t new_write_len = fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE - cfo;
        new_write_len = write_len + new_write_len > len ? len - write_len : new_write_len;
        memcpy(fat32_buf + cfo, buf + write_len, new_write_len);
        virtio_blk_write_sector(sector, fat32_buf);
        cfo += new_write_len;
        write_len += new_write_len;
        if (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
            cluster = next_cluster(cluster);
            cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
        }
    }
    file->cfo += write_len;
    if (file->cfo > file_sz) {
        entry[dir_index].size = file->cfo;
        // virtio_blk_write_sector(sector, fat32_buf);
    }
    return 0;
}