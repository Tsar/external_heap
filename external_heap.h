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
struct TooLargeBlockException {};

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

	/// Добавление блока элементов (максимальный размер блока elementsPerBlock)
	void insert(std::vector<T> const& block)
	{
		if (block.size() > elementsPerBlock)
			throw TooLargeBlockException();

		int64_t blockNum = N / elementsPerBlock;
		if ((N % elementsPerBlock) == 0)
		{
			// Создаём новую вершину кучи
			N += block.size();
			std::sort(block.begin(), block.end(), std::greater<T>());
			storage.writeBlock(blockNum, block);
			siftUp(blockNum);
			return;
		}

		std::vector<T> hblock = storage.readBlock(blockNum);
		hblock.resize(N % elementsPerBlock);

		if (hblock.size() + block.size() <= elementsPerBlock)
		{
			hblock.insert(hblock.end(), block.begin(), block.end());
			N += block.size();
			std::sort(hblock.begin(), hblock.end(), std::greater<T>());
			storage.writeBlock(blockNum, hblock);
			siftUp(blockNum);
			return;
		}

		hblock.insert(hblock.end(), block.begin(), block.begin() + elementsPerBlock - (N % elementsPerBlock));
		assert(hblock.size() == elementsPerBlock);
		std::vector<T> newBlock(block.begin() + elementsPerBlock - (N % elementsPerBlock), block.end());
		N += elementsPerBlock - (N % elementsPerBlock);
		assert((N % elementsPerBlock) == 0);
		std::sort(hblock.begin(), hblock.end(), std::greater<T>());
		storage.writeBlock(blockNum, hblock);
		siftUp(blockNum);

		insert(newBlock);
	}

	/// Пустая ли куча
	bool empty() const
	{
		return N == 0;
	}

    /// Получить количество элементов в куче
	int64_t size() const
    {
        return N;
    }

	/// Получить блок максимальных элементов (но не извлекать)
	std::vector<T> getMaxBlock() const
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

		int64_t bCount = blocksCount();
		std::vector<T> lastBlock = storage.readBlock(bCount - 1);
		if (N % elementsPerBlock != 0 && N > 2 * elementsPerBlock)  // Если последний блок недозаполненный, то дополнить последними элементами из предпоследнего блока
		{
			lastBlock.resize(N % elementsPerBlock);
			std::vector<T> preLastBlock = storage.readBlock(bCount - 2);
			lastBlock.insert(lastBlock.end(), preLastBlock.begin() + (N % elementsPerBlock), preLastBlock.end());
			assert(lastBlock.size() == elementsPerBlock);
			std::sort(lastBlock.begin(), lastBlock.end(), std::greater<T>());
		}

		N -= lastBlock.size();

		siftDown(0, lastBlock);

        return res;
    }

    /// Распечатать содержимое кучи (использовать только для отладки)
	void debugPrint() const
    {
        printf("=================================================\n");
        printf("Elements in heap:   %ld\n", N);
        printf("Elements per block: %ld\n\n", elementsPerBlock);

		int64_t bCount = blocksCount();
		for (int64_t i = 0; i < bCount; ++i)
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
	void exportToDOT(std::string const& fileName) const
	{
		FILE* f = fopen(fileName.c_str(), "w");
		fprintf(f, "graph Heap {\n  node [shape=box];");
		int64_t bCount = blocksCount();
		for (int64_t i = 0; i < bCount; ++i)
		{
			std::vector<T> block = storage.readBlock(i);
			size_t sz = (i == N / elementsPerBlock) ? (N % elementsPerBlock) : elementsPerBlock;
			std::string bStr;
			for (size_t j = 0; j < sz; ++j)
				bStr += toString(block[j]) + (j == sz - 1 ? "" : ", ");
			fprintf(f, "  b%ld [label=\"%s\"];\n", i, bStr.c_str());
		}
		fprintf(f, "\n");
		for (int64_t i = 0; i < bCount; ++i)
		{
			if (i * 2 + 1 < bCount)
				fprintf(f, "  b%ld -- b%ld;\n", i, i * 2 + 1);
			if (i * 2 + 2 < bCount)
				fprintf(f, "  b%ld -- b%ld;\n", i, i * 2 + 2);
		}
		fprintf(f, "}\n");
		fclose(f);
	}

	void printStorageStats() const
	{
		storage.printStats();
	}

private:
	int64_t blocksCount() const
	{
		return (N + elementsPerBlock - 1) / elementsPerBlock;
	}

	/// В toBeLarger в итоге будут бОльшие значение, а в toBeSmaller - меньшие
	void remerge(std::vector<T>& toBeLarger, std::vector<T>& toBeSmaller) const
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
		if (blockNum == N / elementsPerBlock)  // Если это последняя вершина кучи (мб недозаполненная)
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
	void siftDown(int64_t blockNum, std::vector<T>& block)
	{
		int64_t bCount = blocksCount();

		while ((blockNum << 1) + 1 < bCount)  // Пока у текущей вершины есть хотя бы один ребёнок
		{
			std::vector<T> sonL = storage.readBlock((blockNum << 1) + 1);

			if ((blockNum << 1) + 2 < bCount)  // Если у вершины 2 сына
			{
				std::vector<T> sonR = storage.readBlock((blockNum << 1) + 2);
				if ((blockNum << 1) + 3 == bCount && (N % elementsPerBlock) > 0)  // Если sonR - последняя и недозаполненная вершина кучи
					sonR.resize(N % elementsPerBlock);

				if (block[elementsPerBlock - 1] >= sonL[0] && block[elementsPerBlock - 1] >= sonR[0])  // Если свойство кучи не нарушено
					break;

				if (block[elementsPerBlock - 1] >= sonL[0])  // Если свойство кучи нарушено только с ПРАВЫМ сыном (а с левым всё норм)
				{
					remerge(block, sonR);
					storage.writeBlock(blockNum, block);

					// Далее идём чинить sonR и под ним
					blockNum = (blockNum << 1) + 2;
					block = sonR;
					continue;
				}

				if (block[elementsPerBlock - 1] >= sonR[0])  // Если свойство кучи нарушено только с ЛЕВЫМ сыном (а с правым всё норм)
				{
					remerge(block, sonL);
					storage.writeBlock(blockNum, block);

					// Далее идём чинить sonL и под ним
					blockNum = (blockNum << 1) + 1;
					block = sonL;
					continue;
				}

				// Если свойство кучи нарушено для обоих сыновей

				if (sonL[elementsPerBlock - 1] > sonR[sonR.size() - 1])  // Если минимум в sonR
				{
					remerge(sonL, sonR);   // Делаем sonL > sonR (сохраняем минимум в sonR)
					remerge(block, sonL);  // Делаем block > sonL
					storage.writeBlock(blockNum, block);
					storage.writeBlock((blockNum << 1) + 2, sonR);

					// Далее идём чинить sonL и под ним
					blockNum = (blockNum << 1) + 1;
					block = sonL;
					continue;
				}

				// Если минимум в sonL (или в обоих сыновьях)

				remerge(sonR, sonL);   // Делаем sonR > sonL (сохраняем минимум в sonL)
				remerge(block, sonR);  // Делаем block > sonR
				storage.writeBlock(blockNum, block);
				if (sonL.size() == elementsPerBlock)  // Если перед всей этой бодягой sonR не был последней недозаполненной вершиной
					storage.writeBlock((blockNum << 1) + 1, sonL);
				else
				{
					// Свопнули сыновей, чтобы недозаполненным был последний: у них детей нет, так что можно так сделать
					storage.writeBlock((blockNum << 1) + 1, sonR);
					storage.writeBlock((blockNum << 1) + 2, sonL);
					return;
				}

				// Далее идём чинить sonR и под ним
				blockNum = (blockNum << 1) + 2;
				block = sonR;
				continue;
			}

			else  // Если у вершины только 1 сын (в таком случае, он также является последней вершиной кучи)
			{
				if (block[elementsPerBlock - 1] >= sonL[0])  // Если свойство кучи не нарушено
					break;

				if ((N % elementsPerBlock) > 0)
					sonL.resize(N % elementsPerBlock);

				remerge(block, sonL);
				storage.writeBlock(blockNum, block);
				storage.writeBlock((blockNum << 1) + 1, sonL);
				return;
			}
		}
		storage.writeBlock(blockNum, block);
	}

    ExternalStorage<T> storage;
    int64_t elementsPerBlock;
    int64_t N;
};
