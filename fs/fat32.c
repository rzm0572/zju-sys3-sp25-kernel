#include <private_kdefs.h>
#include <fat32.h>
#include <printk.h>
#include <virtio.h>
#include <string.h>
#include <mbr.h>
#include <mm.h>
#include <sys_dirent.h>

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
    (void)size;
    virtio_blk_read_sector(lba, (void*)&fat32_header);
    fat32_volume.first_fat_sec = lba + fat32_header.rsvd_sec_cnt;
    fat32_volume.sec_per_cluster = fat32_header.sec_per_clus;
    fat32_volume.first_data_sec = fat32_volume.first_fat_sec + fat32_header.num_fats * fat32_header.fat_sz32;
    fat32_volume.fat_sz = fat32_header.fat_sz32;
    virtio_blk_read_sector(fat32_volume.first_fat_sec, fat32_buf);
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

int parse_dirname(const char *path, int *index, char *dirname) {
    memset(dirname, 0, 12);
    int ind = *index;
    if (path[ind] == '/') {
        ind++;
    }
    int next_slash_index = next_slash(path + ind);
    if (next_slash_index == -1) {
        strcpy(dirname, path + ind);
        *index += strlen(dirname);
        to_upper_case(dirname);
        return 1;
    }
    int len = next_slash_index - ind;
    strncpy(dirname, path + ind, len);
    *index += len + 1;
    dirname[len] = '\0';
    to_upper_case(dirname);
    return path[next_slash_index + 1] == '\0';
}

struct fat32_dir open_dir(const char *path, char* filename) {
    struct fat32_dir dir = {0};
    char dirname[12];
    int index = 1 + next_slash(path + 1) + 1;
    int reach_end = parse_dirname(path, &index, dirname);

    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
    uint32_t cluster = fat32_header.root_clus;
    filename[0] = '\0';

    if (dirname[0] == '\0') {
        dir.cluster = 0;
        dir.index = -1;
        return dir;
    }

    while (!reach_end) {
        int found = 0;
        while (cluster != INVALID_CLUSTER) {
            // printk("cluster: %x, filename: %s\n", cluster, dirname);
            virtio_blk_read_sector(cluster_to_sector(cluster), fat32_buf);
            for (unsigned long i = 0; i < FAT32_ENTRY_PER_SECTOR; i++) {
                if (entry[i].name[0] == 0x00) {
                    dir.cluster = INVALID_CLUSTER;
                    dir.index = -1;
                    return dir;  // not found
                }
                if (entry[i].name[0] == 0xe5) {
                    continue;
                }
                if (!(entry[i].attr & 0x10)) {
                    continue;  // not a directory
                }
                char subdirname[12];
                memset(&subdirname, 0, 12);
                for (int j = 0; j < 8; j++) {
                    subdirname[j] = entry[i].name[j] == ' ' ? '\0' : entry[i].name[j];
                }
                if (memcmp(subdirname, dirname, 8) == 0) {
                    cluster = entry[i].startlow | ((uint32_t)entry[i].starthi << 16);
                    found = 1;
                    break;
                }
            }

            if (found) {
                break;
            }

            cluster = next_cluster(cluster);
        }

        reach_end = parse_dirname(path, &index, dirname);
    }

    strcpy(filename, dirname);
    dir.cluster = cluster;
    dir.index = -1;
    
    return dir;
}

struct fat32_file fat32_open_file(const char *path) {
    struct fat32_file file = {0};
    char filename[12];

    struct fat32_dir dir = open_dir(path, filename);
    // printk("filename: %s\n", filename);
    // printk("cluster: %x\n", dir.cluster);
    if (dir.cluster == INVALID_CLUSTER) {
        return file;
    }
    if (dir.cluster == 0) {
        printk(ERR("fat32_open_file", "%s: not a file\n"), path);
        return file;
    }

    uint32_t cluster = dir.cluster;
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;

    while (cluster != INVALID_CLUSTER) {
        virtio_blk_read_sector(cluster_to_sector(cluster), fat32_buf);
        for (unsigned long i = 0; i < FAT32_ENTRY_PER_SECTOR; i++) {
            if (entry[i].name[0] == 0x00) {
                printk(ERR("fat32_open_file", "%s: no such file or directory\n"), path);
                return file;  // not found
            }
            if (entry[i].name[0] == 0xe5) {
                continue;  // deleted
            }
            if (entry[i].attr & 0x08) {
                continue;  // long name
            }
            if (entry[i].attr & 0x10) {
                printk(ERR("fat32_open_file", "%lu: directory\n"), i);
                continue;  // directory
            }
            char exist_filename[12];
            memset(&exist_filename, 0, 12);
            for (int j = 0; j < 8; j++) {
                exist_filename[j] = entry[i].name[j] == ' ' ? '\0' : entry[i].name[j];
            }
            if (memcmp(exist_filename, filename, 8) == 0) {
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

struct fat32_file fat32_open_dir(const char *path) {
    struct fat32_file file = {0};
    char dirname[12];

    struct fat32_dir dir = open_dir(path, dirname);
    if (dir.cluster == INVALID_CLUSTER) {
        printk(ERR("fat32_open_dir", "%s: no such file or directory\n"), path);
        return file;
    }
    if (dir.cluster == 0) {
        file.cluster = fat32_header.root_clus;
        file.dir.cluster = 0;
        file.dir.index = -1;
        return file;
    }

    uint32_t cluster = dir.cluster;
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
    while (cluster != INVALID_CLUSTER) {
        virtio_blk_read_sector(cluster_to_sector(cluster), fat32_buf);
        for (unsigned long i = 0; i < FAT32_ENTRY_PER_SECTOR; i++) {
            // printk("%lu %s\n", i, entry[i].name);
            if (entry[i].name[0] == 0x00) {
                printk(ERR("fat32_open_dir", "%s: no such file or directory\n"), path);
                return file;  // not found
            }
            if (entry[i].name[0] == 0xe5) {
                continue;  // deleted
            }
            if (entry[i].attr & 0x08) {
                continue;  // long name
            }
            if (entry[i].attr & 0x10) {
                char exist_dirname[12];
                memset(&exist_dirname, 0, 12);
                for (int j = 0; j < 8; j++) {
                    exist_dirname[j] = entry[i].name[j] == ' ' ? '\0' : entry[i].name[j];
                }
                if (memcmp(exist_dirname, dirname, 8) == 0) {
                    file.cluster = entry[i].startlow | ((uint32_t)entry[i].starthi << 16);
                    file.dir.cluster = cluster;
                    file.dir.index = i;
                    return file;
                }
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
        printk(ERR("fat32_lseek", "whence not implemented\n"));
        while (1);
    }
    return file->cfo;
}

uint64_t fat32_table_sector_of_cluster(uint32_t cluster) {
    return fat32_volume.first_fat_sec + cluster / (VIRTIO_BLK_SECTOR_SIZE / sizeof(uint32_t));
}

int64_t fat32_read(struct file* file, void* buf, uint64_t len) {
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
    uint64_t cfo = file->cfo;
    while (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
        cluster = next_cluster(cluster);
        cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
    }
    uint64_t read_len = 0;
    while (read_len < len) {
        sector = cluster_to_sector(cluster);
        uint64_t new_read_len = fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE - cfo;
        new_read_len = read_len + new_read_len > len ? len - read_len : new_read_len;
        virtio_blk_read_sector(sector, fat32_buf);
        memcpy(buf + read_len, fat32_buf + cfo, new_read_len);
        cfo += new_read_len;
        read_len += new_read_len;
        if (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
            cluster = next_cluster(cluster);
            cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
        }
    }
    file->cfo += read_len;
    return read_len;
}

int64_t fat32_read_dir(struct file* file, void* buf, uint64_t len) {
    uint32_t cluster = file->fat32_file.cluster;
    uint64_t sector = cluster_to_sector(cluster);
    uint32_t dir_index = file->fat32_file.dir.index;
    struct fat32_dir_entry *entry = (struct fat32_dir_entry*)fat32_buf;
    uint32_t file_sz = 0;

    if (file->fat32_file.dir.cluster == 0) {
        file_sz = 512; // TODO
    } else {
        virtio_blk_read_sector(cluster_to_sector(file->fat32_file.dir.cluster), fat32_buf);
        file_sz = entry[dir_index].size;
    }

    if (file->cfo + len > file_sz) {
        len = file_sz - file->cfo;
    }
    if (len <= 0) {
        return 0;
    }
    uint64_t cfo = file->cfo;
    while (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
        cluster = next_cluster(cluster);
        cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
    }
    uint64_t read_len = 0;
    int curr_entry_cnt = 0;
    while (read_len < len) {
        sector = cluster_to_sector(cluster);
        uint64_t new_read_len = fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE - cfo;
        new_read_len = read_len + new_read_len > len ? len - read_len : new_read_len;
        virtio_blk_read_sector(sector, fat32_buf);
        struct fat32_dir_entry *sub_entry = (struct fat32_dir_entry*)(fat32_buf + cfo);
        int reach_end_entry = 0;
        for (unsigned long i = 0; i < new_read_len / sizeof(struct fat32_dir_entry); i++) {
            if (sub_entry[i].name[0] == 0x00) {
                new_read_len = i * sizeof(struct fat32_dir_entry);
                reach_end_entry = 1;
                break;
            }
            if (sub_entry[i].name[0] == 0xe5) {
                continue;
            }
            if (sub_entry[i].attr & 0x08) {
                continue;
            }
            struct dirent *dirent = (struct dirent*)buf + curr_entry_cnt;
            memset(dirent, 0, sizeof(struct dirent));
            memcpy(dirent->d_name, sub_entry[i].name, 12);
            dirent->d_off = (curr_entry_cnt + 1) * sizeof(struct dirent);
            dirent->d_reclen = sizeof(struct dirent);
            if (sub_entry[i].attr & 0x10) {
                dirent->d_type = DT_DIR;
            } else {
                dirent->d_type = DT_REG;
            }
            curr_entry_cnt++;
        }
        cfo += new_read_len;
        read_len += new_read_len;
        if (reach_end_entry) {
            break;
        }

        if (cfo >= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE) {
            cluster = next_cluster(cluster);
            cfo -= fat32_volume.sec_per_cluster * VIRTIO_BLK_SECTOR_SIZE;
        }
    }
    file->cfo += read_len;
    return curr_entry_cnt * sizeof(struct dirent);
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
    uint64_t cfo = file->cfo;
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
