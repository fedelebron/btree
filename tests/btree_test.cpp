#include "../src/btree.hpp"
#include "gtest/gtest.h"

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

