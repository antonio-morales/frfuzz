/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once

#include <string>

#include <iostream>

#include <cstring>

#include <unordered_map>
#include <vector>

#include <unistd.h>

// include libcurl
#include <curl/curl.h>

#include "global.h"

#include "utils/utils.h"

namespace http {

enum class METHOD { GET, POST, PUT, HEAD };

enum class VERSION { HTTP_0_9, HTTP_1_0, HTTP_1_1, HTTP_2, HTTP_3 };

const std::unordered_map<int, std::string> STATUS_CODES = {

    {100, "Continue"},

    {200, "OK"},

    {201, "Created"},

    {202, "Accepted"},

    {204, "No Content"},

    {206, "Partial Content"},

    {301, "Moved Permanently"},

    {302, "Found"},

    {304, "Not Modified"},

    {400, "Bad Request"},

    {401, "Unauthorized"},

    {403, "Forbidden"},

    {404, "Not Found"},

    {405, "Method Not Allowed"},

    {500, "Internal Server Error"},

    {501, "Not Implemented"},

    {505, "HTTP Version Not Supported"}};

struct req_t {

    // Request
    METHOD method;

    std::string original_uri; // URI as received in the request
    std::string uri;          // URI after normalization
    std::vector<std::string> v_path;

    std::string query;
    std::unordered_map<std::string, std::string> query_parameters;

    VERSION version;

    // Headers
    std::unordered_map<std::string, std::string> headers;

    // Body
    std::string body;
    std::unordered_map<std::string, std::string> body_parameters; // application/x-www-form-urlencoded
    std::vector<std::string> body_parts;                          // multipart/form-data

    std::string message; // HTTP Message = Request + Headers + Body

    // Reference to an item from headers
    std::string get_cookie(std::string name) const;

    // std::unordered_map<std::string, std::string> headers;
};

struct resp_t {

    std::string version = "HTTP/1.1";
    uint32_t code;
    std::unordered_map<std::string, std::string> headers;
    std::string body; // body

    std::unordered_map<std::string, std::string> cookies; // Multiple set-cookies allowed
    bool set_cookie(std::string name, std::string value);
};

std::string urlEncode(const std::string &decoded);

std::string urlDecode(const std::string &encoded);

std::unordered_map<std::string, std::string> parseParameters(std::string input);

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string *data);

resp_t get(std::string URL, std::vector<std::string> headers = {}, std::string proxy = "", bool verbose = false);

bool download_to_pwd(std::string URL, std::vector<std::string> headers = {}, std::string proxy = "", bool verbose = false);

resp_t post(std::string URL, const char *data, uint32_t data_size, std::vector<std::string> headers = {}, std::string proxy = "",
            bool verbose = false);

void websocket(std::string URL, const char *data, uint32_t data_size, std::vector<std::string> headers, std::string proxy, bool verbose);

}; // namespace http
