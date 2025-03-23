//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// trie.cpp
//
// Identification: src/primer/trie.cpp
//
// Copyright (c) 2015-2025, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

/**
 * @brief Get the value associated with the given key.
 * 1. If the key is not in the trie, return nullptr.
 * 2. If the key is in the trie but the type is mismatched, return nullptr.
 * 3. Otherwise, return the value.
 */
template <class T>
auto Trie::Get(std::string_view key) const -> const T * {
  // throw NotImplementedException("Trie::Get is not implemented.");

  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.

  auto current = root_;
  for (char c : key) {
    if (current == nullptr) {
      return nullptr;
    }
    auto it = current->children_.find(c);
    if (it == current->children_.end()) {
      return nullptr;
    }
    current = it->second;
  }

  // 循环结束后，current 可能为 nullptr（例如 key 为空时，且 root_ 为 nullptr）
  if (!current->is_value_node_) {
    return nullptr;
  }

  // 这样转换对吗? 用于实现类似 instanceof 的作用.
  const auto *ret = dynamic_cast<const TrieNodeWithValue<T> *>(current.get());
  if (!ret) {
    return nullptr;
  }
  return ret->value_.get();
}

template <class T>
auto deep_copy(std::shared_ptr<const TrieNode> node, std::string_view key, size_t pos, T value)
    -> std::shared_ptr<const TrieNode> {
  // 空字符串应该存储在 root 节点上, 将 root 节点变成 TrieNodeWithValue
  if (key.empty()) {
    if (node == nullptr) {
      // 还不能用 nullptr 来初始化 children_ (map)
      return std::make_shared<const TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
    } else {
      return std::make_shared<const TrieNodeWithValue<T>>(node->children_, std::make_shared<T>(std::move(value)));
    }
  }

  char c = key[pos];

  // leaf node 永远覆写
  if (pos == key.size()) {

    // already exist value in current node
    if (node) {
      // (beyyes) 每次参数传递都要 std::move 肯定不好...  能不能只传递 value ?
      return std::make_shared<const TrieNodeWithValue<T>>(node->children_, std::make_shared<T>(std::move(value)));
    }

    // 这样构造 TrieNodeWithValue 对吗
    return std::make_shared<const TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
  }

  // root node == nullptr
  if (node == nullptr) {
    std::map<char, std::shared_ptr<const TrieNode>> children_;
    children_.emplace(c, deep_copy(nullptr, key, pos + 1, std::move(value)));
    auto trie_ret = std::make_shared<const TrieNode>(children_);
    return trie_ret;
  }

  // char 不存在, 创建新节点
  auto it = node->children_.find(c);
  if (it == node->children_.end()) {
    std::map<char, std::shared_ptr<const TrieNode>> children_;
    children_.emplace(c, deep_copy(nullptr, key, pos + 1, std::move(value)));
    auto ret = std::make_shared<const TrieNode>(children_);
    return ret;
  }

  // c 已经存在, 这行判断可能有问题?
  // std::shared_ptr<const TrieNode> node_copy = node->Clone();
  std::map<char, std::shared_ptr<const TrieNode>> children_;
  for (const auto &pair : node->children_) {
    if (pair.first == c) {
      children_.insert({c, deep_copy(pair.second, key, pos + 1, std::move(value))});
    } else {
      children_.insert(pair);
    }
  }
  return std::make_shared<const TrieNode>(children_);
}

/**
 * @brief Put a new key-value pair into the trie. If the key already exists, overwrite the value.
 * @return the new trie.
 */
template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // throw NotImplementedException("Trie::Put is not implemented.");

  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  return Trie(deep_copy(root_, key, 0, std::move(value)));
}

/**
 * @brief Remove the key from the trie.
 * @return If the key does not exist, return the original trie. Otherwise, returns the new trie.
 */
auto Trie::Remove(std::string_view key) const -> Trie {
  throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
