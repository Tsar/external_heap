#include "external_heap.h"

int main()
{
    ExternalHeap<int32_t> heap("extheap.data", 4096);

    return 0;
}
