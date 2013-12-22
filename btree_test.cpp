#include "btree.hpp"
#include "gtest/gtest.h"

TEST(BTreeTest, ConstructorDefaultsMakeSense) {
  btree<2, int> b;
  EXPECT_EQ(b.root->n, 0) << "Empty B-tree root did not have 0 children.";
  EXPECT_TRUE(b.root->leaf) << "Empty B-tree root was not a leaf.";
}

TEST(BTreeTest, SearchOnEmptyTree) {
  btree<2, int> b;
  EXPECT_EQ(b.search(0).first, nullptr);
}

TEST(BTreeTest, SearchBasic) {
  btree<2, int> b;
  b.root->n++;
  auto n = new decltype(b)::node_type;
  b.root->c[0].reset(n);
  b.root->keys[0] = 3;

  EXPECT_EQ(b.search(3).second, 0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
