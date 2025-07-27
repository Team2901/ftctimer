#include "app.h"

static const char *TAG = "SPIFFS_APP";

// Function to check if a string ends with a specific suffix
bool ends_with(const char *str, const char *suffix)
{
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return str_len >= suffix_len && !strcasecmp(str + str_len - suffix_len, suffix);
}

// Second pass: Fill the allocated memory with formatted strings
char *getFilesBySuffixJson(const char *pSuffix)
{
    size_t nSuffixLen = strlen(pSuffix);
    char *pResult = NULL;
    size_t nSize = 0;
    struct dirent *entry;
    char *pCurrent = NULL;
    char cPrefix = '[';

    for (int pass = 0; pass < 2; pass++)
    {
        DIR *dir = opendir("/spiffs");
        if (!dir)
        {
            return NULL;
        }

        if (pass > 0)
        {
            pCurrent = pResult = malloc(nSize + 2); // Account for the [] and the null but one less comma
            if (pResult == NULL)
            {
                closedir(dir);
                return pResult;
            }
        }
        while ((entry = readdir(dir)) != NULL)
        {
            if (ends_with(entry->d_name, pSuffix))
            {
                int nEntryLen = strlen(entry->d_name) - nSuffixLen;
                if (pass == 0)
                {
                    nSize += nEntryLen + 3; // Add quotes and comma
                }
                else
                {
                    // First character will be '[' and subsquent ones between files will be ','
                    *pCurrent++ = cPrefix;
                    cPrefix = ',';

                    *pCurrent++ = '"';
                    strcpy(pCurrent, entry->d_name);
                    pCurrent += nEntryLen;
                    *pCurrent++ = '"';
                    ESP_LOGI(TAG, "Copying %s", entry->d_name);
                }
            }
        }
        closedir(dir);
        // If there were no patterns then we don't need to do the second pass just to return nothing.
        if (nSize == 0)
        {
            return NULL;
        }
    }
    // Terminate with the closing bracket and a null
    strcpy(pCurrent, "]");
    ESP_LOGI(TAG, "Result %s", pResult);
    return pResult;
}

// // Function to list files in SPIFFS
// void list_spiffs_files(void)
// {
//     ESP_LOGI(TAG, "Listing Directory");
//     DIR *dir = opendir("/spiffs");
//     if (!dir)
//     {
//         ESP_LOGE(TAG, "Failed to open directory");
//         return;
//     }

//     struct dirent *entry;
//     struct stat entry_stat;
//     char full_path[256];

//     while ((entry = readdir(dir)) != NULL)
//     {
//         strcpy(full_path, "/spiffs/");
//         strncat(full_path, entry->d_name, sizeof(full_path) - 9);
//         ESP_LOGI(TAG, "READDIR ENTRY: %s", full_path);

//         // snprintf(full_path, sizeof(full_path), "/spiffs/%s", entry->d_name);
//         if (stat(full_path, &entry_stat) == 0)
//         {
//             ESP_LOGI(TAG, "File: %s, Size: %ld bytes", entry->d_name, entry_stat.st_size);
//         }
//         else
//         {
//             // ESP_LOGE(TAG, "Failed to get stats for %s", entry->d_name);
//             ESP_LOGE(TAG, "Failed to get stats for %s", full_path);
//         }
//     }

//     closedir(dir);
// }
bool makeSpiffsFileName(const char *filename, char *buffer, int len)
{
    int used;
    int fnLen;
    // Make sure they gave us something to write into
    if (buffer == NULL)
    {
        return false;
    }
    // Put in the /spiffs/ prefix
    used = snprintf(buffer, len, "/spiffs/");
    // Figure out how much more we need to copy
    fnLen = strlen(filename);

    // Make sure it will fit, otherwise we bail
    if (fnLen >= len - used)
    {
        return false;
    }
    // Ok we know it fits, just safely copy it into the target buffer
    strncat(buffer + used, filename, len - used);
    return (true);
}

int GetFileSize(const char *filename)
{
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;

    // Construct the file name that we will be opening
    if (!makeSpiffsFileName(filename, filepath, sizeof(filepath) - 1))
    {
        ESP_LOGE(TAG, "Unable to make the filename for %s", filename);
        return -1;
    }
    // Make sure it exists
    if (stat(filepath, &file_stat) == -1)
    {
        return -1;
    }
    return file_stat.st_size;
}

int ReadFile(const char *filename, char *buffer, int len)
{
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;
    FILE *file = NULL;
    int readsize;

    // Construct the file name that we will be opening
    if (!makeSpiffsFileName(filename, filepath, sizeof(filepath) - 1))
    {
        ESP_LOGE(TAG, "Unable to make the filename for %s", filename);
        return -1;
    }
    // Make sure it exists
    if (stat(filepath, &file_stat) == -1)
    {
        ESP_LOGI(TAG, "Unable to find file: %s", filepath);
        return -1;
    }

    if (file_stat.st_size > len)
    {
        ESP_LOGE(TAG, "File too large for buffer %ld > %d", file_stat.st_size, len);
        return -1;
    }
    file = fopen(filepath, "r");
    if (!file)
    {
        ESP_LOGE(TAG, "Unable to open file %s", filepath);
        return -1;
    }
    readsize = fread(buffer, 1, file_stat.st_size, file);
    if (readsize != file_stat.st_size)
    {
        ESP_LOGE(TAG, "Bad Read Length got %d but expected %ld", readsize, file_stat.st_size);
    }
    return readsize;
}

void FileSystem_Init(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    // list_spiffs_files();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
}
