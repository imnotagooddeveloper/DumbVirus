#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#define HASH_FILE "hash.txt"
#define Sonuccuk "sonuc.txt"
#define MAX_HASH_LENGTH 65
#define MAX_FILENAME_LENGTH 256
#define INITIAL_HASH_LIST_SIZE 100

void calculate_sha256(const char *filename, char *output) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx) {
        perror("VP_MD_CTX , OLUŞTURAMADIM");
        strcpy(output, "anaa çöktüm ben C:");
        return;
    }

    const EVP_MD *md = EVP_sha256();
    if (EVP_DigestInit_ex(ctx, md, NULL) != 1) {
        perror("Digest Bana Sorun Çıkardı");
        strcpy(output, "BİŞEY OLDU AAAAAAA");
        EVP_MD_CTX_free(ctx);
        return;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Ben Niye Buna Erişemiyorum?");
        strcpy(output, "AAAAAAAAAAAAAAAAAAAAAA");
        EVP_MD_CTX_free(ctx);
        return;
    }

    char buffer[8192];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file))) {
        if (EVP_DigestUpdate(ctx, buffer, bytesRead) != 1) {
            perror("Bune?");
            strcpy(output, "AAAAAAA");
            fclose(file);
            EVP_MD_CTX_free(ctx);
            return;
        }
    }
    fclose(file);

    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        perror("Eee Ne Olduki");
        strcpy(output, "BİŞEY OLDU");
        EVP_MD_CTX_free(ctx);
        return;
    }

    for (unsigned int i = 0; i < hash_len; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[hash_len * 2] = '\0';

    EVP_MD_CTX_free(ctx);
}

bool is_malicious(const char *sha256hash, char (*hash_list)[MAX_HASH_LENGTH], int hash_count) {
    for (int i = 0; i < hash_count; i++) {
        if (strcmp(sha256hash, hash_list[i]) == 0) {
            return true;
        }
    }
    return false;
}

int main() {
    FILE *hash_file = fopen(HASH_FILE, "r");
    if (hash_file == NULL) {
        perror("Hash DOsyası Yokmu?");
        return 1;
    }

    FILE *result_file = fopen(Sonuccuk, "w");
    if (result_file == NULL) {
        perror("Sonuçcuk Doyşaşını Oyuştuyamıyom :c");
        fclose(hash_file);
        return 1;
    }

    char (*hash_list)[MAX_HASH_LENGTH] = malloc(INITIAL_HASH_LIST_SIZE * MAX_HASH_LENGTH);
    if (!hash_list) {
        perror("Listecik için Yemcikte Alan Oyuştuyamadım :C");
        fclose(hash_file);
        fclose(result_file);
        return 1;
    }

    int hash_count = 0;
    int hash_list_size = INITIAL_HASH_LIST_SIZE;

    while (fgets(hash_list[hash_count], MAX_HASH_LENGTH, hash_file)) {
        hash_list[hash_count][strcspn(hash_list[hash_count], "\n")] = '\0';
        hash_count++;

        if (hash_count >= hash_list_size) {
            hash_list_size *= 2; 
            hash_list = realloc(hash_list, hash_list_size * MAX_HASH_LENGTH);
            if (!hash_list) {
                perror("Hash Doyşaşı için Yey Aytdıyamadım :c");
                fclose(hash_file);
                fclose(result_file);
                return 1;
            }
        }
    }
    fclose(hash_file);

    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Klasöye eyişemiyom");
        free(hash_list);
        fclose(result_file);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            continue;
        }

        char *filename = strdup(entry->d_name);
        if (!filename) {
            perror("Memoyiyi Ayaylayamadım :C (üzgün surat)");
            closedir(dir);
            free(hash_list);
            fclose(result_file);
            return 1;
        }

        char sha256hash[MAX_HASH_LENGTH];
        calculate_sha256(filename, sha256hash);

        if (strcmp(sha256hash, "BUNE AAAAAAa") != 0) {
            if (is_malicious(sha256hash, hash_list, hash_count)) {
                fprintf(result_file, "%-20s          SİL SİL SİİİİİİİİİİL\n", filename);
            } else {
                fprintf(result_file, "%-20s          TM DURSUN\n", filename);
            }
        }

        free(filename);
    }

    closedir(dir);
    free(hash_list);
    fclose(result_file);
    printf("annee bitti.");
    return 0;
}
