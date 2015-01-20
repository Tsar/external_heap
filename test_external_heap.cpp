#include <stdlib.h>

#include "external_heap.h"

int main()
{
    ExternalHeap<int32_t> heap("extheap.data", 4);

    std::string s;
    while (s != "exit")
    {
        int32_t randVal = rand() % 20;
        std::cout << "Inserting " << randVal << " to heap" << std::endl;
        heap.insert(randVal);
        heap.debugPrint();

        std::cin >> s;
    }

    return 0;
}
