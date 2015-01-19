#include "external_storage.h"

template <class T>
class ExternalHeap
{
public:
    ExternalHeap(std::string const& storageFileName, int64_t elementsPerBlock)
        : storage(storageFileName, elementsPerBlock)
    {
    }

    ~ExternalHeap()
    {
    }

    void insert(T element)
    {
    }

private:
    ExternalStorage<T> storage;
};
