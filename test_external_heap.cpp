//#include <stdlib.h>
#include <gtest/gtest.h>

#include "external_heap.h"

TEST(ExternalHeapTesting, TestWithOneElement)
{
	ExternalHeap<int32_t> heap("extheap.data", 4);

	heap.insert(10);
	EXPECT_EQ(heap.size(), 1);

	std::vector<int32_t> maxBlock = heap.extractMax();

	EXPECT_EQ(maxBlock.size(), 1);
	EXPECT_EQ(maxBlock[0], 10);
	EXPECT_EQ(heap.size(), 0);
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

/*
int main()
{
    ExternalHeap<int32_t> heap("extheap.data", 4);

    std::string s;
    while (s != "exit")
    {
		int32_t randVal = rand() % 1000;
        std::cout << "Inserting " << randVal << " to heap" << std::endl;
        heap.insert(randVal);
        heap.debugPrint();
		heap.exportToDOT("extheap.dot");

        std::cin >> s;
    }

    return 0;
}
*/
