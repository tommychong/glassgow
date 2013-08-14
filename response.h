typedef struct ggHttpResponse {
    unsigned int status;
    char *body;
    gint body_len;
    //headers;
    //http_version version;
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

    response->body = 0;
    response->body_len = -1; //Needed for binary data like pictures

    return response;
}

void gg_http_response_free(ggHttpResponse *response) {
    //Free the headers; hrm, what to do about dynamically allocated values/keys?
    g_hash_table_destroy(response->headers);

    /* TODO: is it best to set the response body directly, or use a write(...) api so clearing the buffer is more consistent
     * Hmm... Probably better to use a write api because we will want to do chunked responses sometime..
    if(response->body) {
        //free(response->body);
    }
    */

    free(response);
}
