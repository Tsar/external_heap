#include <stdlib.h>
#include <time.h>
#include <gtest/gtest.h>

#include "external_heap.h"

TEST(ExternalHeapTesting, TestWithOneElement)
{
    ExternalHeap<int32_t> heap("extheap.data", 4);

    heap.insert(10);
    EXPECT_EQ(heap.size(), 1);

    std::vector<int32_t> maxBlock = heap.extractMaxBlock();

    EXPECT_EQ(maxBlock.size(), 1);
    EXPECT_EQ(maxBlock[0], 10);
    EXPECT_EQ(heap.size(), 0);

    heap.printStorageStats();
}

struct Task
{
    int priority;
    int something;

    Task() : priority(0), something(rand()) {}
    Task(int priority) : priority(priority), something(rand()) {}
};

bool operator<(Task const& task1, Task const& task2)
{
    return task1.priority < task2.priority;
}

bool operator>(Task const& task1, Task const& task2)
{
    return task1.priority > task2.priority;
}

bool operator>=(Task const& task1, Task const& task2)
{
    return task1.priority >= task2.priority;
}

TEST(ExternalHeapTesting, TestWithFewElements)
{
    ExternalHeap<Task> heap("extheap.data", 3);

    heap.insert(Task(5));
    heap.insert(Task(1));
    heap.insert(Task(3));
    heap.insert(Task(6));
    heap.insert(Task(4));

    std::vector<Task> maxBlock = heap.extractMaxBlock();

    EXPECT_EQ(maxBlock.size(), 3);
    EXPECT_EQ(maxBlock[0].priority, 6);
    EXPECT_EQ(maxBlock[1].priority, 5);
    EXPECT_EQ(maxBlock[2].priority, 4);

    heap.insert(Task(5));
    heap.insert(Task(1));
    heap.insert(Task(3));
    heap.insert(Task(6));
    heap.insert(Task(4));
    heap.insert(Task(8));

    maxBlock = heap.extractMaxBlock();

    EXPECT_EQ(maxBlock.size(), 3);
    EXPECT_EQ(maxBlock[0].priority, 8);
    EXPECT_EQ(maxBlock[1].priority, 6);
    EXPECT_EQ(maxBlock[2].priority, 5);

    heap.printStorageStats();
}

void TestBlockOperationsWithRandomElements(int64_t count, int64_t blockSize, int64_t insertionBlockSize)
{
    assert(blockSize >= insertionBlockSize);

    ExternalHeap<int> heap("extheap.data", blockSize);
    std::vector<int> testVector;
    for (int64_t i = 0; i < count; ++i)
        testVector.push_back(rand());

    std::vector<int> forInsertion;
    for (int64_t i = 0; i < count; i += insertionBlockSize)
    {
        forInsertion.clear();
        forInsertion.insert(forInsertion.end(), testVector.begin() + i, (i + insertionBlockSize < count) ? testVector.begin() + i + insertionBlockSize : testVector.end());
        assert(forInsertion.size() <= insertionBlockSize);
        heap.insert(forInsertion);
    }

    EXPECT_EQ(heap.size(), count);

    std::sort(testVector.begin(), testVector.end(), std::greater<int>());

    std::vector<int> next;
    int64_t pos = 0;
    while (!heap.empty())
    {
        next = heap.extractMaxBlock();
        EXPECT_EQ(heap.empty() || (next.size() == blockSize), true);
        for (int64_t i = 0; i < next.size(); ++i)
            EXPECT_EQ(next[i], testVector[pos++]);
    }

    EXPECT_EQ(pos, count);
    EXPECT_EQ(heap.size(), 0);

    heap.printStorageStats();
}

void TestOneByOneOperationsWithRandomElements(int64_t count, int64_t blockSize)
{
    ExternalHeap<int> heap("extheap.data", blockSize);
    std::vector<int> testVector;
    for (int64_t i = 0; i < count; ++i)
        testVector.push_back(rand());

    for (int64_t i = 0; i < count; ++i)
        heap.insert(testVector[i]);

    EXPECT_EQ(heap.size(), count);

    std::sort(testVector.begin(), testVector.end(), std::greater<int>());

    int64_t pos = 0;
    while (!heap.empty())
    {
        EXPECT_EQ(heap.extractMax(), testVector[pos++]);
    }

    EXPECT_EQ(pos, count);
    EXPECT_EQ(heap.size(), 0);

    heap.printStorageStats();
}

TEST(ExternalHeapTesting, TestWith100Elements)
{
    TestBlockOperationsWithRandomElements(100, 16, 16);
    TestBlockOperationsWithRandomElements(100, 16, 11);

    TestOneByOneOperationsWithRandomElements(100, 16);
    TestOneByOneOperationsWithRandomElements(100, 16);
}

TEST(ExternalHeapTesting, TestWith10000Elements)
{
    TestBlockOperationsWithRandomElements(10000, 4096, 4096);
    TestBlockOperationsWithRandomElements(10000, 4096, 3000);
}

TEST(ExternalHeapTesting, TestWithDifferentCountsOfElements)
{
    for (int64_t count = 100000; count <= 20000000; count += 100000)
    {
        std::cout << "### Count = " << count << std::endl;
        TestBlockOperationsWithRandomElements(count, 4096, 4096);
    }
}

int main(int argc, char* argv[])
{
    srand(time(0));

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
