// Copyright (c) 2016 Kasper Kronborg Isager and Radosław Niemczyk.
#include "table.hpp"

namespace lsh {
  /**
   * Construct a new classic lookup table.
   *
   * @param config The configuration parameters for the lookup table.
   */
  table::table(const classic& c) {
    unsigned int d = c.dimensions;
    unsigned int s = c.samples;
    unsigned int p = c.partitions;

    this->dimensions_ = d;
    this->masks_.reserve(p);
    this->partitions_.reserve(p);

    for (unsigned int i = 0; i < p; i++) {
      std::random_device random;
      std::mt19937 generator(random());
      std::uniform_int_distribution<> indices(0, d - 1);

      std::vector<bool> c(d);

      for (unsigned int i = 0; i < s; i++) {
        c[indices(generator)] = 1;
      }

      this->masks_.push_back(vector(c));
      this->partitions_.push_back(partition());
    }
  }

  /**
   * Construct a new covering lookup table.
   *
   * @param config The configuration parameters for the lookup table.
   */
  table::table(const covering& c) {
    unsigned int d = c.dimensions;
    unsigned int r = c.radius;
    unsigned int x = r + 1;
    unsigned int n = 1 << x;

    this->dimensions_ = d;
    this->masks_.reserve(n - 1);
    this->partitions_.reserve(n - 1);

    std::vector<vector> m;

    for (unsigned int i = 0; i < d; i++) {
      m.push_back(vector::random(x));
    }

    for (unsigned int i = 1; i < n; i++) {
      std::vector<bool> v(x);
      std::vector<bool> c(d);

      for (unsigned int j = 0; j < x; j++) {
        v[j] = (i >> (x - j - 1)) & 1;
      }

      for (unsigned int j = 0; j < d; j++) {
        c[j] = (m[j] * v) % 2;
      }

      this->masks_.push_back(vector(c));
      this->partitions_.push_back(partition());
    }
  }

  /**
   * Construct a brute-force lookup table.
   *
   * @param config The configuration parameters for the lookup table.
   */
  table::table(const brute& c) {
    unsigned int d = c.dimensions;

    this->dimensions_ = d;
    this->masks_.push_back(vector(std::vector<bool>(d)));
    this->partitions_.push_back(partition());
  }

  /**
   * Get the number of vectors in this lookup table.
   *
   * @return The number of vectors in this lookup table.
   */
  unsigned int table::size() const {
    return this->vectors_.size();
  }

  /**
   * Insert a vector into this lookup table.
   *
   * @param vector The vector to insert into this lookup table.
   */
  void table::insert(const vector& v) {
    if (this->dimensions_ != v.size()) {
      throw std::invalid_argument("Invalid vector size");
    }

    unsigned int n = this->partitions_.size();
    unsigned int u = this->next_id_++;

    this->vectors_.insert({u, v});

    for (unsigned int i = 0; i < n; i++) {
      vector k = this->masks_[i] & v;
      bucket& b = this->partitions_[i][k.hash()];

      b.push_back(u);
    }
  }

  /**
   * Erase a vector from this lookup table.
   *
   * @param vector The vector to erase from this lookup table.
   */
  void table::erase(const vector& v) {
    if (this->dimensions_ != v.size()) {
      throw std::invalid_argument("Invalid vector size");
    }

    unsigned int n = this->partitions_.size();
    unsigned int u = 0;

    for (const auto& it: this->vectors_) {
      if (it.second == v) {
        u = it.first;
        break;
      }
    }

    this->vectors_.erase(u);

    for (unsigned int i = 0; i < n; i++) {
      partition& p = this->partitions_[i];

      for (auto& it: p) {
        bucket& b = it.second;

        for (auto j = b.begin(); j != b.end(); j++) {
          if (*j == u) {
            b.erase(j);
            break;
          }
        }
      }
    }
  }

  /**
   * Query this lookup table for the nearest neighbour of a query vector.
   *
   * @param vector The query vector to look up the nearest neighbour of.
   * @return The nearest neighbouring vector if found, otherwise a vector of size 0.
   */
  vector table::query(const vector& v) const {
    if (this->dimensions_ != v.size()) {
      throw std::invalid_argument("Invalid vector size");
    }

    unsigned int n = this->partitions_.size();

    // Keep track of the best candidate we've encountered.
    const vector* best_c = nullptr;

    // Keep track of the distance to the best candidate.
    unsigned int best_d = UINT_MAX;

    for (unsigned int i = 0; i < n; i++) {
      vector k = this->masks_[i] & v;
      const partition& p = this->partitions_[i];

      auto it = p.find(k.hash());

      if (it == p.end()) {
        continue;
      }

      const bucket& b = p.at(k.hash());

      for (unsigned int u: b) {
        const vector& c = this->vectors_.at(u);

        unsigned int d = vector::distance(v, c);

        if (d < best_d) {
          best_c = &c;
          best_d = d;
        }
      }
    }

    return best_c ? *best_c : vector({});
  }

  /**
   * Compute a number of statistics for this lookup table.
   *
   * @return The statistics computed for this lookup table.
   */
  table::statistics table::stats() const {
    unsigned int bs = 0;
    unsigned int vs = 0;
    unsigned int n = this->partitions_.size();

    for (unsigned int i = 0; i < n; i++) {
      const partition& p = this->partitions_[i];

      bs += p.size();

      for (const auto& it: p) {
        const bucket& b = it.second;

        vs += b.size();
      }
    }

    return {
      .partitions = (unsigned int) this->partitions_.size(),
      .buckets = bs,
      .vectors = vs
    };
  }
}
