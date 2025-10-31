/* SPDX-License-Identifier: AGPL-3.0-only */
#include "HTTP.h"

namespace http {

std::unordered_map<std::string, std::string> parseParameters(std::string input) {

    std::vector<std::string> parameters = split(input, '&');

    std::unordered_map<std::string, std::string> http_parameters;

    for (auto p : parameters) {

        std::vector<std::string> part = split(p, '=');

        size_t pos = p.find_first_of('=');

        if (pos == std::string::npos) {
            continue;
        }

        std::string p_name = p.substr(0, pos);
        std::string p_value = p.substr(pos + 1);

        std::string decoded_name = urlDecode(p_name);
        std::string decoded_value = urlDecode(p_value);

        http_parameters[decoded_name] = decoded_value;
    }

    return http_parameters;
}

std::string req_t::get_cookie(std::string name) const {

    std::string value = "";

    if (headers.contains("Cookie")) {

        std::string cookie_header = headers.at("Cookie");

        std::vector<std::string> cookies = split(cookie_header, ';');

        for (auto &cookie : cookies) {

            // Find first '=' in cookie
            size_t pos = cookie.find_first_of('=');

            if (pos == std::string::npos) {
                continue;
            }

            // TODO: trim?
            std::string cookie_name = trim(cookie.substr(0, pos));
            std::string cookie_value = trim(cookie.substr(pos + 1));

            if (cookie_name == name) {
                value = cookie_value;
                break;
            }
        }
    }

    std::unordered_map<std::string, std::string> cookies;

    return value;
}

bool resp_t::set_cookie(std::string name, std::string value) { cookies[name] = value; }

std::string urlEncode(const std::string &decoded) {

    char *encoded_value = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
    std::string result(encoded_value);
    curl_free(encoded_value);
    return result;
}

std::string urlDecode(const std::string &encoded) {

    int output_length;
    char *decoded_value = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &output_length);
    std::string result(decoded_value, output_length);
    curl_free(decoded_value);
    return result;
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data) {
    data->append((char *)ptr, size * nmemb);
    return size * nmemb;
}

static size_t header_callback(char *buffer, size_t size, size_t nitems, void *userdata) {

    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */

    return nitems * size;
}

resp_t get(std::string URL, std::vector<std::string> headers, std::string proxy, bool verbose) {

    // Cache for avoiding unnecesary http requests

    // TODO: What happens if the server changes the response?

    static std::unordered_map<std::string, std::string> requests_cache;

    // Check HTTP_PROXY global variable
    if (proxy == "") {
        proxy = HTTP_PROXY; // Function argument precedence over global variable
    }

    resp_t r;

    std::string response;

    if (requests_cache.count(URL) > 0) {
        response = requests_cache[URL];

    } else {

        static CURLcode res;

        static CURL *handle;

        static std::string currProxy;

        if (handle == NULL) {
            handle = curl_easy_init();

            res = curl_easy_setopt(handle, CURLOPT_USERAGENT,
                                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36");

            /* send all data to this function  */
            res = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunction);

            /* callback that receives header data */
            // curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);

            currProxy = proxy;
            curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());

            /* we tell libcurl to follow redirection */
            curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        }

        /* we pass our 'chunk' to the callback function */
        res = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

        std::string header_string;

        curl_easy_setopt(handle, CURLOPT_HEADERDATA, &header_string);

        struct curl_slist *headersList = NULL;

        for (std::string &h : headers) {
            headersList = curl_slist_append(headersList, h.c_str());
        }

        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headersList);

        // verbose = true;

        if (verbose) {
            res = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L); // Set verbose mode on
        }

        if (currProxy != proxy) {
            curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());
        }

        /* set URL to operate on */
        res = curl_easy_setopt(handle, CURLOPT_URL, (URL).c_str());

        res = curl_easy_perform(handle);

        r.body = response;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &r.code);

        // r.headers = header_string;

        struct curl_header *h;
        struct curl_header *prev = NULL;
        do {
            h = curl_easy_nextheader(handle, CURLH_HEADER, -1, prev);
            if (h) {
                r.headers.insert({std::string(h->name), std::string(h->value)});
            }
            prev = h;
        } while (h);

        // curl_easy_cleanup(handle);

        // Update cache table
        requests_cache[URL] = response;
    }

    return r;
}

bool download_to_pwd(std::string URL, std::vector<std::string> headers, std::string proxy, bool verbose) {

    bool success = false;

    resp_t r = get(URL, headers, proxy, verbose);

    if (r.code == 200) {
        success = true;
    }

    // TODO: We can get the filename from the URL or from the Content-Disposition header
    std::string filename = URL.substr(URL.find_last_of("/") + 1);

    if (!write_file(filename, r.body)) {
        std::cerr << "Error writing file: " << filename << std::endl;
        return false;
    }

    return success;
}

resp_t post(std::string URL, const char *data, uint32_t data_size, std::vector<std::string> headers, std::string proxy, bool verbose) {

    // Cache for avoiding unnecesary http requests
    static std::unordered_map<std::string, std::string> requests_cache;

    // Check HTTP_PROXY global variable
    if (proxy == "") {
        proxy = HTTP_PROXY; // Function argument precedence over global variable
    }

    resp_t r;

    std::string response;

    // TODO: Request cache for POST requests does not work properly (post parameters can change)
    // if(requests_cache.count(URL) > 0){
    if (1 == -1) {
        response = requests_cache[URL];

    } else {

        static CURLcode res;

        static CURL *handle;

        static std::string currProxy;

        if (handle == NULL) {
            handle = curl_easy_init();

            res = curl_easy_setopt(handle, CURLOPT_USERAGENT,
                                   "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36");

            /* send all data to this function  */
            res = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunction);

            currProxy = proxy;

            curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());
        }

        /* we pass our 'chunk' to the callback function */
        res = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

        std::string header_string;

        curl_easy_setopt(handle, CURLOPT_HEADERDATA, &header_string);

        struct curl_slist *headersList = NULL;

        for (std::string &h : headers) {
            headersList = curl_slist_append(headersList, h.c_str());
        }

        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headersList);

        // verbose = true;

        if (verbose) {
            res = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L); // Set verbose mode on
        }

        if (currProxy != proxy) {
            curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());
        }

        /* set URL to operate on */
        res = curl_easy_setopt(handle, CURLOPT_URL, (URL).c_str());

        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE_LARGE, data_size);

        res = curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);

        res = curl_easy_perform(handle);

        if (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
        }

        // curl_easy_cleanup(handle);

        // Update cache table
        // requests_cache[URL] = response;

        r.body = response;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &r.code);
    }

    return r;
}

static int ping(CURL *curl, const char *send_payload) {
    size_t sent;
    CURLcode result = curl_ws_send(curl, send_payload, strlen(send_payload), &sent, 0, CURLWS_PING);
    return (int)result;
}

static int recv_pong(CURL *curl, const char *exected_payload) {
    size_t rlen;
    const curl_ws_frame *meta;
    char buffer[256];
    CURLcode result = curl_ws_recv(curl, buffer, sizeof(buffer), &rlen, &meta);
    if (!result) {
        if (meta->flags & CURLWS_PONG) {
            int same = 0;
            fprintf(stderr, "ws: got PONG back\n");
            if (rlen == strlen(exected_payload)) {
                if (!memcmp(exected_payload, buffer, rlen)) {
                    fprintf(stderr, "ws: got the same payload back\n");
                    same = 1;
                }
            }
            if (!same)
                fprintf(stderr, "ws: did NOT get the same payload back\n");
        } else {
            fprintf(stderr, "recv_pong: got %u bytes rflags %x\n", (int)rlen, meta->flags);
        }
    }
    fprintf(stderr, "ws: curl_ws_recv returned %u, received %u\n", (unsigned int)result, (unsigned int)rlen);
    return (int)result;
}

static int recv_any(CURL *curl) {
    size_t rlen;
    const curl_ws_frame *meta;
    char buffer[256];
    CURLcode result = curl_ws_recv(curl, buffer, sizeof(buffer), &rlen, &meta);
    if (result)
        return result;

    return 0;
}

/* close the connection */
static void websocket_close(CURL *curl) {
    size_t sent;
    (void)curl_ws_send(curl, "", 0, &sent, 0, CURLWS_CLOSE);
}

void websocket(std::string URL, const char *data, uint32_t data_size, std::vector<std::string> headers, std::string proxy, bool verbose) {

    resp_t r;

    std::string response;

    static CURLcode res;

    static CURL *handle;

    static std::string currProxy;

    if (handle == NULL) {
        handle = curl_easy_init();

        res = curl_easy_setopt(handle, CURLOPT_USERAGENT,
                               "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/107.0.0.0 Safari/537.36");

        /* send all data to this function  */
        res = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFunction);

        currProxy = proxy;
        curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());
    }

    /* we pass our 'chunk' to the callback function */
    res = curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

    std::string header_string;

    curl_easy_setopt(handle, CURLOPT_HEADERDATA, &header_string);

    struct curl_slist *headersList = NULL;

    for (std::string &h : headers) {
        headersList = curl_slist_append(headersList, h.c_str());
    }

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headersList);

    // verbose = true;

    if (verbose) {
        res = curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L); // Set verbose mode on
    }

    if (currProxy != proxy) {
        curl_easy_setopt(handle, CURLOPT_PROXY, currProxy.c_str());
    }

    /* set URL to operate on */
    res = curl_easy_setopt(handle, CURLOPT_URL, (URL).c_str());

    /* websocket style */
    res = curl_easy_setopt(handle, CURLOPT_CONNECT_ONLY, 2L);

    // curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE_LARGE, data_size);

    // res = curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data);

    res = curl_easy_perform(handle);

    if (res != CURLE_OK) {
        std::cout << "Error in http::websocket" << std::endl;
        exit(1);

    } else {

        int i = 0;
        do {
            recv_any(handle);
            if (ping(handle, "foobar"))
                return;
            if (recv_pong(handle, "foobar")) {
                return;
            }
            sleep(2);
        } while (i++ < 10);

        websocket_close(handle);
    }

    long response_code;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);

    // curl_easy_cleanup(handle);

    // Update cache table
    // requests_cache[URL] = response;

    r.body = response;
    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &r.code);

    // return r;
}

}; // namespace http