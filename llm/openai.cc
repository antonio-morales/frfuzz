/* SPDX-License-Identifier: AGPL-3.0-only */
#include "openai.h"
// #include "network/HTTP.h"  // when you wire real HTTP
// #include "json.h"          // reuse your JSON wrapper
#include <utility>

using namespace llm::openai;

OpenAI::OpenAI(std::string model, std::string api_key) : model_(std::move(model)), api_key_(std::move(api_key)) {

    if (model_ != "gpt-5-nano" && model_ != "gpt-5-mini" && model_ != "gpt-5") {
        throw std::runtime_error("Unsupported OpenAI model: " + model_);
    }

    this->url = OPENAI_API_URL;
}

JSON OpenAI::createRequest(const std::string &content) {

    // reasoning effort : minimal, low, medium, and high

    JSON body = {{"model", model_},
                 {"instructions", instructions_},
                 {"input", JSON::array({JSON{{"role", "user"}, {"content", content}}})},
                 // Temperature is not supported in gpt-5
                 //{"temperature", 0.3},
                 {"max_output_tokens", 5000},
                 {"reasoning", {{"effort", effort_}}}};

    return body;
}

std::string OpenAI::parseResponse(const JSON &j) {

    // We need to check for incomplete_details ("incomplete_details": {"reason": "max_output_tokens"},)
    if (j.contains("incomplete_details")) {

        if (j["incomplete_details"].contains("reason")) {
            std::string reason = j["incomplete_details"]["reason"];
            std::cerr << "Warning: Response is incomplete: " << reason << "\n";
            return "";
        }
    }

    std::string out;
    if (j.contains("output") && j["output"].is_array()) {
        for (const auto &item : j["output"]) {
            if (item.value("type", "") == "message" && item.contains("content")) {
                for (const auto &c : item["content"]) {
                    if (c.value("type", "") == "output_text") {
                        out += c.value("text", "");
                    }
                }
            }
        }
    }

    return out;
}

llm::IBackend *llm::openai::make_backend(const std::string &model, const std::string &api_key) { return new OpenAI(model, api_key); }