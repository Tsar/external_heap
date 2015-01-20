#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "external_storage.h"

template <class T>
class ExternalHeap
{
public:
    ExternalHeap(std::string const& storageFileName, int64_t elementsPerBlock)
        : storage(storageFileName, elementsPerBlock, true /* TODO: сделать возможность читать кучу из файла [1] */)
        , elementsPerBlock(elementsPerBlock)
        , N(0)  /* TODO: сделать возможность читать кучу из файла [2] */
    {
    }

    ~ExternalHeap()
    {
    }

    /// Добавление элемента
    void insert(T const& element)
    {
        int64_t blockNum = N / elementsPerBlock;
        std::vector<T> block;
        bool siftupNeeded = false;

        if ((N % elementsPerBlock) == 0)
        {
            // Создаём новую вершину кучи
            block.push_back(element);
            siftupNeeded = true;
        }
        else
        {
            // Используем последнюю недозаполненную вершину кучи
            block = storage.readBlock(blockNum);  // Чтение блока
            assert(block.size() == elementsPerBlock);

            T largest = block[0];

            if (largest < element)  // Если нарушится свойство кучи, запоминаем
                siftupNeeded = true;

            // Добавляем элемент в последнюю недозаполненную вершину кучи
            block[N % elementsPerBlock] = element;
            std::sort(block.begin(), block.begin() + (N % elementsPerBlock) + 1, std::greater<T>());
        }

        storage.writeBlock(blockNum, block);  // Запись блока
        ++N;

        if (siftupNeeded)
            siftup(blockNum);
    }

    /// Распечатать содержимое кучи (использовать только для отладки)
    void debugPrint()
    {
        printf("=================================================\n");
        printf("Elements in heap:   %ld\n", N);
        printf("Elements per block: %ld\n\n", elementsPerBlock);

        int64_t blocksCount = (N + elementsPerBlock - 1) / elementsPerBlock;
        for (int64_t i = 0; i < blocksCount; ++i)
        {
            std::vector<T> block = storage.readBlock(i);
            size_t sz = (i == N / elementsPerBlock) ? (N % elementsPerBlock) : elementsPerBlock;

            printf("Block %2ld: ", i);
            for (size_t j = 0; j < sz; ++j)
            {
                std::cout << std::setw(5) << block[j];
            }
            printf("\n");
        }
        printf("\n");
    }

private:
    void remerge(std::vector<T>& toBeLarger, std::vector<T>& toBeSmaller)
    {
        assert(toBeLarger.size() == elementsPerBlock);

        // TODO: может быть следует улучшить и делать настоящий merge
        toBeLarger.insert(toBeLarger.end(), toBeSmaller.begin(), toBeSmaller.end());
        std::sort(toBeLarger.begin(), toBeLarger.end(), std::greater<T>());

        toBeSmaller.clear();
        toBeSmaller.insert(toBeSmaller.end(), toBeLarger.begin() + elementsPerBlock, toBeLarger.end());
        toBeLarger.resize(elementsPerBlock);
    }

    /// Поднятие больших значений наверх
    void siftup(int64_t blockNum)
    {
        if (blockNum == 0)
            return;

        std::vector<T> block = storage.readBlock(blockNum);
        if (blockNum == N / elementsPerBlock)  // если это последняя вершина кучи (мб недозаполненная)
            block.resize(N % elementsPerBlock);

        int64_t parentNum = (blockNum - 1) >> 1;
        std::vector<T> parent = storage.readBlock(parentNum);
        while (blockNum > 0 && parent[elementsPerBlock - 1] < block[0])  // Пока не корень и нарушается свойство нашей кучи (все элементы родителя >= всех потомка)
        {
            remerge(parent, block);
            storage.writeBlock(blockNum, block);
            storage.writeBlock(parentNum, parent);  // TODO: думать: по идее этот write должен быть уже после всего цикла и 1 раз

            block = parent;
            blockNum = parentNum;
            parentNum = (parentNum - 1) >> 1;
            parent = storage.readBlock(parentNum);
        }
    }

    ExternalStorage<T> storage;
    int64_t elementsPerBlock;
    int64_t N;
};
