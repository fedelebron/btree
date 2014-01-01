#include <algorithm>
#include <cassert>
#include <memory>
#include <ostream>
#include <utility>
template <unsigned int t,
          typename key> struct btree_node {
  /**
   * The number of keys this node has.
   */
  unsigned int n;

  /**
   * The keys for this node.
   */
  key keys[2 * t - 1];

  /**
   * Whether or not this node is a leaf.
   */
  bool leaf;

  /**
   * Pointers to this node's children.
   */
  std::unique_ptr<btree_node> c[2 * t];
};

template <unsigned int t,
          typename key> struct btree {

  typedef btree_node<t, key> node_type;

  btree();

  /**
   * Search for a node in the tree with a given key k.
   * Returns a pair (&n, i) such that k is the ith key in node n.
   * If no such node exists, n is nullptr, and i is undefined.
   */
  std::pair<const node_type*, int> search(const key& k) const;

  /**
   * Insert a key into the tree.
   * If the key exists, does nothing.
   */
  void insert(const key&);

  /**
   * Remove a key into the tree.
   * If the key does not exist, does nothing.
   */
  void remove(const key&);

  /**
   * Finds the greatest key in the tree.
   * Assumes the tree is not empty.
   */
  const key& greatest() const;

  /**
   * Finds the smallest key in the tree.
   * Assumes the tree is not empty.
   */
  const key& smallest() const;

  /**
   * Check B-tree invariants. The values of the tree must be >= lower,
   * and <= upper.
   */
  bool check(const key& lower, const key& upper) const;

  /**
   * Dump a graphviz visualization of the tree to the given stream.
   */
  template<unsigned int t_, typename key_>
    friend std::ostream& operator<<(std::ostream&, btree<t_, key_>&);
private:

  /**
   * A pointer to the root of the tree.
   */
  std::unique_ptr<node_type> root;

  /**
   * Helper function for search. Searches within a given subtree, using
   * the provided node n as a root of the subtree.
   */
  std::pair<const node_type*, int> search_node(const node_type* n,
                                               const key& k) const;
  /**
   * Finds the greatest element in a given subtree.
   */
  std::pair<node_type*, int> greatest_in_subtree(node_type* r) const;

  /**
   * Finds the smallest element in a given subtree.
   */
  std::pair<node_type*, int> smallest_in_subtree(node_type* r) const;

  /**
   * Split the ith child of x, assuming that x is not full, and its ith
   * child is full.
   * A node being full means it has 2t children.
   */
  void split(node_type* x, int);

  /**
   * Helper function for insert.
   * Inserts the key at the subtree rooted at n, assuming n is not full.
   */
  void insert_nonfull(node_type* n, const key&);

  /**
   * Helper function for check. Recursively checks the subtree rooted at
   * the given node.
   */
  bool check_node(const node_type*, bool, const key&, const key&) const;

  /**
   * Given a parent node p, a minimal child p.c[i], and a non-minimal sibling
   * s of p.c[i] (left sibling if left == true, else right), we "rotate" a
   * key from s up to p, and from p down to p.c[i].
   */
  void rotate(node_type* p, unsigned int i, bool left);

  /**
   * Merges the ith and (i+1)th children of p, assumed to both have t - 1 keys,
   * into a single 2 * t - 1 key node, using the parent's ith key as the new
   * node's median key.
   * Returns the merged node (useful for merging a singleton root's children).
   */
  node_type* merge(node_type* p, int i);

  /**
   * Assuming x is a leaf, removes the ith key from x.
   */
  void remove_from_leaf(node_type* x, int i);

  /**
   * Helper function for remove_recursive, removes the
   * greatest key in the subtree rooted at r, and returns
   * said key.
   */
  key remove_greatest(node_type* r);

  /**
   * Helper function for remove_recursive, removes the
   * smallest key in the subtree rooted at r, and returns
   * said key.
   */
  key remove_smallest(node_type* r);

  /**
   * Delete the key k from the subtree rooted at r.
   */
  void remove_recursive(node_type* r, const key& k);

  /**
   * Dump the subtree rooted at this node as in graphviz format to the
   * given output stream.
   */
  void dump_subtree_graphviz(const node_type*, std::ostream&) const;
};


template<unsigned int t, typename key>
    btree<t, key>::btree() : root(new node_type) {
  root->n = 0;
  root->leaf = true;
}

template<unsigned int t, typename key>
    std::pair<const typename btree<t, key>::node_type*, int>
    btree<t, key>::search(const key& k) const {
  return search_node(root.get(), k);
}

template<unsigned int t, typename key>
    std::pair<const typename btree<t, key>::node_type*, int>
    btree<t, key>::search_node(const btree<t, key>::node_type* x,
                               const key& k) const {
  unsigned int i;
#ifdef BINARY_SEARCH
  const key* r = std::lower_bound(x->keys, x->keys + x->n, k);
  i = r - x->keys;
#else
  i = 0;
  while (i < x->n && k > x->keys[i]) ++i;
#endif
  if (i < x->n && k == x->keys[i]) return std::make_pair(x, i);
  if (x->leaf) return std::make_pair(nullptr, -1);
  return search_node(x->c[i].get(), k);
}

template<unsigned int t, typename key>
    void btree<t, key>::split(btree<t, key>::node_type* x, int i) {
  node_type* y = x->c[i].get();
  /* z will be x's new child, with the rightmost half of
   * y's keys and children */
  node_type* z = new node_type;
  z->leaf = y->leaf;
  z->n = t - 1;
  for (unsigned int j = 0; j < t - 1; ++j) {
    z->keys[j] = y->keys[j + t];
  }
  if (!y->leaf) {
    for (unsigned int j = 0; j < t; ++j) {
      z->c[j].reset(y->c[j + t].release());
    }
  }
  y->n = t - 1;
  for (int j = x->n; j >= i + 1; --j) {
    x->c[j + 1].reset(x->c[j].release());
  }
  x->c[i + 1].reset(z);
  for (int j = x->n - 1; j >= i; --j) {
    x->keys[j + 1] = x->keys[j];
  }
  x->keys[i] = y->keys[t - 1];
  x->n++;
}

template <unsigned int t, typename key>
    void btree<t, key>::insert(const key& k) {
  if (root->n == 2 * t - 1) {
    auto r = root.release();
    root.reset(new node_type);
    root->leaf = false;
    root->n = 0;
    root->c[0].reset(r);
    split(root.get(), 0);
    insert_nonfull(root.get(), k);
  } else insert_nonfull(root.get(), k);
}

template <unsigned int t, typename key>
    void btree<t, key>::insert_nonfull(btree_node<t, key>* x, const key& k) {
  int i = x->n - 1;
  if (x->leaf) {
    while (i >= 0 && k < x->keys[i]) {
      x->keys[i + 1] = x->keys[i];
      --i;
    }
    x->keys[i + 1] = k;
    x->n = x->n + 1;
  } else {
    while (i >= 0 && k < x->keys[i]) {
      --i;
    }
    ++i;
    if (x->c[i]->n == 2 * t - 1) {
      split(x, i);
      if (k > x->keys[i]) ++i;
    }
    insert_nonfull(x->c[i].get(), k);
  }
}

template <unsigned int t, typename key>
    bool btree<t, key>::check(const key& lower, const key& upper) const {
  return check_node(root.get(), true, lower, upper);
}

template <unsigned int t, typename key>
    bool btree<t, key>::check_node(const btree<t, key>::node_type* x,
                                   bool is_root,
                                   const key& lower,
                                   const key& upper) const {
  int n = x->n;
  if (!is_root && n < t - 1) return false;

  if (n > 0 && !x->leaf && x->keys[0] <= lower) return false;
  if (n > 0 && !x->leaf && !check_node(x->c[0].get(),
                           false,
                           lower,
                           x->keys[0])) return false;
  for (int i = 1; i < n - 1; ++i) {
    if (x->keys[i] >= x->keys[i + 1]) return false;
    if (!x->leaf && !check_node(x->c[i].get(),
                    false,
                    x->keys[i - 1],
                    x->keys[i])) return false;
  }
  if (n >= 1 && x->keys[n - 1] >= upper) return false;
  if (n >= 2 && !x->leaf && !check_node(x->c[n - 1].get(),
                            false,
                            x->keys[n - 2],
                            x->keys[n - 1])) return false;
  if (n >= 1 && !x->leaf && !check_node(x->c[n].get(),
                            false,
                            x->keys[n - 1],
                            upper)) return false;

  return true;
}

template <unsigned int t, typename key> void btree<t, key>::rotate(
    typename btree<t, key>::node_type* parent,
    unsigned int i,
    bool left) {
  node_type* child = parent->c[i].get();
  /* am I removing a key from the left sibling? */
  if (left) {
    node_type* sibling = parent->c[i - 1].get();
    unsigned int n = child->n;
    /* make room in c, shifting all keys and children to the right */
    child->c[n + 1].reset(child->c[n].release());
    for (unsigned int j = child->n; j >= 1; --j) {
      child->keys[j] = child->keys[j - 1];
      child->c[j].reset(child->c[j - 1].release());
    }
    child->n++;
    /* lower the parent's key down to the child */
    child->keys[0] = parent->keys[i - 1];
    /* raise the sibling's last key to the parent */
    parent->keys[i - 1] = sibling->keys[sibling->n - 1];
    /* hang sibling's last child at the beginning of child */
    child->c[0].reset(sibling->c[sibling->n].release());
    sibling->n--;
  } else {
    node_type* sibling = parent->c[i + 1].get();
    unsigned int n = child->n;
    /* lower the parent's key down to the child */
    child->keys[n] = parent->keys[i];
    /* raise the sibling's first key to the parent */
    parent->keys[i] = sibling->keys[0];
    /* hang sibling's first child at the end of child */
    child->c[n + 1].reset(sibling->c[0].release());
    child->n++;
    /* shift everything in sibling to the left */
    for (unsigned int i = 1; i < sibling->n; ++i) {
      sibling->keys[i - 1] = sibling->keys[i];
      sibling->c[i - 1].reset(sibling->c[i].release());
    }
    sibling->c[sibling->n - 1].reset(sibling->c[sibling->n].release());
    sibling->n--;
  }
}

template <unsigned int t, typename key> typename btree<t, key>::node_type* btree<t, key>::merge(
    typename btree<t, key>::node_type* parent,
    int i) {
  /* we'll merge the ith and i+1th children of parent */
  node_type* left = parent->c[i].get();
  node_type* right = parent->c[i + 1].get();

  assert(left->n == t - 1);
  assert(right->n == t - 1);

  /* lower the parent's ith key, the median for the new merged node */
  left->keys[t - 1] = parent->keys[i];

  /* move over right's keys to left, after the parent's key */
  for (unsigned int j = 0; j < t - 1; ++j) {
    left->keys[t + j] = right->keys[j];
    left->c[t + j].reset(right->c[j].release());
  }
  left->c[2 * t - 1].reset(right->c[t - 1].release());

  /* 2 * (t - 1) + 1 = 2 * t - 1 */
  left->n = 2 * t - 1;

  /* clear out the empty slot in the parent */
  parent->c[i + 1].reset();
  /* move over the parent's keys and children */
  for (unsigned int j = i; j < parent->n - 1; ++j) {
    parent->keys[j] = parent->keys[j + 1];
    parent->c[j + 1].reset(parent->c[j + 2].release());
  }
  parent->n--;

  return left;
}

template <unsigned int t, typename key> void btree<t, key>::remove_from_leaf(
    typename btree<t, key>::node_type* x,
    int i) {
  assert(x->leaf);
  for (int j = i; j < x->n - 1; ++j) {
    x->keys[j] = x->keys[j + 1];
  }
  x->n--;
}

template <unsigned int t, typename key>
    std::pair<typename btree<t, key>::node_type*, int> btree<t, key>::greatest_in_subtree(
      typename btree<t, key>::node_type* x) const {
  if (x->leaf) return std::make_pair(x, x->n - 1);
  return greatest_in_subtree(x->c[x->n].get());
}

template <unsigned int t, typename key>
    std::pair<typename btree<t, key>::node_type*, int> btree<t, key>::smallest_in_subtree(
      typename btree<t, key>::node_type* x) const {
  if (x->leaf) return std::make_pair(x, 0);
  return smallest_in_subtree(x->c[0].get());
}

template <unsigned int t, typename key> const key& btree<t, key>::greatest() const {
  assert(root->n);
  node_type* x;
  int i;
  std::tie(x, i) = greatest_in_subtree(root.get());
  return x->keys[i];
}

template <unsigned int t, typename key> const key& btree<t, key>::smallest() const {
  assert(root->n);
  node_type* x;
  int i;
  std::tie(x, i) = smallest_in_subtree(root.get());
  return x->keys[i];
}

template <unsigned int t, typename key> key btree<t, key>::remove_greatest(
    typename btree<t, key>::node_type* x) {
  /* invariant: x has at least t keys */
  /* if x is a leaf of >= t keys, we just remove the last one */
  if (x->leaf) {
    x->n--;
    return x->keys[x->n];
  }
  /* if the last child has >= t keys,
   * we remove the greatest key rooted at it.
   */
  node_type* z = x->c[x->n].get();
  if (z->n >= t) {
    return remove_greatest(z);
  }
  /* z is minimal, so we can't step into it.
   * if z's sibling has an extra key, rotate it
   * onto z, and delete the greatest key rooted at z.
   */
  node_type* y = x->c[x->n - 1].get();
  if (y->n >= t) {
    rotate(x, x->n, true);
    return remove_greatest(z);
  }

  /* z is minimal and so is its sibling y.
   * merge them both, then remove the greatest
   * key rooted at the merged node.
   */
  return remove_greatest(merge(x, x->n - 1));
}

template <unsigned int t, typename key> key btree<t, key>::remove_smallest(
    typename btree<t, key>::node_type* x) {
  /* see remove_greatest for comments */
  if (x->leaf) {
    key tmp = x->keys[0];
    remove_from_leaf(x, 0);
    return tmp;
  }

  node_type* z = x->c[0].get();
  if (z->n >= t) {
    return remove_smallest(z);
  }

  node_type* y = x->c[1].get();
  if (y->n >= t) {
    rotate(x, 0, false);
    return remove_smallest(z);
  }

  return remove_smallest(merge(x, 0));
}

template <unsigned int t, typename key> void btree<t, key>::remove_recursive(
    typename btree<t, key>::node_type* x,
    const key& k) {
  /* invariant: either x == tree.root.get(), or x->n >= t */
  assert(x->n >= t || x == root.get());
  int i = 0;
  while (i < x->n && x->keys[i] < k) ++i;
  if (i < x->n && x->keys[i] == k) {
    if (x->leaf) {
      /* k was found in x, and x is a leaf, simply remove k */
      remove_from_leaf(x, i);
    } else {
      /* k was found in x, but x is not a leaf.
       * replace x->keys[i] with its successor or predecessor, and
       * remove this other key.
       */
      if (x->c[i]->n >= t) {
        x->keys[i] = remove_greatest(x->c[i].get());
      } else if (x->c[i + 1]->n >= t) {
        x->keys[i] = remove_smallest(x->c[i + 1].get());
      } else {
        node_type* merged = merge(x, i);
        if (x->n == 0) {
          /* if we just left the root keyless,
           * the merged node is the new root
           */
          assert(x == root.get());
          x->c[0].release();
          root.reset(merged);
        }
        remove_recursive(merged, k);
      }
    }
  } else {
    /* k was not in x. if it exists, it's in subtree r. */
    if (x->leaf) return;
    node_type* r = x->c[i].get();
    if (r->n == t - 1) {
      /* we'd like to recursively remove k in r,
       * but r does not satisfy the invariant r->n >= t,
       * and it certainly is not the root of the tree.
       * if r has a sibling with >= t keys, we lower
       * a key from x to r, substituting this key in x
       * with a key from this sibling.
       */
      if (i < x->n && x->c[i + 1]->n >= t) {
        rotate(x, i, false);
      } else if (i && x->c[i - 1]->n >= t) {
        rotate(x, i, true);
      } else {
        /* x, and both of its siblings, all have t - 1
         * keys. i grab r and its next sibling (or
         * previous sibling if r is the last child
         * of x) and merge them, removing a key from x.
         */
        node_type* merged;
        if (i == x->n) {
          merged = merge(x, i - 1);
        } else {
          merged = merge(x, i);
        }
        /* if x was the root, and x->n == 0,
         * then merged is now the new root.
         */
        if (x->n == 0) {
          assert(root.get() == x);
          assert(root->c[0].get() == merged);
          /* we can't just kill the root,
           * since it still owns what merged
           * now points to. so we release root's
           * ownership of the new merged node,
           * and then kill the root.
           */
          root->c[0].release();
          root.reset(merged);
        }
        /* after merging, now we should look for
         * k inside the new merged node
         */
        r = merged;
      }
    }
    /* remove k from its subtree, knowing r->n >= t */
    remove_recursive(r, k);
  }
}

template <unsigned int t, typename key> void btree<t, key>::remove(const key& k) {
  remove_recursive(root.get(), k);
}

template <unsigned int t, typename key>
    std::ostream& operator<<(std::ostream& o, btree<t, key>& tree) {
  o << "digraph G{splines=false;node[fontname=\"helvetica\"];";
  tree.dump_subtree_graphviz(tree.root.get(), o);
  o << "}";
  return o;
}

template <unsigned int t, typename key> void btree<t, key>::dump_subtree_graphviz(
    const typename btree<t, key>::node_type* node, std::ostream& o) const {
  o << "node" << node << "[shape=none;label=<<table style=\"rounded\"";
  o << " border=\"0\" bgcolor=\"deepskyblue\" cellspacing=\"4\"><tr>";
  for (unsigned int i = 0; i < node->n; ++i) {
    o << "<td port=\"child" << i << "\" bgcolor=\"gray\" border=\"1\"></td>";
    o << "<td port=\"key" << i << "\" bgcolor=\"white\" border=\"1\">";
    o << node->keys[i] << "</td>";
  }
  o << "<td port=\"child" << node->n << "\" border=\"1\" bgcolor=\"gray\"></td>";
  o << "</tr></table>>]";

  if (!node->leaf) {
    for (unsigned int i = 0; i <= node->n; ++i) {
      o << "node" << node << ":child" << i << ":c -> node" << node->c[i].get() << ";";
    }

    for (unsigned int i = 0; i <= node->n; ++i) {
      dump_subtree_graphviz(node->c[i].get(), o);
    }
  }
}
