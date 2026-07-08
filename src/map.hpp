/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   typedef pair<const Key, T> value_type;

   enum Color { RED, BLACK };

   struct Node {
       value_type data;
       Node *left, *right, *parent;
       Color color;

       Node(const value_type &val) 
           : data(val), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
       Node(value_type &&val) 
           : data(std::move(val)), left(nullptr), right(nullptr), parent(nullptr), color(RED) {}
   };

   class iterator {
      private:
       Node *node;
       const map *map_ptr;

       Node* get_successor(Node *n) const {
           if (n->right) {
               Node *curr = n->right;
               while (curr->left) curr = curr->left;
               return curr;
           }
           Node *p = n->parent;
           while (p && n == p->right) {
               n = p;
               p = p->parent;
           }
           return p;
       }

       Node* get_predecessor(Node *n) const {
           if (n->left) {
               Node *curr = n->left;
               while (curr->right) curr = curr->right;
               return curr;
           }
           Node *p = n->parent;
           while (p && n == p->left) {
               n = p;
               p = p->parent;
           }
           return p;
       }

      public:
       iterator() : node(nullptr), map_ptr(nullptr) {}
       iterator(Node *n, const map *m) : node(n), map_ptr(m) {}
       iterator(const iterator &other) : node(other.node), map_ptr(other.map_ptr) {}

       iterator operator++(int) {
           iterator tmp = *this;
           ++(*this);
           return tmp;
       }

       iterator &operator++() {
           if (!node) throw invalid_iterator();
           if (node == map_ptr->header) throw invalid_iterator();
           node = get_successor(node);
           return *this;
       }

       iterator operator--(int) {
           iterator tmp = *this;
           --(*this);
           return tmp;
       }

       iterator &operator--() {
           if (!node) throw invalid_iterator();
           if (node == map_ptr->header) {
               node = map_ptr->header->right;
               if (!node) throw invalid_iterator();
           } else {
               node = get_predecessor(node);
           }
           if (!node) throw invalid_iterator();
           return *this;
       }

       value_type &operator*() const {
           if (!node || node == map_ptr->header) throw invalid_iterator();
           return node->data;
       }

       bool operator==(const iterator &rhs) const { return node == rhs.node; }
       bool operator!=(const iterator &rhs) const { return node != rhs.node; }

       value_type *operator->() const noexcept {
           if (!node || node == map_ptr->header) return nullptr;
           return &(node->data);
       }
   };

   class const_iterator {
      private:
       Node *node;
       const map *map_ptr;

       Node* get_successor(Node *n) const {
           if (n->right) {
               Node *curr = n->right;
               while (curr->left) curr = curr->left;
               return curr;
           }
           Node *p = n->parent;
           while (p && n == p->right) {
               n = p;
               p = p->parent;
           }
           return p;
       }

       Node* get_predecessor(Node *n) const {
           if (n->left) {
               Node *curr = n->left;
               while (curr->right) curr = curr->right;
               return curr;
           }
           Node *p = n->parent;
           while (p && n == p->left) {
               n = p;
               p = p->parent;
           }
           return p;
       }

      public:
       const_iterator() : node(nullptr), map_ptr(nullptr) {}
       const_iterator(Node *n, const map *m) : node(n), map_ptr(m) {}
       const_iterator(const const_iterator &other) : node(other.node), map_ptr(other.map_ptr) {}
       const_iterator(const iterator &other) : node(other.node), map_ptr(other.map_ptr) {}

       const_iterator operator++(int) {
           const_iterator tmp = *this;
           ++(*this);
           return tmp;
       }

       const_iterator &operator++() {
           if (!node) throw invalid_iterator();
           if (node == map_ptr->header) throw invalid_iterator();
           node = get_successor(node);
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator tmp = *this;
           --(*this);
           return tmp;
       }

       const_iterator &operator--() {
           if (!node) throw invalid_iterator();
           if (node == map_ptr->header) {
               node = map_ptr->header->right;
               if (!node) throw invalid_iterator();
           } else {
               node = get_predecessor(node);
           }
           if (!node) throw invalid_iterator();
           return *this;
       }

       const value_type &operator*() const {
           if (!node || node == map_ptr->header) throw invalid_iterator();
           return node->data;
       }

       bool operator==(const const_iterator &rhs) const { return node == rhs.node; }
       bool operator!=(const const_iterator &rhs) const { return node != rhs.node; }

       const value_type *operator->() const noexcept {
           if (!node || node == map_ptr->header) return nullptr;
           return &(node->data);
       }
   };

   map() : _size(0) {
       header = new Node(value_type(Key(), T()));
       header->color = BLACK;
       header->left = header->right = header->parent = nullptr;
   }

   map(const map &other) : _size(0) {
       header = new Node(value_type(Key(), T()));
       header->color = BLACK;
       header->left = header->right = header->parent = nullptr;
       for (auto it = other.begin(); it != other.end(); ++it) {
           insert(*it);
       }
   }

   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           for (auto it = other.begin(); it != other.end(); ++it) {
               insert(*it);
           }
       }
       return *this;
   }

   ~map() {
       clear();
       delete header;
   }

   T &at(const Key &key) {
       Node *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data.second;
   }

   const T &at(const Key &key) const {
       Node *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data.second;
   }

   T &operator[](const Key &key) {
       Node *n = find_node(key);
       if (n) return n->data.second;
       
       value_type vt(key, T());
       auto res = insert(vt);
       return res.first->second; // This is a bit of a hack, we need the value ref
   }
   
   // Overload for operator[] to return T& from the inserted node
   T &operator[](const Key &key) {
       Node *n = find_node(key);
       if (n) return n->data.second;
       
       value_type vt(key, T());
       pair<iterator, bool> res = insert(vt);
       return (*res.first).second;
   }

   const T &operator[](const Key &key) const {
       Node *n = find_node(key);
       if (!n) throw index_out_of_bound();
       return n->data.second;
   }

   iterator begin() {
       Node *n = header->right;
       if (!n) return iterator(header, this);
       while (n->left) n = n->left;
       return iterator(n, this);
   }

   const_iterator cbegin() const {
       Node *n = header->right;
       if (!n) return const_iterator(header, this);
       while (n->left) n = n->left;
       return const_iterator(n, this);
   }

   iterator end() {
       return iterator(header, this);
   }

   const_iterator cend() const {
       return const_iterator(header, this);
   }

   bool empty() const { return _size == 0; }
   size_t size() const { return _size; }

   void clear() {
       while (!empty()) {
           erase(begin());
       }
   }

   pair<iterator, bool> insert(const value_type &value) {
       Node *z = new Node(value.first, value.second);
       Node *y = nullptr;
       Node *x = header->right;

       while (x != nullptr) {
           y = x;
           if (Compare()(value.first, x->data.first)) x = x->left;
           else if (Compare()(x->data.first, value.first)) x = x->right;
           else {
               delete z;
               return {iterator(x, this), false};
           }
       }

       z->parent = y;
       if (!y) {
           header->right = z;
       } else if (Compare()(value.first, y->data.first)) {
           y->left = z;
       } else {
           y->right = z;
       }

       insert_fixup(z);
       _size++;
       return {iterator(z, this), true};
   }

   // Helper for operator[] and insert
   pair<iterator, bool> insert(value_type &&value) {
       Node *z = new Node(std::move(value));
       Node *y = nullptr;
       Node *x = header->right;
       Key key_copy = z->data.first;

       while (x != nullptr) {
           y = x;
           if (Compare()(key_copy, x->data.first)) x = x->left;
           else if (Compare()(x->data.first, key_copy)) x = x->right;
           else {
               delete z;
               return {iterator(x, this), false};
           }
       }

       z->parent = y;
       if (!y) {
           header->right = z;
       } else if (Compare()(key_copy, y->data.first)) {
           y->left = z;
       } else {
           y->right = z;
       }

       insert_fixup(z);
       _size++;
       return {iterator(z, this), true};
   }

   void erase(iterator pos) {
       Node *z = pos.node;
       if (!z || z == header) throw invalid_iterator();
       
       Node *y = z;
       Node *x;
       Color y_original_color = y->color;

       if (!z->left) {
           x = z->right;
           transplant(z, z->right);
       } else if (!z->right) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_original_color = y->color;
           x = y->right;
           if (y->parent == z) {
               if (x) x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       delete z;
       if (y_original_color == BLACK) {
           erase_fixup(x, (x ? x->parent : (y ? y->parent : header)));
       }
       _size--;
   }

   size_t count(const Key &key) const {
       return find_node(key) ? 1 : 0;
   }

   iterator find(const Key &key) {
       Node *n = find_node(key);
       return iterator(n, this);
   }

   const_iterator find(const Key &key) const {
       Node *n = find_node(key);
       return const_iterator(n, this);
   }

  private:
   Node *header;
   size_t _size;

   Node* find_node(const Key &key) const {
       Node *curr = header->right;
       while (curr) {
           if (Compare()(key, curr->data.first)) curr = curr->left;
           else if (Compare()(curr->data.first, key)) curr = curr->right;
           else return curr;
       }
       return nullptr;
   }

   Node* minimum(Node *x) const {
       while (x->left) x = x->left;
       return x;
   }

   void rotate_left(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left) y->left->parent = x;
       y->parent = x->parent;
       if (!x->parent) header->right = y;
       else if (x == x->parent->left) x->parent->left = y;
       else x->parent->right = y;
       y->left = x;
       x->parent = y;
   }

   void rotate_right(Node *y) {
       Node *x = y->left;
       y->left = x->right;
       if (x->right) x->right->parent = y;
       x->parent = y->parent;
       if (!y->parent) header->right = x;
       else if (y == y->parent->left) y->parent->left = x;
       else y->parent->right = x;
       x->right = y;
       y->parent = x;
   }

   void insert_fixup(Node *z) {
       while (z->parent && z->parent->color == RED) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y && y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       rotate_left(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   rotate_right(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y && y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rotate_right(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   rotate_left(z->parent->parent);
               }
           }
       }
       header->right ? header->right->color = BLACK : 0; // Root is always black
       // In this implementation, the actual root is header->right
       if (header->right) header->right->color = BLACK;
   }

   void transplant(Node *u, Node *v) {
       if (!u->parent) header->right = v;
       else if (u == u->parent->left) u->parent->left = v;
       else u->parent->right = v;
       if (v) v->parent = u->parent;
   }

   void erase_fixup(Node *x, Node *x_parent) {
       while (x != header->right && (!x || x->color == BLACK)) {
           if (!x_parent) break;
           if (x == x_parent->left) {
               Node *w = x_parent->right;
               if (w && w->color == RED) {
                   w->color = BLACK;
                   x_parent->color = RED;
                   rotate_left(x_parent);
                   w = x_parent->right;
               }
               if ((!w->left || w->left->color == BLACK) && 
                   (!w->right || w->right->color == BLACK)) {
                   w->color = RED;
                   x = x_parent;
                   x_parent = x->parent;
               } else {
                   if (!w->right || w->right->color == BLACK) {
                       if (w->left) w->left->color = BLACK;
                       w->color = RED;
                       rotate_right(w);
                       w = x_parent->right;
                   }
                   w->color = x_parent->color;
                   x_parent->color = BLACK;
                   if (w->right) w->right->color = BLACK;
                   rotate_left(x_parent);
                   x = header->right;
               }
           } else {
               Node *w = x_parent->left;
               if (w && w->color == RED) {
                   w->color = BLACK;
                   x_parent->color = RED;
                   rotate_right(x_parent);
                   w = x_parent->left;
               }
               if ((!w->right || w->right->color == BLACK) && 
                   (!w->left || w->left->color == BLACK)) {
                   w->color = RED;
                   x = x_parent;
                   x_parent = x->parent;
               } else {
                   if (!w->left || w->left->color == BLACK) {
                       if (w->right) w->right->color = BLACK;
                       w->color = RED;
                       rotate_left(w);
                       w = x_parent->left;
                   }
                   w->color = x_parent->color;
                   x_parent->color = BLACK;
                   if (w->left) w->left->color = BLACK;
                   rotate_right(x_parent);
                   x = header->right;
               }
           }
       }
       if (x) x->color = BLACK;
   }
};

}

#endif
