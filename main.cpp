#include "malloc2.cpp"

int main() {
    std::cout <<sizeof(MallocMetadata)<<std::endl;
    std::cout <<smalloc(10)<<std::endl;
    int* ptr = (int*)smalloc(32);

    std::cout <<ptr<<std::endl;
    sfree (ptr);
    ptr = (int*)smalloc(15);
    std::cout <<ptr<<std::endl;
    memset(ptr, 5 ,15);
    //std::cout <<"nside ptr  :"<<((int)ptr)<<std::endl;
    //std::cout <<"size ptr"<<alocationList->getSize(ptr)<<std::endl;
    std::cout << srealloc(ptr,7) << std::endl;
    std::cout << srealloc(ptr,25) << std::endl;
    std::cout <<smalloc(18)<<std::endl;
    ptr = (int*) scalloc(4,6);
    std::cout <<ptr<<std::endl;

    char* ptr3 = (char*)scalloc(1, 10);
    for (int i = 0; i < 10; i++) {
        if (ptr3[i] != 0) cout << "WTF you stupied Yaron!";
    }
//    std::cout <<"size ptr"<<alocationList->getSize(ptr)<<std::endl;


    return 0;
}
