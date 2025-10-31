/* SPDX-License-Identifier: AGPL-3.0-only */
#include "llm.h"
#include "openai.h"
#include <stdexcept>

using namespace llm;

Client::Client(const std::string &model, const std::string &api_key) : model_(model), api_key_(api_key) {
    if (model.starts_with("gpt")) { // starts_with "gpt"
        impl_ = openai::make_backend(model_, api_key_);
    } else if (model.starts_with("gemini")) {
        // dropped in v0.1
    } else if (model.starts_with("claude")) {
        // dropped in v0.1
    }

    if (!impl_) {
        throw std::runtime_error("Unsupported model: " + model_);
    }
}

Client::~Client() { delete impl_; }

std::string Client::send(const std::string &content) {

    // The function create a JSON body with the model, instructions and prompt

    bool verbose = false;

    std::vector<std::string> headers = {"Content-Type: application/json", std::string("Authorization: Bearer ") + api_key_};

    JSON body = impl_->createRequest(content);

    http::resp_t response = http::post(impl_->url, body.dump().c_str(), body.dump().size(), headers, "", verbose);

    if (response.code != 200) {
        std::cerr << "Error: " << response.code << "\n";
        std::cerr << "Response body: " << response.body << "\n";
        return "";
    }

    std::string out;

    try {

        JSON j = JSON::parse(response.body);

        out = impl_->parseResponse(j);

    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON response: " << e.what() << "\n";
        return "";
    }

    return out;
}