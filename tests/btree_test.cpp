#include "../src/btree.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <vector>
#include <cstdlib>
TEST(BTreeTest, SearchOnEmptyTree) {
  btree<2, int> b;
  EXPECT_EQ(b.search(0).first, nullptr) << "Found a nonexistant element.";
}

TEST(BTreeTest, SearchInsertBasic) {
  btree<2, int> b;
  b.insert(3);
  auto r = b.search(3).first;
  EXPECT_EQ(r->keys[0], 3) << "Key 3 was not inserted properly.";
  EXPECT_EQ(r->n, 1) << "Root does not have 1 key.";
  EXPECT_TRUE(r->leaf) << "Root of a 2-B-tree with 1 key is not a leaf.";
}

TEST(BTreeTest, InsertAndSearch) {
  btree<2, int> b;
  EXPECT_EQ(b.search(2).first, nullptr) << "Incorrectly found nonexistent 2.";
  b.insert(2);
  EXPECT_NE(b.search(2).first, nullptr) << "Did not find 2 after insertion.";

  EXPECT_EQ(b.search(1).first, nullptr) << "Incorrectly found nonexistent 1.";
  b.insert(1);
  EXPECT_NE(b.search(1).first, nullptr) << "Did not find 1 after insertion.";

  auto r = b.search(1).first;
  
  EXPECT_EQ(r->keys[0], 1) << "Incorrect first key stored at root.";
  EXPECT_EQ(r->keys[1], 2) << "Incorrect second key stored at root.";
}

TEST(BTreeTest, SearchEdges) {
  btree<2, int> b;
  EXPECT_EQ(b.search(0).first, nullptr) << "Found 0.";
  b.insert(1);
  b.insert(2);
  b.insert(3);
  EXPECT_EQ(b.search(4).first, nullptr) << "Found 4.";
}

TEST(BTreeTest, Greatest) {
  btree<2, int> b;
  b.insert(4);
  EXPECT_EQ(b.greatest(), 4) << "Greatest element wasn't 4.";
  b.insert(5);
  EXPECT_EQ(b.greatest(), 5) << "Greatest element wasn't 5.";
  b.insert(1);
  b.insert(2);
  EXPECT_EQ(b.greatest(), 5) << "Greatest element wasn't 5.";
  b.insert(6);
  EXPECT_EQ(b.greatest(), 6) << "Greatest element wasn't 6.";
}

TEST(BTreeTest, Smallest) {
  btree<3, int> b;
  b.insert(6);
  EXPECT_EQ(b.smallest(), 6) << "Smallest element wasn't 6.";
  b.insert(5);
  EXPECT_EQ(b.smallest(), 5) << "Smallest element wasn't 5.";
  b.insert(8);
  b.insert(9);
  EXPECT_EQ(b.smallest(), 5) << "Smallest element wasn't 5.";
  b.insert(4);
  EXPECT_EQ(b.smallest(), 4) << "Smallest element wasn't 4.";
}

TEST(BTreeTest, DeleteInLeaf) {
  btree<2, int> b;
  b.insert(4);
  b.insert(5);
  b.remove(5);
  EXPECT_EQ(b.search(5).first, nullptr) << "Found 5.";
}

TEST(BTreeTest, DeleteRoot) {
  btree<2, int> b;
  b.insert(1);
  b.remove(1);
  EXPECT_EQ(b.search(1).first, nullptr) << "Found 1.";
  b.insert(2);
  b.remove(2);
  EXPECT_EQ(b.search(2).first, nullptr) << "Found 2.";
}

TEST(BTreeTest, DeleteLeaf) {
  btree<2, int> b;
  b.insert(4);
  b.insert(5);
  b.insert(6);
  b.insert(7);
  b.remove(7);
  EXPECT_EQ(b.search(7).first, nullptr) << "Found 7.";
  for (int i = 4; i <= 6; ++i) {
    EXPECT_NE(b.search(i).first, nullptr) << "Did not find " << i << ".";
  }
}

TEST(BTreeTest, DeleteThorough) {
  btree<2, int> b;
  int n = 1000;
  std::vector<int> v(n);
  for (int i = 0; i < n; ++i) {
    v[i] = i;
  }
  
  std::srand(0xdeadbeef);
  std::random_shuffle(v.begin(), v.end());

  for (int i = 0; i < n; ++i) {
    EXPECT_EQ(b.search(v[i]).first, nullptr) << "Found " << v[i] << ".";
    b.insert(v[i]);
    ASSERT_TRUE(b.check(-1, n)) << "Failed internal consistency check."
                                << " after inserting v[" << i << "] = "
                                << v[i] << ".";
  }

  std::random_shuffle(v.begin(), v.end());
  for (int i = 0; i < n; ++i) {
    EXPECT_NE(b.search(v[i]).first, nullptr) << "Did not find " << v[i]
                                             << ".";
    b.remove(v[i]);
    ASSERT_TRUE(b.check(-1, n)) << "Failed internal consistency check"
                                << " after removing v[" << i << "] = "
                                << v[i] << ".";
    EXPECT_EQ(b.search(v[i]).first, nullptr) << "Found " << v[i]
                                             << " after deleting it.";
  }
}
