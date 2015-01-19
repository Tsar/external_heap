#include <gtest/gtest.h>

#include "external_storage.h"

TEST(ExternalStorageTesting, WriteAndReadBlock0)
{
    ExternalStorage<unsigned char> storage("storage.data", 4, true);

    std::vector<unsigned char> b1;
    b1.push_back('1');
    b1.push_back('2');
    b1.push_back('3');
    b1.push_back('4');

    /// Writing block
    storage.writeBlock(0, b1);

    /// Reading block
    std::vector<unsigned char> b2 = storage.readBlock(0);

    EXPECT_EQ(b1, b2);
}

TEST(ExternalStorageTesting, WriteAndReadBlocks)
{
    std::vector<unsigned char> b1, b2;
    {
        ExternalStorage<unsigned char> storage("storage.data", 4, true);

        b1.push_back('a');
        b1.push_back('b');
        b1.push_back('c');
        b1.push_back('d');

        storage.writeBlock(100, b1);
        b2 = storage.readBlock(100);

        EXPECT_EQ(b1, b2);

        b1[0] = 'q';
        b1[1] = 'w';
        b1[2] = 'e';
        b1[3] = 'r';

        storage.writeBlock(17, b1);
        b2 = storage.readBlock(17);

        EXPECT_EQ(b1, b2);
    }
    {
        ExternalStorage<unsigned char> storage("storage.data", 4);

        b2 = storage.readBlock(100);
        EXPECT_EQ(b2.size(), 4);
        EXPECT_NE(b1, b2);

        b2 = storage.readBlock(101);
        EXPECT_EQ(b2.size(), 0);

        b2 = storage.readBlock(17);
        EXPECT_EQ(b1, b2);
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
