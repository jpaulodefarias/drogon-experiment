#pragma once

#include "domain/UserRepository.hpp"

#include <drogon/orm/DbClient.h>

namespace example {
class PostgresUserRepository final : public UserRepository {
  public:
    explicit PostgresUserRepository(drogon::orm::DbClientPtr client);
    void save(User user, OnSaved onSaved, OnError onError) override;
    void findById(std::string id, OnFound onFound, OnError onError) override;
    void list(OnListed onListed, OnError onError) override;
    void deleteById(std::string id, OnDeleted onDeleted, OnError onError) override;
    void deleteAll(OnDeletedAll onDeleted, OnError onError) override;

  private:
    drogon::orm::DbClientPtr client_;
};
} // namespace example
