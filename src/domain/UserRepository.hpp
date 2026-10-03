#pragma once

#include "domain/User.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace example {
enum class RepositoryError { Conflict, Unavailable };

class UserRepository {
  public:
    using OnSaved = std::function<void(User)>;
    using OnFound = std::function<void(std::optional<User>)>;
    using OnListed = std::function<void(std::vector<User>)>;
    using OnDeleted = std::function<void(bool)>;
    using OnDeletedAll = std::function<void()>;
    using OnError = std::function<void(RepositoryError)>;

    virtual ~UserRepository() = default;
    virtual void save(User user, OnSaved onSaved, OnError onError) = 0;
    virtual void findById(std::string id, OnFound onFound, OnError onError) = 0;
    virtual void list(OnListed onListed, OnError onError) = 0;
    virtual void deleteById(std::string id, OnDeleted onDeleted, OnError onError) = 0;
    virtual void deleteAll(OnDeletedAll onDeleted, OnError onError) = 0;
};
} // namespace example
