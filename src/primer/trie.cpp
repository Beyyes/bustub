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

  if (root_ == nullptr) {
    return nullptr;
  }

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

  // 循环结束后, current 可能为 nullptr（例如 key 为空时, 且 root_ 为 nullptr）
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
auto DeepCopy(const std::shared_ptr<const TrieNode> &node, std::string_view key, const size_t pos, T value)
    -> std::shared_ptr<const TrieNode> {
  if (pos == key.size()) {
    // (beyyes) 每次都要 std::move, 因为 T value 可能是 unique_ptr
    auto value_ptr = std::make_shared<T>(std::move(value));
    if (node != nullptr) {
      // already exist value in current node
      return std::make_shared<const TrieNodeWithValue<T>>(node->children_, value_ptr);
    }
    return std::make_shared<const TrieNodeWithValue<T>>(value_ptr);
  }

  const char c = key[pos];

  // root node is nullptr, or create non-exist key
  if (node == nullptr) {
    std::map<char, std::shared_ptr<const TrieNode>> children;
    children.emplace(c, DeepCopy(nullptr, key, pos + 1, std::move(value)));
    auto trie_ret = std::make_shared<const TrieNode>(children);
    return trie_ret;
  }

  // Clone 体现多态, 无需判断 TrieNode 还是 TrieNodeWithValue
  // COW 实现, 总需要 Clone, 是否会有并发问题?
  std::unique_ptr<TrieNode> node_copy = node->Clone();

  if (const auto it = node->children_.find(c); it == node->children_.end()) {
    // key 不存在, 创建新节点
    node_copy->children_.emplace(c, DeepCopy(nullptr, key, pos + 1, std::move(value)));
  } else {
    // key 存在, map覆盖值得用 operator[]
    node_copy->children_[c] = DeepCopy(node->children_.find(c)->second, key, pos + 1, std::move(value));
  }

  std::shared_ptr<const TrieNode> ret = std::move(node_copy);
  return ret;
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

  // 空字符串应该存储在 root 节点上, 将 root 节点变成 TrieNodeWithValue
  if (key.empty()) {
    if (root_ == nullptr) {
      return Trie(std::make_shared<const TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value))));
    }
    return Trie(std::make_shared<const TrieNodeWithValue<T>>(root_->children_, std::make_shared<T>(std::move(value))));
  }

  return Trie(DeepCopy(root_, key, 0, std::move(value)));
}

template <class T>
static auto CloneAndPut(const std::shared_ptr<const TrieNode> &node, std::string_view key, const size_t index,
                        T &&value) -> std::shared_ptr<const TrieNode> {
  if (index == key.size()) {
    // Create a new value node, inheriting children from the existing node if present.
    auto value_ptr = std::make_shared<T>(std::forward<T>(value));
    if (node != nullptr) {
      return std::make_shared<TrieNodeWithValue<T>>(node->children_, value_ptr);
    }
    return std::make_shared<TrieNodeWithValue<T>>(value_ptr);
  }

  const char c = key[index];
  std::shared_ptr<const TrieNode> child_node;

  if (node != nullptr) {
    if (const auto it = node->children_.find(c); it != node->children_.end()) {
      child_node = it->second;
    }
  }

  // Recursively process the child node.
  auto new_child = CloneAndPut<T>(child_node, key, index + 1, std::forward<T>(value));

  // Clone the current node or create a new one if it doesn't exist.
  std::unique_ptr<TrieNode> new_node;
  if (node != nullptr) {
    new_node = node->Clone();
  } else {
    new_node = std::make_unique<TrieNode>();
  }

  // Update the child pointer for the current character.
  new_node->children_[c] = new_child;

  // release 改成 move? release 和 move 的安全高效区别?
  return std::shared_ptr<const TrieNode>(new_node.release());
}

// template <class T>
// auto Trie::Put(std::string_view key, T value) const -> Trie {
//   if (key.empty()) {
//     // Handle empty key case, replace the root with a new value node.
//     std::shared_ptr<const TrieNode> new_root;
//     if (root_ != nullptr) {
//       new_root = std::make_shared<TrieNodeWithValue<T>>(root_->children_, std::make_shared<T>(std::move(value)));
//     } else {
//       new_root = std::make_shared<TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
//     }
//     return Trie(new_root);
//   }
//
//   // Recursively process each character to build the new trie.
//   auto new_root = CloneAndPut<T>(root_, key, 0, std::move(value));
//   return Trie(new_root);
// }

auto RemoveCopy(const std::shared_ptr<const TrieNode> &node, std::string_view key, const size_t pos)
    -> std::shared_ptr<const TrieNode> {
  const char c = key[pos];

  if (pos == key.size()) {
    // is value node: has children, return TrieNode with children; no children, return TrieNode with empty children?
    // not value node, return null
    if (node->is_value_node_) {
      // is last node, return with no children
      if (node->children_.empty()) {
        return std::make_shared<const TrieNode>();
      }
      // is not last node, change TrieNodeWithValue to TrieNode
      return std::make_shared<const TrieNode>(node->children_);
    }
    return nullptr;
  }

  const auto it = node->children_.find(c);
  if (it == node->children_.end()) {
    return nullptr;
  }

  std::shared_ptr<const TrieNode> child_node = RemoveCopy(it->second, key, pos + 1);
  if (child_node == nullptr) {
    return nullptr;
  }

  std::unique_ptr<TrieNode> new_node = node->Clone();

  // only when child_node is non value node can erase
  if (child_node->children_.empty() && !child_node->is_value_node_) {
    new_node->children_.erase(c);
    return std::move(new_node);
  }

  new_node->children_[c] = std::move(child_node);
  return std::move(new_node);
}

/**
 * @brief Remove the key from the trie.
 * @return If the key does not exist, return the original trie. Otherwise, returns the new trie.
 */
auto Trie::Remove(std::string_view key) const -> Trie {
  // throw NotImplementedException("Trie::Remove is not implemented.");

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value anymore,
  // you should convert it to `TrieNode`. If a node doesn't have children anymore, you should remove it.

  if (key.empty()) {
    if (root_ == nullptr) {
      return *this;
    }
    return Trie(std::make_shared<const TrieNode>(root_->children_));
  }

  if (root_ == nullptr) {
    return *this;
  }

  if (const std::shared_ptr<const TrieNode> child = RemoveCopy(root_, key, 0); child != nullptr) {
    if (child->children_.empty()) {
      return Trie(nullptr);
    }
    return Trie(child);
  }
  return *this;

  // 这样转换对吗? 用于实现类似 instanceof 的作用.
  // const auto *ret = dynamic_cast<const TrieNodeWithValue*>(current.get());
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
