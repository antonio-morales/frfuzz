/* SPDX-License-Identifier: AGPL-3.0-only */
#pragma once
#include <memory>
#include <string>

#include "json.h"
#include "network/HTTP.h"

namespace llm {

class IBackend {
  public:
    virtual ~IBackend() = default;
    virtual void setInstructions(const std::string &instr) = 0;
    virtual void setEffort(const std::string &effort) = 0;
    virtual JSON createRequest(const std::string &prompt) = 0;
    virtual std::string parseResponse(const JSON &j) = 0;
    std::string url;
};

class Client {
  public:
    Client(const std::string &model, const std::string &api_key);
    ~Client(); // defined in .cc
    Client(Client &&) noexcept = default;
    Client &operator=(Client &&) noexcept = default;

    // Non-copyable to avoid duplicating backend state (make copyable later if needed)
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    void setInstructions(const std::string &instr) {
        instructions_ = instr;
        if (impl_)
            impl_->setInstructions(instructions_);
    }

    void setEffort(const std::string &effort) {
        if (impl_)
            impl_->setEffort(effort);
    }

    std::string send(const std::string &prompt);

    const std::string &model() const { return model_; }

  private:
    IBackend *impl_ = nullptr;
    std::string model_;
    std::string api_key_;

    std::string instructions_ = "";
};

} // namespace llm
