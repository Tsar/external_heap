#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "external_storage.h"

template <class T>
std::string toString(T const& val)
{
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

struct NoElementsInHeapException {};

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
			siftUp(blockNum);
    }

    /// Получить количество элементов в куче
    int64_t size()
    {
        return N;
    }

	/// Получить блок максимальных элементов (но не извлекать)
	std::vector<T> getMaxBlock()
    {
        if (N == 0)
            throw NoElementsInHeapException();

		std::vector<T> res = storage.readBlock(0);
		if (N < elementsPerBlock)
			res.resize(N);
		return res;
    }

	/// Извлечь блок максимальных элементов
	std::vector<T> extractMax()
    {
        if (N == 0)
            throw NoElementsInHeapException();

		std::vector<T> res = storage.readBlock(0);
		if (N <= elementsPerBlock)
		{
			res.resize(N);
			N = 0;
			return res;
		}

		int64_t blocksCount = (N + elementsPerBlock - 1) / elementsPerBlock;
		std::vector<T> lastBlock = storage.readBlock(blocksCount - 1);
		if (N % elementsPerBlock != 0 && N > 2 * elementsPerBlock)  // Если последний блок недозаполненный, то дополнить последними элементами из предпоследнего блока
		{
			lastBlock.resize(N % elementsPerBlock);
			std::vector<T> preLastBlock = storage.readBlock(blocksCount - 2);
			lastBlock.insert(lastBlock.end(), preLastBlock.begin() + (N % elementsPerBlock), preLastBlock.end());
			assert(lastBlock.size() == elementsPerBlock);
		}

		N -= lastBlock.size();
		storage.writeBlock(0, lastBlock);

		siftDown();

        return res;
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

	/// Создать dot-файл (использовать только для отладки)
	/// Команда для конвертации в ps: dot -Tps extheap.dot -o extheap.ps
	void exportToDOT(std::string const& fileName)
	{
		FILE* f = fopen(fileName.c_str(), "w");
		fprintf(f, "graph Heap {\n  node [shape=box];");
		int64_t blocksCount = (N + elementsPerBlock - 1) / elementsPerBlock;
		for (int64_t i = 0; i < blocksCount; ++i)
		{
			std::vector<T> block = storage.readBlock(i);
			size_t sz = (i == N / elementsPerBlock) ? (N % elementsPerBlock) : elementsPerBlock;
			std::string bStr;
			for (size_t j = 0; j < sz; ++j)
				bStr += toString(block[j]) + (j == sz - 1 ? "" : ", ");
			fprintf(f, "  b%ld [label=\"%s\"];\n", i, bStr.c_str());
		}
		fprintf(f, "\n");
		for (int64_t i = 0; i < blocksCount; ++i)
		{
			if (i * 2 + 1 < blocksCount)
				fprintf(f, "  b%ld -- b%ld;\n", i, i * 2 + 1);
			if (i * 2 + 2 < blocksCount)
				fprintf(f, "  b%ld -- b%ld;\n", i, i * 2 + 2);
		}
		fprintf(f, "}\n");
		fclose(f);
	}

private:
	/// В toBeLarger в итоге будут бОльшие значение, а в toBeSmaller - меньшие
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
	void siftUp(int64_t blockNum)
    {
        if (blockNum == 0)
            return;

        std::vector<T> block = storage.readBlock(blockNum);
        if (blockNum == N / elementsPerBlock)  // если это последняя вершина кучи (мб недозаполненная)
            block.resize(N % elementsPerBlock);

		std::vector<T> parent = storage.readBlock((blockNum - 1) >> 1);
        while (blockNum > 0 && parent[elementsPerBlock - 1] < block[0])  // Пока не корень и нарушается свойство нашей кучи (все элементы родителя >= всех потомка)
        {
            remerge(parent, block);
            storage.writeBlock(blockNum, block);

			block = parent;
			blockNum = (blockNum - 1) >> 1;
			parent = storage.readBlock((blockNum - 1) >> 1);
        }
		storage.writeBlock(blockNum, block);
    }

	/// Опускание маленьких значений вниз
	void siftDown()
	{
		/// TODO
	}

    ExternalStorage<T> storage;
    int64_t elementsPerBlock;
    int64_t N;
};
