#pragma once

#include <cstdio>
#include <initializer_list>
#include <utility>

template<typename ValueType>
class Set {
private:
    struct RBTree {
        struct Node {
            Node* left = nullptr;
            Node* right = nullptr;
            Node* parent = nullptr;
            ValueType key;
            bool red = false;

            Node() = default;

            explicit Node(ValueType key) : key(key) {
            }

            explicit Node(Node* parent) : parent(parent) {
            }

            Node(Node* parent, ValueType key) : parent(parent), key(key) {
            }
        };

        size_t size_ = 0;
        Node* root_ = nullptr;
        Node* end_iterator_ = nullptr;
        Node* begin_iterator_ = nullptr;

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
                    Node* next = lower_bound(v->left, key);
                    if (next == nullptr || v->key < next->key) {
                        return v;
                    } else {
                        return next;
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

        void update_corners() {
            if (root_ == nullptr) {
                begin_iterator_ = nullptr;
                end_iterator_ = nullptr;
            } else {
                Node* v = root_;
                while (v->right != nullptr) {
                    v = v->right;
                }
                end_iterator_ = v;

                v = root_;
                while (v->left != nullptr) {
                    v = v->left;
                }
                begin_iterator_ = v;
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

        static bool is_red(Node* v) {
            return v != nullptr && v->red;
        }

        static bool is_left(Node* v) {
            return v->parent->left == v;
        }

        static Node* get_uncle(Node* v) {
            if (is_left(v->parent)) {
                return v->parent->parent->right;
            } else {
                return v->parent->parent->left;
            }
        }

        static Node* get_dad(Node* v) {
            return v->parent;
        }

        static Node* get_brother(Node* v) {
            if (is_left(v)) {
                return v->parent->right;
            } else {
                return v->parent->left;
            }
        }

        static Node* get_grandad(Node* v) {
            return v->parent->parent;
        }

        static bool is_root(Node* v) {
            return get_dad(v) == nullptr;
        }

        void rotate_right(Node* v) {
            Node* left_child = v->left;
            v->left = left_child->right;
            if (v->left != nullptr) {
                v->left->parent = v;
            }
            left_child->right = v;
            if (v->parent != nullptr) {
                if (is_left(v)) {
                    v->parent->left = left_child;
                } else {
                    v->parent->right = left_child;
                }
            } else {
                root_ = left_child;
            }
            left_child->parent = v->parent;
            v->parent = left_child;
        }

        void rotate_left(Node* v) {
            Node* right_child = v->right;
            v->right = right_child->left;
            if (v->right != nullptr) {
                v->right->parent = v;
            }
            right_child->left = v;
            if (v->parent != nullptr) {
                if (is_left(v)) {
                    v->parent->left = right_child;
                } else {
                    v->parent->right = right_child;
                }
            } else {
                root_ = right_child;
            }
            right_child->parent = v->parent;
            v->parent = right_child;
        }

        void fix_insertation(Node* v) {
            if (v->left != nullptr || v->right != nullptr || !is_red(v)) {
                return;
            }
            while (!is_root(v) && is_red(get_dad(v))) {
                if (is_left(v->parent)) {
                    if (is_red(get_uncle(v))) {
                        get_uncle(v)->red = false;
                        get_dad(v)->red = false;
                        get_grandad(v)->red = true;
                        v = get_grandad(v);
                    } else {
                        if (!is_left(v)) {
                            v = v->parent;
                            rotate_left(v);
                        }
                        get_dad(v)->red = false;
                        get_grandad(v)->red = true;
                        rotate_right(get_grandad(v));
                    }
                } else {
                    if (is_red(get_uncle(v))) {
                        get_uncle(v)->red = false;
                        get_dad(v)->red = false;
                        get_grandad(v)->red = true;
                        v = get_grandad(v);
                    } else {
                        if (is_left(v)) {
                            v = v->parent;
                            rotate_right(v);
                        }
                        get_dad(v)->red = false;
                        get_grandad(v)->red = true;
                        rotate_left(get_grandad(v));
                    }
                }
            }
            root_->red = false;
        }

        void insert(const ValueType& x) {
            if (root_ == nullptr) {
                root_ = new Node(x);
                ++size_;
                update_corners();
                return;
            }

            Node* current_node = insert(root_, x);
            fix_insertation(current_node);
            update_corners();
        }

        void fix_erasing(Node* v) {
            while (!is_root(v) && !v->red) {
                if (is_left(v)) {
                    if (is_red(get_brother(v))) {
                        get_brother(v)->red = false;
                        get_dad(v)->red = true;
                        rotate_left(get_dad(v));
                    }
                    if (!is_red(get_brother(v)->left) && !is_red(get_brother(v)->right)) {
                        get_brother(v)->red = true;
                        v = v->parent;
                    } else {
                        if (is_red(get_brother(v)->left)) {
                            get_brother(v)->left->red = false;
                            get_brother(v)->red = true;
                            rotate_right(get_brother(v));
                        }
                        std::swap(get_brother(v)->red, get_dad(v)->red);
                        get_brother(v)->right->red = false;
                        rotate_left(get_dad(v));
                        v = root_;
                    }
                } else {
                    if (is_red(get_brother(v))) {
                        get_brother(v)->red = false;
                        get_dad(v)->red = true;
                        rotate_right(get_dad(v));
                    }
                    if (!is_red(get_brother(v)->right) && !is_red(get_brother(v)->left)) {
                        get_brother(v)->red = true;
                        v = v->parent;
                    } else {
                        if (is_red(get_brother(v)->right)) {
                            get_brother(v)->right->red = false;
                            get_brother(v)->red = true;
                            rotate_left(get_brother(v));
                        }
                        std::swap(get_brother(v)->red, get_dad(v)->red);
                        get_brother(v)->left->red = false;
                        rotate_right(get_dad(v));
                        v = root_;
                    }
                }
            }
            v->red = false;
        }

        void erase(const ValueType& x) {
            Node* vert = lower_bound(root_, x);
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
                    root_ = nullptr;
                    delete vert;
                } else {
                    if (!is_red(vert)) {
                        fix_erasing(vert);
                    }
                    if (is_left(vert)) {
                        vert->parent->left = nullptr;
                    } else {
                        vert->parent->right = nullptr;
                    }
                    delete vert;
                }
                update_corners();
                return;
            }
            if (is_red(vert)) {
                if (vert->right != nullptr) {
                    vert->left = vert->right;
                }
                if (is_left(vert)) {
                    get_dad(vert)->left = vert->left;
                } else {
                    get_dad(vert)->right = vert->left;
                }
                vert->left->parent = get_dad(vert);
                delete vert;
                update_corners();
                return;
            } else if (is_root(vert)) {
                if (vert->left == nullptr) {
                    root_ = vert->right;
                } else {
                    root_ = vert->left;
                }
                root_->red = false;
                root_->parent = nullptr;
                delete vert;
                update_corners();
                return;
            } else {
                if (vert->right != nullptr) {
                    vert->left = vert->right;
                }
                if (is_left(vert)) {
                    get_dad(vert)->left = vert->left;
                } else {
                    get_dad(vert)->right = vert->left;
                }
                vert->left->parent = get_dad(vert);
                Node* to_delete = vert;
                vert = vert->left;
                delete to_delete;
            }
            fix_erasing(vert);
            update_corners();
        }
    };

    void dfs_destruct(typename RBTree::Node* it) {
        if (it == nullptr) {
            return;
        }
        dfs_destruct(it->left);
        dfs_destruct(it->right);
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

    Set(const Set& other) {
        for (const auto& x : other) {
            tree_.insert(x);
        }
    }

    Set& operator=(const Set& other) {
        if (other.tree_.root_ == tree_.root_) {
            return *this;
        }

        dfs_destruct(tree_.root_);
        tree_.size_ = 0;
        tree_.root_ = nullptr;
        tree_.end_iterator_ = nullptr;
        tree_.begin_iterator_ = nullptr;

        for (const auto& x : other) {
            tree_.insert(x);
        }
        return *this;
    }

    ~Set() {
        dfs_destruct(tree_.root_);
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
            iterator next = next_node(*this);
            *this = next;
            return *this;
        }

        iterator operator++(int) {
            iterator next = next_node(*this);
            auto to_return = *this;
            *this = next;
            return to_return;
        }
        iterator& operator--() {
            iterator next = previous_node(*this);
            *this = next;
            return *this;
        }

        iterator operator--(int) {
            iterator next = previous_node(*this);
            auto to_return = *this;
            *this = next;
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

        iterator next_node(iterator p) {
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

        iterator previous_node(iterator p) {
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
        return iterator(tree_.begin_iterator_);
    }

    iterator end() const {
        return iterator(tree_.end_iterator_, true);
    }

    iterator lower_bound(const ValueType& key) const {
        if (empty()) {
            return end();
        }
        if (tree_.end_iterator_->key < key) {
            return end();
        } else {
            return iterator(tree_.lower_bound(tree_.root_, key));
        }
    }

    iterator find(const ValueType& key) const {
        iterator it(tree_.lower_bound(tree_.root_, key));
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
