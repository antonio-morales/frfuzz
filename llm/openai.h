/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once
#include "llm.h"
#include <memory>
#include <string>

namespace llm::openai {

inline constexpr const char *OPENAI_API_URL = "https://api.openai.com/v1/responses";

IBackend *make_backend(const std::string &model, const std::string &api_key);

// Concrete backend
class OpenAI : public IBackend {

  public:
    OpenAI(std::string model, std::string api_key);
    void setInstructions(const std::string &instr) override { instructions_ = instr; }
    void setEffort(const std::string &effort) override { effort_ = effort; }
    JSON createRequest(const std::string &content) override;
    std::string parseResponse(const JSON &j) override;

  private:
    std::string model_;
    std::string api_key_;
    std::string instructions_;
    std::string effort_ = "low"; // minimal, low, medium, and high (default is low)
};

} // namespace llm::openai