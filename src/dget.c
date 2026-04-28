#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

const char* get_filename_from_url(const char *url) {
    const char *last_slash = strrchr(url, '/');
    if (!last_slash || *(last_slash + 1) == '\0') {
        return "downloaded_file";
    }
    return last_slash + 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: dget <url>\n");
        return 1;
    }

    const char *url = argv[1];
    const char *filename = get_filename_from_url(url);

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to init curl\n");
        return 1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("Failed to open file for writing\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        printf("Download failed: %s\n", curl_easy_strerror(res));
    } else {
        printf("Downloaded: %s\n", filename);
    }

    curl_easy_cleanup(curl);
    fclose(fp);

    return 0;
}
