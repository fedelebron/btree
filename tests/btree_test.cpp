#include "../src/btree.hpp"
#include "gtest/gtest.h"

TEST(BTreeTest, ConstructorDefaultsMakeSense) {
  btree<2, int> b;
  EXPECT_EQ(b.root->n, 0) << "Empty B-tree root did not have 0 children.";
  EXPECT_TRUE(b.root->leaf) << "Empty B-tree root was not a leaf.";
}

TEST(BTreeTest, SearchOnEmptyTree) {
  btree<2, int> b;
  EXPECT_EQ(b.search(0).first, nullptr) << "Found a nonexistant element.";
}

TEST(BTreeTest, SearchBasic) {
  btree<2, int> b;
  b.root->n++;
  auto n = new decltype(b)::node_type;
  b.root->c[0].reset(n);
  b.root->keys[0] = 3;

  EXPECT_NE(b.search(3).first, nullptr) << "Failed to find 3.";
  EXPECT_EQ(b.search(3).second, 0) << "Incorrect key returned for 3.";
}

TEST(BTreeTest, InsertBasic) {
  btree<2, int> b;
  b.insert(3);

  EXPECT_EQ(b.root->keys[0], 3) << "Key 3 was not inserted properly.";
  EXPECT_EQ(b.root->n, 1) << "Root does not have 1 key.";
  EXPECT_TRUE(b.root->leaf) << "Root of a 2-B-tree with 1 key is not a leaf.";
}

TEST(BTreeTest, InsertAndSearch) {
  btree<2, int> b;
  EXPECT_EQ(b.search(2).first, nullptr) << "Incorrectly found nonexistent 2.";
  b.insert(2);
  EXPECT_NE(b.search(2).first, nullptr) << "Did not find 2 after insertion.";

  EXPECT_EQ(b.search(1).first, nullptr) << "Incorrectly found nonexistent 1.";
  b.insert(1);
  EXPECT_NE(b.search(1).first, nullptr) << "Did not find 1 after insertion.";

  EXPECT_EQ(b.root->keys[0], 1) << "Incorrect first key stored at root.";
  EXPECT_EQ(b.root->keys[1], 2) << "Incorrect second key stored at root.";
}

TEST(BTreeTest, SearchEdges) {
  btree<2, int> b;
  EXPECT_EQ(b.search(0).first, nullptr) << "Found 0.";
  b.insert(1);
  b.insert(2);
  b.insert(3);
  EXPECT_EQ(b.search(4).first, nullptr) << "Found 4.";
}


TEST(BTreeTest, InsertMillions) {
  btree<5, long long int> b;
  long long int n = 4000000;
  for (long long int i = 0; i < n; ++i) {
    long long int x = i * i % 8000009;;
    EXPECT_EQ(b.search(x).first, nullptr) << "Found " << x << ".";
    b.insert(x);
    EXPECT_NE(b.search(x).first, nullptr) << "Did not find " << x << ".";
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
