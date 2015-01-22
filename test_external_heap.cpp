#include <stdlib.h>
#include <time.h>
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

void TestWithRandomElements(int64_t count, int64_t blockSize)
{
	ExternalHeap<int> heap("extheap.data", blockSize);
	std::vector<int> testVector;
	for (int64_t i = 0; i < count; ++i)
		testVector.push_back(rand());

	for (int64_t i = 0; i < count; ++i)
		heap.insert(testVector[i]);

	EXPECT_EQ(heap.size(), count);

	std::sort(testVector.begin(), testVector.end(), std::greater<int>());

	std::vector<int> next;
	int64_t pos = 0;
	while (!heap.empty())
	{
		next = heap.extractMax();
		EXPECT_EQ(heap.empty() || (next.size() == blockSize), true);
		for (int64_t i = 0; i < next.size(); ++i)
			EXPECT_EQ(next[i], testVector[pos++]);
	}

	EXPECT_EQ(pos, count);
	EXPECT_EQ(heap.size(), 0);
}

TEST(ExternalHeapTesting, TestWith100Elements)
{
	TestWithRandomElements(100, 5);
}

TEST(ExternalHeapTesting, TestWith10000Elements)
{
	TestWithRandomElements(10000, 4096);
}

int main(int argc, char* argv[])
{
	srand(time(0));

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
