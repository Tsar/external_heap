#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

template <class T>
class ExternalStorage
{
public:
    ExternalStorage(std::string const& storageFileName, int64_t elementsPerBlock, bool clearStorage = false)
        : storageFileName(storageFileName)
        , elementsPerBlock(elementsPerBlock)
        , blockSize(elementsPerBlock * sizeof(T))
    {
        if (clearStorage)
        {
            clear();
            return;
        }

        f = fopen(storageFileName.c_str(), "r+");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            blocksCount = ftell(f) / blockSize;
            return;
        }

        clear();  // called here for creating file for storage
    }

    ~ExternalStorage()
    {
        fclose(f);
    }

    void clear()
    {
        f = fopen(storageFileName.c_str(), "w+");
        blocksCount = 0;
    }

    std::vector<T> readBlock(int64_t blockNum)
    {
        if (blockNum >= blocksCount)
            return std::vector<T>();

        std::vector<T> res(elementsPerBlock);
        fseek(f, blockSize * blockNum, SEEK_SET);
        fread(res.data(), sizeof(T), elementsPerBlock, f);
        return res;
    }

    void writeBlock(int64_t blockNum, std::vector<T> const& block)
    {
        if (block.size() != elementsPerBlock)
            return;

        if (blockNum >= blocksCount)
        {
            if (blockNum > blocksCount)
            {
                fseek(f, blockSize * blocksCount, SEEK_SET);
                for (int64_t i = blocksCount; i < blockNum; ++i)
                    fwrite(block.data(), sizeof(T), elementsPerBlock, f);  // just filling space with any data
            }
            blocksCount = blockNum + 1;
        }

        fseek(f, blockSize * blockNum, SEEK_SET);
        fwrite(block.data(), sizeof(T), elementsPerBlock, f);
        fflush(f);
    }

private:
    FILE* f;
    std::string storageFileName;
    int64_t elementsPerBlock;
    int64_t blockSize;
    int64_t blocksCount;
};
