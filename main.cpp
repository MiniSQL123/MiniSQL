
#include <iostream>
#include "interpreter.h"
#include "buffer_manager.h"

BufferManager BM;
CatalogManager CM;

int main() {
    std::cout<<">>> Welcome to MiniSQL"<<std::endl;
    while(1){
        Interpreter query;
        query.getQuery();
        query.ProcessQuery();
    }
    return 0;
}
