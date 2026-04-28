#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/stat.h>

#define JSMN_STATIC
#include "jsmn.h"

#define DGET_VERSION "1.0"
#define MAX_RETRIES 3

static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}

static const char *basename_from_url(const char *url)
{
    const char *p = strrchr(url, '/');
    return (p && *(p + 1)) ? p + 1 : "download";
}

static int download(const char *url, const char *path)
{
    CURL *c = curl_easy_init();
    if (!c) return 1;

    FILE *f = fopen(path, "wb");
    if (!f) return 1;

    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, f);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode r = curl_easy_perform(c);

    fclose(f);
    curl_easy_cleanup(c);

    return r != CURLE_OK;
}

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long s = ftell(f);
    rewind(f);

    char *b = malloc(s + 1);
    if (!b) return NULL;

    fread(b, 1, s, f);
    b[s] = 0;

    fclose(f);
    return b;
}

static void json_copy(char *dst, size_t dstsz, const char *json, jsmntok_t *t)
{
    int len = t->end - t->start;
    if (len >= (int)dstsz)
        len = dstsz - 1;

    memcpy(dst, json + t->start, len);
    dst[len] = 0;
}

static int install_file(const char *url, const char *out)
{
    int i = 0;
    while (i < MAX_RETRIES) {
        if (!download(url, out))
            return 0;
        i++;
    }
    return 1;
}

static int install_manifest(const char *file)
{
    char *json = read_file(file);
    if (!json) return 1;

    jsmn_parser p;
    jsmntok_t t[512];

    jsmn_init(&p);
    int r = jsmn_parse(&p, json, strlen(json), t, 512);
    if (r < 0) {
        free(json);
        return 1;
    }

    int fidx = -1;

    for (int i = 1; i < r; i++) {
        if (t[i].type == JSMN_STRING) {
            if (!strncmp(json + t[i].start, "files", t[i].end - t[i].start)) {
                fidx = i + 1;
                break;
            }
        }
    }

    if (fidx < 0 || t[fidx].type != JSMN_ARRAY) {
        free(json);
        return 1;
    }

    for (int i = fidx + 1; i < r && t[i].type == JSMN_OBJECT; i++) {

        char url[512] = {0};
        char out[256] = {0};

        int end = i + t[i].size * 2;

        for (int j = i + 1; j < end && j < r; j++) {
            if (t[j].type != JSMN_STRING)
                continue;

            if (!strncmp(json + t[j].start, "url", t[j].end - t[j].start))
                json_copy(url, sizeof(url), json, &t[j + 1]);

            if (!strncmp(json + t[j].start, "output", t[j].end - t[j].start))
                json_copy(out, sizeof(out), json, &t[j + 1]);
        }

        if (url[0] && out[0]) {
            if (install_file(url, out)) {
                free(json);
                return 1;
            }
        }

        i = end;
    }

    free(json);
    return 0;
}

static int fetch_url(const char *url)
{
    const char *name = basename_from_url(url);
    return download(url, name);
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;

    if (!strcmp(argv[1], "--version")) {
        printf("dget %s\n", DGET_VERSION);
        return 0;
    }

    if (!strcmp(argv[1], "install"))
        return install_manifest(argv[2]);

    return fetch_url(argv[1]);
}
