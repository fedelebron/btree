#include <algorithm>
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
