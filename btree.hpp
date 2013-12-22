#include <memory>
#include <utility>

template <unsigned int t,
          typename key,
          typename value> struct btree_node {
  
  /**
   * The number of children this node has.
   */
  unsigned int n;

  /**
   * The keys for the children of this node.
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
          typename key,
          typename value> struct btree {
  typedef btree_node<t, key, value> node_type;

 btree();

 /**
   * A pointer to the root of the tree.
   */
  std::unique_ptr<node_type> root;

  /**
   * Search for a node in the tree with a given key k.
   * Returns a pair (&n, i) such that k is the ith key in node n.
   * If no such node exists, n is nullptr, and i is undefined.
   */
  std::pair<node_type*, int> search(const key& k) const;

  /**
   * Split the ith child of x, assuming that x is not full, and its ith
   * child is full.
   * A node being full means it has 2t children.
   */
  void split(node_type* x, int);

private:

  /**
   * Helper function for search. Searches within a given subtree, using
   * the provided node n as a root of the subtree.
   */
  std::pair<node_type*, int> search_node(const node_type* n,
                                         const key& k) const;
};


template<unsigned int t,
         typename key,
         typename value> btree<t, key, value>::btree() :
           root(new node_type) {
  root->n = 0;
  root->leaf = true;
}

template<unsigned int t, typename key, typename value>
    std::pair<typename btree<t, key, value>::node_type*, int>
    btree<t, key, value>::search(const key& k) const {
  return search_node(root.get(), k); 
}

template<unsigned int t, typename key, typename value>
    std::pair<typename btree<t, key, value>::node_type*, int>
    btree<t, key, value>::search_node(const btree<t, key, value>::node_type* x,
                                      const key& k) const {
  int i = 0;
  while (i < x->n && k > x->keys[i]) ++i;
  if (i < x->n && k == x->keys[i]) return std::make_pair(x->c[i].get(), i);
  if (x->leaf) return std::make_pair(nullptr, -1);
  return search_node(x->c[i].get(), k);
}

template<unsigned int t, typename key, typename value>
    void btree<t, key, value>::split(btree<t, key, value>::node_type* x,
                                     int i) {
  node_type y = x->c[i];
  node_type z = new node_type;
  z->leaf = x->leaf;
  z.n = t - 1;
  for (int j = 0; j < t - 1; ++j) {
    z->keys[j] = y->keys[j + t];
  }
  if (!y->leaf) {
    for (int j = 0; j < t; ++j) {
      z->c[j] = y->c[j + t];
    }
  }
  y->n = t - 1;
  for (int j = x->n; j >= i + 1; --j) {
    x->c[j + 1] = x->c[j];
  }
  x->c[i + 1] = z;
  for (int j = x->n - 1; j >= i; --j) {
    x->keys[j + 1] = x->keys[j];
  }
  x->keys[i] = y->keys[t];
  x->n++;
}

