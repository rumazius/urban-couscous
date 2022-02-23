#pragma once

#include <cstdio>
#include <initializer_list>
#include <utility>

template<typename ValueType>
class Set {
private:
    struct RBTree {
        struct Node {
            Node* left;
            Node* right;
            Node* parent;
            ValueType key;
            bool red;

            Node() : left(nullptr), right(nullptr), parent(nullptr), key(0), red(false) {
            }
            explicit Node(ValueType key) : left(nullptr), right(nullptr), parent(nullptr), key(key), red(false) {
            }
            explicit Node(Node* parent) : left(nullptr), right(nullptr), parent(parent), key(0), red(false) {
            }
            Node(Node* parent, ValueType key) : left(nullptr), right(nullptr), parent(parent), key(key), red(false) {
            }
        };

        size_t size_ = 0;
        Node* root = nullptr;
        Node* end_iter = nullptr;
        Node* begin_iter = nullptr;

        RBTree() = default;

        Node* lower_bound(Node* v, const ValueType& key) const {
            if (v == nullptr) {
                return nullptr;
            }
            if (!(v->key < key) && !(key < v->key)) {
                return v;
            } else if (v->key < key) {
                if (v->right == nullptr) {
                    return nullptr;
                } else {
                    return lower_bound(v->right, key);
                }
            } else {
                if (v->left == nullptr) {
                    return v;
                } else {
                    Node* nxt = lower_bound(v->left, key);
                    if (nxt == nullptr || v->key < nxt->key) {
                        return v;
                    } else {
                        return nxt;
                    }
                }
            }
        }

        Node* upper_bound(Node* v, const ValueType& key) const {
            if (v == nullptr) {
                return nullptr;
            }
            if (key < v->key) {
                if (v->left == nullptr) {
                    return v;
                } else {
                    Node* nxt = upper_bound(v->left, key);
                    if (nxt == nullptr || v->key < nxt->key) {
                        return v;
                    } else {
                        return nxt;
                    }
                }
            } else {
                if (v->right == nullptr) {
                    return nullptr;
                } else {
                    return upper_bound(v->right, key);
                }
            }
        }

        void UpdateCorners() {
            if (root == nullptr) {
                begin_iter = nullptr;
                end_iter = nullptr;
            } else {
                Node* v = root;
                while (v->right != nullptr) {
                    v = v->right;
                }
                end_iter = v;

                v = root;
                while (v->left != nullptr) {
                    v = v->left;
                }
                begin_iter = v;
            }
        }

        Node* insert(Node* v, const ValueType& key) {
            if (!(v->key < key) && !(key < v->key)) {
                return v;
            } else if (v->key < key) {
                if (v->right == nullptr) {
                    v->right = new Node(v, key);
                    v->right->red = true;
                    ++size_;
                    return v->right;
                } else {
                    return insert(v->right, key);
                }
            } else {
                if (v->left == nullptr) {
                    v->left = new Node(v, key);
                    v->left->red = true;
                    ++size_;
                    return v->left;
                } else {
                    return insert(v->left, key);
                }
            }
        }

        static bool IsRed(Node* v) {
            return v != nullptr && v->red;
        }
        static bool IsLeft(Node* v) {
            return v->parent->left == v;
        }
        static Node* GetUncle(Node* v) {
            if (IsLeft(v->parent)) {
                return v->parent->parent->right;
            } else {
                return v->parent->parent->left;
            }
        }
        static Node* GetDad(Node* v) {
            return v->parent;
        }
        static Node* GetBrother(Node* v) {
            if (IsLeft(v)) {
                return v->parent->right;
            } else {
                return v->parent->left;
            }
        }
        static Node* GetGrandad(Node* v) {
            return v->parent->parent;
        }
        static bool IsRoot(Node* v) {
            return GetDad(v) == nullptr;
        }

        void RotateRight(Node* v) {
            Node* left_child = v->left;
            v->left = left_child->right;
            if (v->left != nullptr) {
                v->left->parent = v;
            }
            left_child->right = v;
            if (v->parent != nullptr) {
                if (IsLeft(v)) {
                    v->parent->left = left_child;
                } else {
                    v->parent->right = left_child;
                }
            } else {
                root = left_child;
            }
            left_child->parent = v->parent;
            v->parent = left_child;
        }
        void RotateLeft(Node* v) {
            Node* right_child = v->right;
            v->right = right_child->left;
            if (v->right != nullptr) {
                v->right->parent = v;
            }
            right_child->left = v;
            if (v->parent != nullptr) {
                if (IsLeft(v)) {
                    v->parent->left = right_child;
                } else {
                    v->parent->right = right_child;
                }
            } else {
                root = right_child;
            }
            right_child->parent = v->parent;
            v->parent = right_child;
        }

        void FixInsertation(Node* v) {
            if (v->left != nullptr || v->right != nullptr || !IsRed(v)) {
                return;
            }
            while (!IsRoot(v) && IsRed(GetDad(v))) {
                if (IsLeft(v->parent)) {
                    if (IsRed(GetUncle(v))) {
                        GetUncle(v)->red = false;
                        GetDad(v)->red = false;
                        GetGrandad(v)->red = true;
                        v = GetGrandad(v);
                    } else {
                        if (!IsLeft(v)) {
                            v = v->parent;
                            RotateLeft(v);
                        }
                        GetDad(v)->red = false;
                        GetGrandad(v)->red = true;
                        RotateRight(GetGrandad(v));
                    }
                } else {
                    if (IsRed(GetUncle(v))) {
                        GetUncle(v)->red = false;
                        GetDad(v)->red = false;
                        GetGrandad(v)->red = true;
                        v = GetGrandad(v);
                    } else {
                        if (IsLeft(v)) {
                            v = v->parent;
                            RotateRight(v);
                        }
                        GetDad(v)->red = false;
                        GetGrandad(v)->red = true;
                        RotateLeft(GetGrandad(v));
                    }
                }
            }
            root->red = false;
        }

        void insert(const ValueType& x) {
            if (root == nullptr) {
                root = new Node(x);
                ++size_;
                UpdateCorners();
                return;
            }

            Node* current_node = insert(root, x);
            FixInsertation(current_node);
            UpdateCorners();
        }

        void FixErasing(Node* v) {
            while (!IsRoot(v) && !v->red) {
                if (IsLeft(v)) {
                    if (IsRed(GetBrother(v))) {
                        GetBrother(v)->red = false;
                        GetDad(v)->red = true;
                        RotateLeft(GetDad(v));
                    }
                    if (!IsRed(GetBrother(v)->left) && !IsRed(GetBrother(v)->right)) {
                        GetBrother(v)->red = true;
                        v = v->parent;
                    } else {
                        if (IsRed(GetBrother(v)->left)) {
                            GetBrother(v)->left->red = false;
                            GetBrother(v)->red = true;
                            RotateRight(GetBrother(v));
                        }
                        std::swap(GetBrother(v)->red, GetDad(v)->red);
                        GetBrother(v)->right->red = false;
                        RotateLeft(GetDad(v));
                        v = root;
                    }
                } else {
                    if (IsRed(GetBrother(v))) {
                        GetBrother(v)->red = false;
                        GetDad(v)->red = true;
                        RotateRight(GetDad(v));
                    }
                    if (!IsRed(GetBrother(v)->right) && !IsRed(GetBrother(v)->left)) {
                        GetBrother(v)->red = true;
                        v = v->parent;
                    } else {
                        if (IsRed(GetBrother(v)->right)) {
                            GetBrother(v)->right->red = false;
                            GetBrother(v)->red = true;
                            RotateLeft(GetBrother(v));
                        }
                        std::swap(GetBrother(v)->red, GetDad(v)->red);
                        GetBrother(v)->left->red = false;
                        RotateRight(GetDad(v));
                        v = root;
                    }
                }
            }
            v->red = false;
        }

        void erase(const ValueType& x) {
            Node* vert = lower_bound(root, x);
            if (vert == nullptr || !(!(vert->key < x) && !(x < vert->key))) {
                return;
            }
            --size_;
            if (vert->left != nullptr && vert->right != nullptr) {
                Node* greater = upper_bound(vert, x);
                vert->key = greater->key;
                vert = greater;
            }
            if (vert->left == nullptr && vert->right == nullptr) {
                if (vert->parent == nullptr) {
                    root = nullptr;
                    delete vert;
                } else {
                    if (!IsRed(vert)) {
                        FixErasing(vert);
                    }
                    if (IsLeft(vert)) {
                        vert->parent->left = nullptr;
                    } else {
                        vert->parent->right = nullptr;
                    }
                    delete vert;
                }
                UpdateCorners();
                return;
            }
            if (IsRed(vert)) {
                if (vert->right != nullptr) {
                    vert->left = vert->right;
                }
                if (IsLeft(vert)) {
                    GetDad(vert)->left = vert->left;
                } else {
                    GetDad(vert)->right = vert->left;
                }
                vert->left->parent = GetDad(vert);
                delete vert;
                UpdateCorners();
                return;
            } else if (IsRoot(vert)) {
                if (vert->left == nullptr) {
                    root = vert->right;
                } else {
                    root = vert->left;
                }
                root->red = false;
                root->parent = nullptr;
                delete vert;
                UpdateCorners();
                return;
            } else {
                if (vert->right != nullptr) {
                    vert->left = vert->right;
                }
                if (IsLeft(vert)) {
                    GetDad(vert)->left = vert->left;
                } else {
                    GetDad(vert)->right = vert->left;
                }
                vert->left->parent = GetDad(vert);
                Node* to_delete = vert;
                vert = vert->left;
                delete to_delete;
            }
            FixErasing(vert);
            UpdateCorners();
        }
    };

    void DfsDestruct(typename RBTree::Node* it) {
        if (it == nullptr) {
            return;
        }
        DfsDestruct(it->left);
        DfsDestruct(it->right);
        delete it;
    }

    RBTree tree_;
public:
    Set() = default;

    template<typename Iterator>
    Set(Iterator first, Iterator last) {
        for (auto i = first; i != last; ++i) {
            tree_.insert(*i);
        }
    }

    Set(std::initializer_list<ValueType> elems) {
        for (const auto& x : elems) {
            tree_.insert(x);
        }
    }

    Set(const Set& oth) {
        for (const auto& x : oth) {
            tree_.insert(x);
        }
    }

    Set& operator=(const Set& oth) {
        if (oth.tree_.root == tree_.root) {
            return *this;
        }

        DfsDestruct(tree_.root);
        tree_.size_ = 0;
        tree_.root = nullptr;
        tree_.end_iter = nullptr;
        tree_.begin_iter = nullptr;

        for (const auto& x : oth) {
            tree_.insert(x);
        }
        return *this;
    }

    ~Set() {
        DfsDestruct(tree_.root);
    }

    size_t size() const {
        return tree_.size_;
    }

    bool empty() const {
        return tree_.size_ == 0;
    }

    void insert(const ValueType& x) {
        tree_.insert(x);
    }

    void erase(const ValueType& x) {
        tree_.erase(x);
    }

    class iterator {
    public:
        using TreeNode = const typename RBTree::Node*;

        iterator() = default;
        explicit iterator(TreeNode p, bool) : node_(p), is_end_(true) {
        }
        explicit iterator(TreeNode p) : node_(p), is_end_(false) {
        }
        iterator(const iterator& other) : node_(other.node_), is_end_(other.is_end_) {
        }

        iterator& operator++() {
            iterator nxt = NextNode(*this);
            *this = nxt;
            return *this;
        }


        iterator operator++(int) {
            iterator nxt = NextNode(*this);
            auto to_return = *this;
            *this = nxt;
            return to_return;
        }
        iterator& operator--() {
            iterator nxt = PrevNode(*this);
            *this = nxt;
            return *this;
        }

        iterator operator--(int) {
            iterator nxt = PrevNode(*this);
            auto to_return = *this;
            *this = nxt;
            return to_return;
        }

        const ValueType& operator*() {
            return node_->key;
        }

        const ValueType* operator->() {
            return &node_->key;
        }

        bool operator==(const iterator& other) const {
            if (node_ == nullptr || other.node_ == nullptr) {
                return true;
            }
            return other.node_ == node_ && other.is_end_ == is_end_;
        }

        bool operator!=(const iterator& other) const {
            if (node_ == nullptr || other.node_ == nullptr) {
                return false;
            }
            return other.node_ != node_ || other.is_end_ != is_end_;
        }
    private:
        TreeNode node_ = nullptr;
        bool is_end_ = false;

        iterator NextNode(iterator p) {
            if (p.node_->right == nullptr) {
                TreeNode x = p.node_;
                while (x->parent != nullptr) {
                    if (x->parent->left == x) {
                        return iterator(x->parent);
                    } else {
                        x = x->parent;
                    }
                }
                return iterator(p.node_, true);
            } else {
                TreeNode x = p.node_->right;
                while (x->left != nullptr) {
                    x = x->left;
                }
                return iterator(x);
            }
        }

        iterator PrevNode(iterator p) {
            if (p.is_end_) {
                p.is_end_ = false;
                return p;
            }
            if (p.node_->left == nullptr) {
                TreeNode x = p.node_;
                while (x->parent != nullptr) {
                    if (x->parent->right == x) {
                        return iterator(x->parent);
                    } else {
                        x = x->parent;
                    }
                }
                return iterator(p.node_, true);
            } else {
                TreeNode x = p.node_->left;
                while (x->right != nullptr) {
                    x = x->right;
                }
                return iterator(x);
            }
        }
    };

    iterator begin() const {
        return iterator(tree_.begin_iter);
    }

    iterator end() const {
        return iterator(tree_.end_iter, true);
    }

    iterator lower_bound(const ValueType& key) const {
        if (empty()) {
            return end();
        }
        if (tree_.end_iter->key < key) {
            return end();
        } else {
            return iterator(tree_.lower_bound(tree_.root, key));
        }
    }

    iterator find(const ValueType& key) const {
        iterator it(tree_.lower_bound(tree_.root, key));
        if (it == end()) {
            return end();
        }
        if ((!(*it < key) && !(key < *it))) {
            return it;
        } else {
            return end();
        }
    }
};
