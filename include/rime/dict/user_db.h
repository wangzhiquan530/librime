//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2013-04-16 GONG Chen <chen.sst@gmail.com>
//
#ifndef RIME_USER_DB_H_
#define RIME_USER_DB_H_

#include <stdint.h>
#include <string>
#include <rime/component.h>
#include <rime/dict/db.h>
#include <rime/dict/db_utils.h>

namespace rime {

using TickCount = uint64_t;

/// Properties of a user db entry value.
struct UserDbValue {
  int commits = 0;
  double dee = 0.0;
  TickCount tick = 0;

  UserDbValue() = default;
  UserDbValue(const std::string& value);

  std::string Pack() const;
  bool Unpack(const std::string& value);
};

/**
 * A placeholder class for user db.
 *
 * Note: do not directly use this class to instantiate a user db.
 * Instead, use the rime::UserDbWrapper<T> template, which creates
 * wrapper classes for underlying implementations of rime::Db.
 */
class UserDb {
 public:
  /// Abstract class for a user db component.
  class Component : public Db::Component {
   public:
    virtual std::string extension() const = 0;
    virtual std::string snapshot_extension() const = 0;
  };

  /// Requires a registered component for a user db class.
  static Component* Require(const std::string& name) {
    return static_cast<Component*>(Db::Require(name));
  }

  UserDb() = delete;
};

/// A helper class to provide extra functionalities related to user db.
class UserDbHelper {
 public:
  UserDbHelper(Db* db) : db_(db) {
  }
  UserDbHelper(const unique_ptr<Db>& db) : db_(db.get()) {
  }
  UserDbHelper(const shared_ptr<Db>& db) : db_(db.get()) {
  }

  bool UpdateUserInfo();
  static bool IsUniformFormat(const std::string& name);
  bool UniformBackup(const std::string& snapshot_file);
  bool UniformRestore(const std::string& snapshot_file);

  bool IsUserDb();
  std::string GetDbName();
  std::string GetUserId();
  std::string GetRimeVersion();

 protected:
  Db* db_;
};

/// A template to define a user db class based on an implementation of rime::Db.
template <class BaseDb>
class UserDbWrapper : public BaseDb {
 public:
  UserDbWrapper(const std::string& db_name);

  virtual bool CreateMetadata() {
    return BaseDb::CreateMetadata() &&
        UserDbHelper(this).UpdateUserInfo();
  }
  virtual bool Backup(const std::string& snapshot_file) {
    return UserDbHelper::IsUniformFormat(snapshot_file) ?
        UserDbHelper(this).UniformBackup(snapshot_file) :
        BaseDb::Backup(snapshot_file);
  }
  virtual bool Restore(const std::string& snapshot_file) {
    return UserDbHelper::IsUniformFormat(snapshot_file) ?
        UserDbHelper(this).UniformRestore(snapshot_file) :
        BaseDb::Restore(snapshot_file);
  }
};

/// Provides information of the db file format by its base class.
template <class BaseDb>
struct UserDbFormat {
  static const std::string extension;
  static const std::string snapshot_extension;
};

/// Implements a component that serves as a factory for a user db class.
template <class BaseDb>
class UserDbComponent : public UserDb::Component {
 public:
  virtual Db* Create(const std::string& name) {
    return new UserDbWrapper<BaseDb>(name + extension());
  }

  virtual std::string extension() const {
    return UserDbFormat<BaseDb>::extension;
  }
  virtual std::string snapshot_extension() const {
    return UserDbFormat<BaseDb>::snapshot_extension;
  }
};

class UserDbMerger : public Sink {
 public:
  explicit UserDbMerger(Db* db);
  virtual ~UserDbMerger();

  virtual bool MetaPut(const std::string& key, const std::string& value);
  virtual bool Put(const std::string& key, const std::string& value);

  void CloseMerge();

 protected:
  Db* db_;
  TickCount our_tick_;
  TickCount their_tick_;
  TickCount max_tick_;
  int merged_entries_;
};

class UserDbImporter : public Sink {
 public:
  explicit UserDbImporter(Db* db);

  virtual bool MetaPut(const std::string& key, const std::string& value);
  virtual bool Put(const std::string& key, const std::string& value);

 protected:
  Db* db_;
};

}  // namespace rime

#endif  // RIME_USER_DB_H_
