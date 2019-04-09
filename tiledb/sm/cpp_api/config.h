/**
 * @file   tiledb_cpp_api_config.h
 *
 * @author Ravi Gaddipati
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2018 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file declares the C++ API for the TileDB Config object.
 */

#ifndef TILEDB_CPP_API_CONFIG_H
#define TILEDB_CPP_API_CONFIG_H

#include "tiledb.h"
#include "utils.h"

#include <memory>
#include <string>

namespace tiledb {

class Config;  // Forward decl for impl classes

namespace impl {

class ConfigIter : public std::iterator<
                       std::forward_iterator_tag,
                       const std::pair<std::string, std::string>> {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Iterate over a config for params matching a given prefix. **/
  ConfigIter(const Config& config, std::string prefix, bool done = false)
      : prefix_(std::move(prefix))
      , done_(done) {
    init(config);
  }

  ConfigIter(const ConfigIter&) = default;
  ConfigIter(ConfigIter&&) = default;
  ConfigIter& operator=(const ConfigIter&) = default;
  ConfigIter& operator=(ConfigIter&&) = default;

  bool operator==(const ConfigIter& o) const {
    return done_ == o.done_;
  }

  bool operator!=(const ConfigIter& o) const {
    return done_ != o.done_;
  }

  const std::pair<std::string, std::string>& operator*() const {
    return here_;
  }

  const std::pair<std::string, std::string>* operator->() const {
    return &here_;
  }

  ConfigIter& operator++();

  /* ********************************* */
  /*          STATIC FUNCTIONS         */
  /* ********************************* */

  /** Wrapper function for freeing a config iter C object. */
  static void free(tiledb_config_iter_t* config_iter) {
    tiledb_config_iter_free(&config_iter);
  }

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** Prefix of parameters to match. **/
  std::string prefix_;

  /** Pointer to iter object. **/
  std::shared_ptr<tiledb_config_iter_t> iter_;

  /** Current object. **/
  std::pair<std::string, std::string> here_;

  /** If iter is done. **/
  bool done_;

  /* ********************************* */
  /*          PRIVATE METHODS          */
  /* ********************************* */

  /** Init the iterator object **/
  void init(const Config& config);
};

/** Proxy to set params via operator `[]`. */
struct ConfigProxy {
  ConfigProxy(Config& conf, std::string param)
      : conf(conf)
      , param(std::move(param)) {
  }

  template <typename T>
  ConfigProxy& operator=(const T& val);

  ConfigProxy& operator=(const char* val);

  ConfigProxy& operator=(const std::string& val);

  ConfigProxy operator[](const std::string& append);

  operator std::string();

  Config& conf;
  const std::string param;
};

}  // namespace impl

/**
 * Carries configuration parameters for a context.
 *
 * **Example:**
 *
 * @code{.cpp}
 * Config conf;
 * conf["vfs.s3.region"] = "us-east-1a";
 * conf["vfs.s3.use_virtual_addressing"] = "true";
 * Context ctx(conf);
 * // array/kv operations with ctx
 * @endcode
 * */
class Config {
 public:
  using iterator = impl::ConfigIter;
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  Config() {
    create_config();
  }

  /**
   * Constructor that takes as input a filename (URI) that stores the config
   * parameters. The file must have the following (text) format:
   *
   * `{parameter} {value}`
   *
   * Anything following a `#` character is considered a comment and, thus, is
   * ignored.
   *
   * See `Config::set` for the various TileDB config parameters and allowed
   * values.
   *
   * @param filename The name of the file where the parameters will be read
   *     from.
   */
  explicit Config(const std::string& filename) {
    create_config();
    tiledb_error_t* err;
    tiledb_config_load_from_file(config_.get(), filename.c_str(), &err);
    impl::check_config_error(err);
  }

  /** Constructor from a C config object. */
  explicit Config(tiledb_config_t** config) {
    if (config) {
      config_ = std::shared_ptr<tiledb_config_t>(*config, Config::free);
      *config = nullptr;
    }
  }

  /* ********************************* */
  /*                API                */
  /* ********************************* */

  /** Saves the config parameters to a (local) text file. */
  void save_to_file(const std::string filename) {
    tiledb_error_t* err;
    tiledb_config_save_to_file(config_.get(), filename.c_str(), &err);
    impl::check_config_error(err);
  }

  /** Returns the pointer to the TileDB C config object. */
  std::shared_ptr<tiledb_config_t> ptr() const {
    return config_;
  }

  /** Auxiliary operator for getting the underlying C TileDB object. */
  operator tiledb_config_t*() const {
    return config_.get();
  }

  /** Sets a config parameter.
   *
   * \copydetails tiledb_config_set()
   *
   */
  Config& set(const std::string& param, const std::string& value) {
    tiledb_error_t* err;
    tiledb_config_set(config_.get(), param.c_str(), value.c_str(), &err);
    impl::check_config_error(err);
    return *this;
  }

  /**
   * Get a parameter from the configuration by key.
   * @param param Name of configuration parameter
   * @return Value of configuration parameter
   * @throws TileDBError if the parameter does not exist
   */
  std::string get(const std::string& param) const {
    const char* val;
    tiledb_error_t* err;
    tiledb_config_get(config_.get(), param.c_str(), &val, &err);
    impl::check_config_error(err);

    if (val == nullptr)
      throw TileDBError("Config Error: Invalid parameter '" + param + "'");

    return val;
  }

  /**
   * Operator that enables setting parameters with `[]`.
   *
   * **Example:**
   *
   * @code{.cpp}
   * Config conf;
   * conf["vfs.s3.region"] = "us-east-1a";
   * conf["vfs.s3.use_virtual_addressing"] = "true";
   * Context ctx(conf);
   * @endcode
   *
   * @param param Name of parameter to set
   * @return "Proxy" object supporting assignment.
   */
  impl::ConfigProxy operator[](const std::string& param);

  /**
   * Resets a config parameter to its default value.
   *
   * @param param Name of parameter
   * @return Reference to this Config instance
   */
  Config& unset(const std::string& param) {
    tiledb_error_t* err;
    tiledb_config_unset(config_.get(), param.c_str(), &err);
    impl::check_config_error(err);

    return *this;
  }

  /**
   * Iterate over params starting with a prefix.
   *
   * **Example:**
   * @code{.cpp}
   * tiledb::Config config;
   * for (auto it = config.begin("vfs"), ite = config.end(); it != ite; ++it) {
   *   std::string name = it->first, value = it->second;
   * }
   * @endcode
   *
   * @param prefix Prefix to iterate over
   * @return iterator
   */
  iterator begin(const std::string& prefix) {
    return iterator{*this, prefix, false};
  }

  /**
   * Iterate over all params.
   *
   * **Example:**
   * @code{.cpp}
   * tiledb::Config config;
   * for (auto it = config.begin(), ite = config.end(); it != ite; ++it) {
   *   std::string name = it->first, value = it->second;
   * }
   * @endcode
   *
   * @return iterator
   */
  iterator begin() {
    return iterator{*this, "", false};
  }

  /** End iterator. **/
  iterator end() {
    return iterator{*this, "", true};
  }

  /* ********************************* */
  /*          STATIC FUNCTIONS         */
  /* ********************************* */

  /** Wrapper function for freeing a config C object. */
  static void free(tiledb_config_t* config) {
    tiledb_config_free(&config);
  }

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** The TileDB C config object. */
  std::shared_ptr<tiledb_config_t> config_;

  /* ********************************* */
  /*          PRIVATE METHODS          */
  /* ********************************* */

  /** Creates the TileDB C config object. */
  void create_config() {
    tiledb_config_t* config;
    tiledb_error_t* err;
    tiledb_config_alloc(&config, &err);
    impl::check_config_error(err);

    config_ = std::shared_ptr<tiledb_config_t>(config, Config::free);
  }
};

/* ********************************* */
/*            DEFINITIONS            */
/* ********************************* */

inline impl::ConfigProxy Config::operator[](const std::string& param) {
  return {*this, param};
}

namespace impl {

template <typename T>
inline ConfigProxy& impl::ConfigProxy::operator=(const T& val) {
  conf.set(param, std::to_string(val));
  return *this;
}

inline ConfigProxy& impl::ConfigProxy::operator=(const char* val) {
  conf.set(param, std::string(val));
  return *this;
}

inline ConfigProxy& impl::ConfigProxy::operator=(const std::string& val) {
  conf.set(param, val);
  return *this;
}

inline ConfigProxy impl::ConfigProxy::operator[](const std::string& append) {
  return {conf, param + append};
}

inline ConfigProxy::operator std::string() {
  return conf.get(param);
}

inline void ConfigIter::init(const Config& config) {
  tiledb_config_iter_t* iter;
  tiledb_error_t* err;
  const char* p = prefix_.size() ? prefix_.c_str() : nullptr;
  tiledb_config_iter_alloc(config.ptr().get(), p, &iter, &err);
  check_config_error(err);

  iter_ = std::shared_ptr<tiledb_config_iter_t>(iter, ConfigIter::free);

  // Get first param-value pair
  int done;
  tiledb_config_iter_done(iter_.get(), &done, &err);
  check_config_error(err);
  if (done == 1) {
    done_ = true;
  } else {
    const char *param, *value;
    tiledb_config_iter_here(iter_.get(), &param, &value, &err);
    check_config_error(err);
    here_ = std::pair<std::string, std::string>(param, value);
  }
}

inline ConfigIter& ConfigIter::operator++() {
  if (done_)
    return *this;
  int done;
  tiledb_error_t* err;

  tiledb_config_iter_next(iter_.get(), &err);
  check_config_error(err);

  tiledb_config_iter_done(iter_.get(), &done, &err);
  check_config_error(err);
  if (done == 1) {
    done_ = true;
    return *this;
  }

  const char *param, *value;
  tiledb_config_iter_here(iter_.get(), &param, &value, &err);
  check_config_error(err);
  here_ = std::pair<std::string, std::string>(param, value);
  return *this;
}

}  // namespace impl

}  // namespace tiledb

#endif  // TILEDB_CPP_API_CONFIG_H
