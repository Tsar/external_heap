#include "external_storage.h"

template <class T>
class ExternalHeap
{
public:
    ExternalHeap(std::string const& storageFileName, int64_t elementsPerBlock)
        : storage(storageFileName, elementsPerBlock, true /* TODO: allow reading heap from file in future */)
        , elementsPerBlock(elementsPerBlock)
    {
    }

    ~ExternalHeap()
    {
    }

    void insert(T const& element)
    {
        ++N;
        std::vector<T> block = storage.readBlock(N / elementsPerBlock);
        if (block.size() == 0)
        {
            block.resize(elementsPerBlock);
            block[0] = element;
        }
        else
        {
            block[N % elementsPerBlock] = element;
        }
        storage.writeBlock(N / elementsPerBlock, block);
        /// TODO: fix heap
    }

private:
    ExternalStorage<T> storage;
    int64_t elementsPerBlock;
    int64_t N;
};
