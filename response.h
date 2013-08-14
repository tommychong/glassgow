typedef struct ggHttpResponse {
    unsigned int status;
    char *body;
    gint body_len;
    //headers;
    //http_version version;
    GHashTable* headers;
} ggHttpResponse;

gchar* gg_get_response_header(ggHttpResponse *response, gchar *key) {
    return (gchar *) g_hash_table_lookup(response->headers, key);
}

void gg_set_response_header(ggHttpResponse *response, gchar *key, gchar *value) {
    g_hash_table_insert(response->headers, key, value);
}

void gg_set_response_header_num(ggHttpResponse *response, gchar *key, gint value) {
    gchar *value_string = (gchar*) malloc(32);
    g_ascii_dtostr(value_string, 32, value);
    g_hash_table_insert(response->headers, key, value_string);
}

ggHttpResponse* gg_http_response_new() {
    ggHttpResponse *response = malloc(sizeof(ggHttpResponse));
    response->headers = g_hash_table_new(g_str_hash, g_str_equal);
    gg_set_response_header(response, "Server", "Glassgow pre-alpha");
    response->body_len = -1; //Needed for binary data like pictures

    return response;
}

void gg_http_response_free(ggHttpResponse *response) {
    //Free the headers
    //free the request object itself
}
