typedef struct ggHttpResponse {
    unsigned int status;
    GString *body;
    GHashTable* headers;
} ggHttpResponse;

gchar* gg_status_code_to_message (guint status) {
    switch (status) {
        case 200:
            return "OK";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        default:
            return "Unsupported Status Code";
    }
}

void gg_write (ggHttpResponse *response, char *chunk) {
    g_string_append(response->body, chunk);
}

void gg_write_len (ggHttpResponse *response, char *chunk, guint len) {
    g_string_append_len(response->body, chunk, len);
}

gchar* gg_get_response_header(ggHttpResponse *response, gchar *key) {
    return (gchar *) g_hash_table_lookup(response->headers, key);
}

void gg_set_response_header(ggHttpResponse *response, gchar *key, gchar *value) {
    g_hash_table_insert(response->headers, g_strdup(key), g_strdup(value));
}

void gg_set_response_header_num(ggHttpResponse *response, gchar *key, gint value) {
    gchar *value_string = (gchar*) malloc(32);
    g_ascii_dtostr(value_string, 32, value);
    g_hash_table_insert(response->headers, g_strdup(key), value_string);
}

ggHttpResponse* gg_http_response_new() {
    ggHttpResponse *response = malloc(sizeof(ggHttpResponse));
    response->headers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    gg_set_response_header(response, "Server", "Glassgow/pre-alpha");

    //TODO: what's the best default allocation size for buffer?
    response->body = g_string_sized_new(1024);

    return response;
}

void gg_http_response_free(ggHttpResponse *response) {
    //Free the headers; hrm, what to do about dynamically allocated values/keys?
    g_hash_table_destroy(response->headers);

    g_string_free(response->body, TRUE);

    free(response);
}
